#pragma once

#include "point_cloud.h"
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/media/mesh/obj_reader.h>
#include <fstream>

#pragma warning(disable:4996)

using namespace cgv::utils::file;
using namespace cgv::utils;
using namespace std;
using namespace cgv::media::mesh;

class point_cloud_obj_loader : public obj_reader
{
protected:
	point_cloud* pc;
public:
	///
	point_cloud_obj_loader(point_cloud* _pc) : pc(_pc) {}
	/// overide this function to process a vertex
	void process_vertex(const v3d_type& p)
	{
		pc->P.push_back(point_cloud::Pnt(p));
	}
	/// overide this function to process a normal
	void process_normal(const v3d_type& n)
	{
		pc->N.push_back(point_cloud::Nml(n));
	}
};

point_cloud::point_cloud()
{ 
	no_normals_contained = false;
}

point_cloud::point_cloud(const string& file_name)
{
	no_normals_contained = false;
	read(file_name);
}

void point_cloud::clear()
{
	P.clear();
	N.clear();
	C.clear();
}

bool point_cloud::read(const string& _file_name)
{
	string ext = to_lower(get_extension(_file_name));
	bool success = false;
	if (ext == "bpc")
		success = read_bin(_file_name);
	if (ext == "points")
		success = read_points(_file_name);
	if (ext == "wrl")
		success = read_wrl(_file_name);
	if (ext == "apc" || ext == "pnt")
		success = read_ascii(_file_name);
	if (ext == "obj" || ext == "pobj")
		success = read_obj(_file_name);
	if (success) {
		set_file_name(_file_name);
		compute_box();
	}
	else
		cerr << "unknown extension <." << ext << ">." << endl;
	return success;
}

bool point_cloud::write(const string& _file_name)
{
	file_name = drop_extension(_file_name);
	string ext = to_lower(get_extension(_file_name));
	if (ext == "bpc")
		return write_bin();
	if (ext == "apc" || ext == "pnt")
		return write_ascii(ext == "apc");
	if (ext == "obj" || ext == "pobj")
		return write_obj();
	cerr << "unknown extension <." << ext << ">." << endl;
	return false;
}

bool point_cloud::read_points(const std::string& file_name)
{
	string content;
	if (!cgv::utils::file::read(file_name, content, true))
		return false;
	clear();
	vector<line> lines;
	split_to_lines(content, lines);
	bool do_parse = false;
	unsigned i;
	for (i=0; i<lines.size(); ++i) {
		if (do_parse) {
			if (lines[i].empty())
				continue;
			vector<token> numbers;
			tokenizer(lines[i]).bite_all(numbers);
			double values[9];
			unsigned n = min(9,(int)numbers.size());
			unsigned j;
			for (j=0; j<n; ++j) {
				if (!is_double(numbers[j].begin, numbers[j].end, values[j]))
					break;
			}
			if (j >= 3)
				P.push_back(Pnt((coord_type)values[0],(coord_type)values[1],(coord_type)values[2]));
			if (j >= 6)
				N.push_back(Nml((coord_type)values[3],(coord_type)values[4],(coord_type)values[5]));
			if (j >= 9)
				C.push_back(Clr((coord_type)values[6],(coord_type)values[7],(coord_type)values[8]));

			if ( (P.size() % 10000) == 0)
				cout << "read " << P.size() << " points" << endl;
		}
		if (lines[i] == "#Data:") {
			do_parse = true;
			cout << "starting to parse after line " << i << endl;
		}
	}
	return true;
}

bool point_cloud::read_wrl(const std::string& file_name)
{
	string content;
	if (!cgv::utils::file::read(file_name, content, true))
		return false;
	clear();
	vector<line> lines;
	split_to_lines(content, lines);
	int parse_mode = 0;
	unsigned i, j;
	for (i=0; i<lines.size(); ++i) {
		if (lines[i].empty())
			continue;
		vector<token> toks;
		tokenizer(lines[i]).set_sep("[]{},").bite_all(toks);
		switch (parse_mode) {
		case 0 :
			for (j=0; j<toks.size(); ++j)
				if (toks[j] == "Shape")
					++parse_mode;
			break;
		case 1 :
			for (j=0; j<toks.size(); ++j) {
				if (toks[j] == "point")
					++parse_mode;
				else if (toks[j] == "normal")
					parse_mode += 2;
				else if (toks[j] == "color")
					parse_mode += 3;
				else if (toks[j] == "[") {
					if (parse_mode > 1 && parse_mode < 5)
						parse_mode += 3;
				}
			}
			break;
		case 2 :
		case 3 :
		case 4 :
			for (j=0; j<toks.size(); ++j) {
				if (toks[j] == "[") {
					parse_mode += 3;
					break;
				}
			}
			break;
		case 5 :
		case 6 :
		case 7 :
			if (toks[0] == "]")
				parse_mode = 1;
			else {
				if (toks.size() >= 3) {
					double x,y,z;
					if (is_double(toks[0].begin, toks[0].end, x) && 
						is_double(toks[1].begin, toks[1].end, y) && 
						is_double(toks[2].begin, toks[2].end, z)) {
							switch (parse_mode) {
							case 5 : 
								P.push_back(Pnt((coord_type)x,(coord_type)y,(coord_type)z)); 
								if ( (P.size() % 10000) == 0)
									cout << "read " << P.size() << " points" << endl;
								break;
							case 6 : 
								N.push_back(Nml((coord_type)x,(coord_type)y,(coord_type)z)); 
								if ( (N.size() % 10000) == 0)
									cout << "read " << N.size() << " normals" << endl;
								break;
							case 7 : 
								C.push_back(Clr((coord_type)x,(coord_type)y,(coord_type)z)); 
								if ( (C.size() % 10000) == 0)
									cout << "read " << C.size() << " colors" << endl;
								break;
							}
					}
				}
			}
			break;
		}
	}
	return true;
}


bool point_cloud::read_bin(const string& file_name)
{
	FILE* fp = fopen(file_name.c_str(), "rb");
	if (!fp)
		return false;
	unsigned int n, m;
	bool success = 
		fread(&n,sizeof(unsigned int),1,fp) == 1 &&
		fread(&m,sizeof(unsigned int),1,fp) == 1;
	if (success) {
		clear();
		bool has_clrs = m >= 2*n;
		P.resize(n);
		if (has_clrs) {
			m = m-2*n;
			C.resize(n);
		}
		N.resize(m);
		success = fread(&P[0][0],sizeof(Pnt),n,fp) == n;
		if (m > 0)
			success = success && (fread(&N[0][0],sizeof(Nml),m,fp) == m);
		if (has_clrs)
			success = success && fread(&C[0][0],sizeof(Clr),n,fp) == n;
	}
	if (success) {
		if (n != m && m != 0)
			cerr << "ups different number of normals: " << m << " instead of " << n << endl;
	}
	return fclose(fp) == 0 && success;
}

bool point_cloud::read_obj(const string& _file_name) 
{
	point_cloud_obj_loader pc_obj(this);
	if (!exists(_file_name))
		return false;
	clear();
	if (!pc_obj.read_obj(_file_name))
		return false;
	if (P.size() != N.size())
		cerr << "ups different number of normals: " << N.size() << " instead of " << P.size() << endl;
	return true;
}

void point_cloud::set_file_name(const string& _file_name)
{
	file_name = drop_extension(_file_name);
}


bool point_cloud::read_ascii(const string& file_name)
{
	ifstream is(file_name.c_str());
	if (is.fail()) 
		return false;
	clear();
	while (!is.eof()) {
		char buffer[4096];
		is.getline(buffer,4096);
		float x, y, z, nx, ny, nz, r, g, b;
		unsigned int n = sscanf(buffer, "%f %f %f %f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz, &r, &g, &b);
		if (n == 3 || n == 6 || n == 9)
			P.push_back(Pnt(x,y,z));
		if (n == 6) {
			if (no_normals_contained)
				C.push_back(Clr(nx,ny,nz));
			else
				N.push_back(Nml(nx,ny,nz));
		}
		if (n == 9)
			C.push_back(Clr(r,g,b));
	}
	if (has_normals() && (P.size() != N.size()))
		cerr << "ups different number of normals: " << N.size() << " instead of " << P.size() << endl;
	return true;
}


bool point_cloud::write_ascii(bool write_nmls) const
{
	ofstream os((file_name+(write_nmls?".apc":".pnt")).c_str());
	if (os.fail()) 
		return false;
	for (unsigned int i=0; i<P.size(); ++i) {
		os << P[i][0] << " " << P[i][1] << " " << P[i][2];
		if (write_nmls) 
			os << " " << N[i][0] << " " << N[i][1] << " " << N[i][2];
		os << endl;
	}
	return !os.fail();
}

bool point_cloud::write_bin() const
{
	FILE* fp = fopen((file_name+".bpc").c_str(), "wb");
	if (!fp)
		return false;
	unsigned int n = (unsigned int)P.size();
	unsigned int m = (unsigned int)N.size();
	unsigned int m1 = m;
	if (has_colors() && C.size() == n)
		m1 = 2*n+m;
	bool success = 
		fwrite(&n,sizeof(unsigned int),1,fp) == 1 &&
		fwrite(&m1,sizeof(unsigned int),1,fp) == 1 &&
		fwrite(&P[0][0],sizeof(Pnt),n,fp) == n;
	if (has_normals())
		success = success && (fwrite(&N[0][0],sizeof(Nml),m,fp) == m);
	if (has_colors() && C.size() == n)
		success = success && (fwrite(&C[0][0],sizeof(Clr),n,fp) == n);
	return fclose(fp) == 0 && success;
}

bool point_cloud::write_obj() const
{
	ofstream os((file_name+".pobj").c_str());
	if (os.fail()) 
		return false;
	unsigned int i;
	for (i=0; i<P.size(); ++i)
		os << "v " << P[i][0] << " " << P[i][1] << " " << P[i][2] << endl;
	for (i=0; i<N.size(); ++i)
		os << "vn " << N[i][0] << " " << N[i][1] << " " << N[i][2] << endl;
	return !os.fail();
}

void point_cloud::compute_box() 
{
	box.invalidate();
	for (unsigned int i = 0; i < P.size(); ++i)
		box.add_point(P[i]);
}

bool point_cloud::has_colors() const
{
	return !C.empty();
}

bool point_cloud::has_normals() const 
{
	return !N.empty(); 
}
