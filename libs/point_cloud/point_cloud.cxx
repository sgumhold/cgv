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

class point_cloud_obj_loader : public obj_reader, public point_cloud_types
{
protected:
	std::vector<Pnt>& P;
	std::vector<Nml>& N;
	std::vector<Clr>& C;
public:
	///
	point_cloud_obj_loader(std::vector<Pnt>& _P, std::vector<Nml>& _N, std::vector<Clr>& _C) : P(_P), N(_N), C(_C) {}
	/// overide this function to process a vertex
	void process_vertex(const v3d_type& p)
	{
		P.push_back(point_cloud::Pnt(p));
	}
	/// overide this function to process a normal
	void process_normal(const v3d_type& n)
	{
		N.push_back(point_cloud::Nml(n));
	}
	/// overide this function to process a color (this called for vc prefixes which is is not in the standard but for example used in pobj-files)
	void process_color(const color_type& c)
	{
		C.push_back(c);
	}

};

point_cloud::point_cloud()
{ 
	has_clrs = false;
	has_nmls = false;
	no_normals_contained = false;
	box_out_of_date = false;
}

point_cloud::point_cloud(const string& file_name)
{
	has_clrs = false;
	has_nmls = false;
	no_normals_contained = false;
	box_out_of_date = false;
	read(file_name);
}

void point_cloud::clear()
{
	P.clear();
	N.clear();
	C.clear();
	box_out_of_date = true;
}
/// append another point cloud
void point_cloud::append(const point_cloud& pc)
{
	if (pc.get_nr_points() == 0)
		return;
	Cnt old_n = P.size();
	Cnt n = P.size()+pc.get_nr_points();
	if (has_normals())
		N.resize(n);
	if (has_colors())
		C.resize(n);
	P.resize(n);
	if (has_normals() && pc.has_normals())
		std::copy(pc.N.begin(), pc.N.end(), N.begin()+old_n);
	if (has_colors() && pc.has_colors())
		std::copy(pc.C.begin(), pc.C.end(), C.begin()+old_n);
	std::copy(pc.P.begin(), pc.P.end(), P.begin()+old_n);
	box_out_of_date = true;
}

/// clip on box
void point_cloud::clip(const Box clip_box)
{
	Idx j=0;
	for (Idx i=0; i<(Idx)get_nr_points(); ++i) {
		if (clip_box.inside(pnt(i))) {
			if (j < i) {
				pnt(j) = pnt(i);
				if (has_colors())
					clr(j) = clr(i);
				if (has_normals())
					nml(j) = nml(i);
			}
			++j;
		}
	}
	if (j != get_nr_points()) {
		if (has_colors())
			C.resize(j);
		if (has_normals())
			N.resize(j);
		P.resize(j);
	}
	box_out_of_date = true;
}

/// translate by direction
void point_cloud::translate(const Dir& dir)
{
	for (Idx i=0; i<(Idx)get_nr_points(); ++i) {
		pnt(i) += dir;
	}
	B.ref_min_pnt() += dir;
	B.ref_max_pnt() += dir;
}

/// transform with linear transform 
void point_cloud::transform(const Mat& mat)
{
	for (Idx i=0; i<(Idx)get_nr_points(); ++i) {
		pnt(i) = mat*pnt(i);
	}
	box_out_of_date = true;
}

/// transform with affine transform 
void point_cloud::transform(const AMat& amat)
{
	HVec h(0,0,0,1);
	for (Idx i=0; i<(Idx)get_nr_points(); ++i) {
		(Dir&)h=pnt(i);
		pnt(i) = amat*h;
	}
	box_out_of_date = true;
}

/// transform with homogeneous transform and w-clip
void point_cloud::transform(const HMat& hmat)
{
	HVec h0(0,0,0,1), h1;
	for (Idx i=0; i<(Idx)get_nr_points(); ++i) {
		(Dir&)h0=pnt(i);
		h1 = hmat*h0;
		pnt(i) = (1/h1(3))*(const Dir&)h1;
	}
	box_out_of_date = true;
}

/// add a point and allocate normal and color if necessary
point_cloud::Idx point_cloud::add_point(const Pnt& p)
{
	if (has_normals())
		N.resize(N.size()+1);
	if (has_colors())
		C.resize(P.size()+1);
	P.push_back(p);
	return (Idx)P.size()-1;
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
	if (ext == "ply")
		success = read_ply(_file_name);
	if (success) {
		if (N.size() > 0)
			has_nmls = true;
		else if (P.size() > 0)
			has_nmls = false;

		if (C.size() > 0)
			has_clrs = true;
		else if (P.size() > 0)
			has_clrs = false;

		box_out_of_date = true;
	}
	else {
		cerr << "unknown extension <." << ext << ">." << endl;
	}
	if (N.size() > 0 && P.size() != N.size()) {
		cerr << "ups different number of normals: " << N.size() << " instead of " << P.size() << endl;
		N.resize(P.size());
	}
	if (C.size() > 0 && P.size() != C.size()) {
		cerr << "ups different number of colors: " << C.size() << " instead of " << P.size() << endl;
		C.resize(P.size());
	}
	return success;
}

bool point_cloud::write(const string& _file_name)
{
	string ext = to_lower(get_extension(_file_name));
	if (ext == "bpc")
		return write_bin(_file_name);
	if (ext == "apc" || ext == "pnt")
		return write_ascii(_file_name, ext == "apc");
	if (ext == "obj" || ext == "pobj")
		return write_obj(_file_name);
	if (ext == "ply")
		return write_ply(_file_name);
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
				P.push_back(Pnt((Crd)values[0],(Crd)values[1],(Crd)values[2]));
			if (j >= 6)
				N.push_back(Nml((Crd)values[3],(Crd)values[4],(Crd)values[5]));
			if (j >= 9)
				C.push_back(Clr((Crd)values[6],(Crd)values[7],(Crd)values[8]));

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
								P.push_back(Pnt((Crd)x,(Crd)y,(Crd)z)); 
								if ( (P.size() % 10000) == 0)
									cout << "read " << P.size() << " points" << endl;
								break;
							case 6 : 
								N.push_back(Nml((Crd)x,(Crd)y,(Crd)z)); 
								if ( (N.size() % 10000) == 0)
									cout << "read " << N.size() << " normals" << endl;
								break;
							case 7 : 
								C.push_back(Clr((Crd)x,(Crd)y,(Crd)z)); 
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
	Cnt n, m;
	bool success = 
		fread(&n,sizeof(Cnt),1,fp) == 1 &&
		fread(&m,sizeof(Cnt),1,fp) == 1;
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
	return fclose(fp) == 0 && success;
}

bool point_cloud::read_obj(const string& _file_name) 
{
	point_cloud_obj_loader pc_obj(P,N,C);
	if (!exists(_file_name))
		return false;
	clear();
	if (!pc_obj.read_obj(_file_name))
		return false;
	return true;
}
#include "ply.h"

struct PlyVertex 
{
  float x,y,z;             /* the usual 3-space position of a vertex */
  float nx, ny, nz;
  unsigned char red, green, blue, alpha;
};

static PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", Float32, Float32, offsetof(PlyVertex,x), 0, 0, 0, 0},
  {"y", Float32, Float32, offsetof(PlyVertex,y), 0, 0, 0, 0},
  {"z", Float32, Float32, offsetof(PlyVertex,z), 0, 0, 0, 0},
  {"nx", Float32, Float32, offsetof(PlyVertex,nx), 0, 0, 0, 0},
  {"ny", Float32, Float32, offsetof(PlyVertex,ny), 0, 0, 0, 0},
  {"nz", Float32, Float32, offsetof(PlyVertex,nz), 0, 0, 0, 0},
  {"red", Uint8, Uint8, offsetof(PlyVertex,red), 0, 0, 0, 0},
  {"green", Uint8, Uint8, offsetof(PlyVertex,green), 0, 0, 0, 0},
  {"blue", Uint8, Uint8, offsetof(PlyVertex,blue), 0, 0, 0, 0},
  {"alpha", Uint8, Uint8, offsetof(PlyVertex,alpha), 0, 0, 0, 0},
};

typedef struct PlyFace {
  unsigned char nverts;
  int *verts;
} PlyFace;

static PlyProperty face_props[] = { /* list of property information for a face */
{"vertex_indices", Int32, Int32, offsetof(PlyFace,verts), 1, Uint8, Uint8, offsetof(PlyFace,nverts)},
};

static char* propNames[] = { "vertex", "face" };

bool point_cloud::read_ply(const string& _file_name) 
{
	PlyFile* ply_in =  open_ply_for_read(const_cast<char*>(_file_name.c_str()));
  
	for (int elementType = 0; elementType < ply_in->num_elem_types; ++elementType) {
		int nrVertices;
		char* elem_name = setup_element_read_ply (ply_in, elementType, &nrVertices);
		if (strcmp("vertex", elem_name) == 0) {
			P.resize(nrVertices);
			N.resize(nrVertices);
			C.resize(nrVertices);
			for (int p=0; p<10; ++p) 
				setup_property_ply(ply_in, &vert_props[p]);
			for (int j = 0; j < nrVertices; j++) {
				PlyVertex vertex;
				get_element_ply(ply_in, (void *)&vertex);
				P[j].set(vertex.x, vertex.y, vertex.z);
				N[j].set(vertex.nx, vertex.ny, vertex.nz);
				C[j][0] = vertex.red*1.0f/255;
				C[j][1] = vertex.green*1.0f/255;
				C[j][2] = vertex.blue*1.0f/255;
			}
		}
	}
	/* close the PLY file */
	close_ply (ply_in);
	free_ply (ply_in);
	return true;
}

bool point_cloud::write_ply(const std::string& file_name) const
{
	PlyFile* ply_out = open_ply_for_write(file_name.c_str(), 2, propNames, PLY_BINARY_LE);
	if (!ply_out) 
		return false;
	describe_element_ply (ply_out, "vertex", P.size());
	for (int p=0; p<10; ++p) 
		describe_property_ply (ply_out, &vert_props[p]);
	describe_element_ply (ply_out, "face", 0);
	describe_property_ply (ply_out, &face_props[0]);
	header_complete_ply(ply_out);

	put_element_setup_ply (ply_out, "vertex");

	for (int j = 0; j < (int)P.size(); j++) {
		PlyVertex vertex;
		vertex.x = P[j][0];
		vertex.y = P[j][1];
		vertex.z = P[j][2];
		if (N.size() == P.size()) {
			vertex.nx = N[j][0];
			vertex.ny = N[j][1];
			vertex.nz = N[j][2];
		}
		else {
			vertex.nx = 0.0f;
			vertex.ny = 0.0f;
			vertex.nz = 1.0f;
		}
		if (C.size() == P.size()) {
			vertex.red = (unsigned char)(C[j][0]*255);
			vertex.green = (unsigned char)(C[j][1]*255);
			vertex.blue = (unsigned char)(C[j][2]*255);
		}
		else {
			vertex.red   = 255;
			vertex.green = 255;
			vertex.blue  = 255;
		}
		vertex.alpha = 255;
		put_element_ply(ply_out, (void *)&vertex);
	}
	put_element_setup_ply (ply_out, "face");
	close_ply (ply_out);
	free_ply (ply_out);
	return true;
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
	return true;
}


bool point_cloud::write_ascii(const std::string& file_name, bool write_nmls) const
{
	ofstream os(file_name.c_str());
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

bool point_cloud::write_bin(const std::string& file_name) const
{
	FILE* fp = fopen(file_name.c_str(), "wb");
	if (!fp)
		return false;
	Cnt n = (Cnt)P.size();
	Cnt m = (Cnt)N.size();
	Cnt m1 = m;
	if (has_colors() && C.size() == n)
		m1 = 2*n+m;
	bool success = 
		fwrite(&n, sizeof(Cnt),1,fp) == 1 &&
		fwrite(&m1,sizeof(Cnt),1,fp) == 1 &&
		fwrite(&P[0][0],sizeof(Pnt),n,fp) == n;
	if (has_normals())
		success = success && (fwrite(&N[0][0],sizeof(Nml),m,fp) == m);
	if (has_colors() && C.size() == n)
		success = success && (fwrite(&C[0][0],sizeof(Clr),n,fp) == n);
	return fclose(fp) == 0 && success;
}

bool point_cloud::write_obj(const std::string& file_name) const
{
	ofstream os(file_name.c_str());
	if (os.fail()) 
		return false;
	unsigned int i;
	for (i=0; i<P.size(); ++i) {
		if (has_colors())
			os << "v " << P[i][0] << " " << P[i][1] << " " << P[i][2] << " " << C[i][0] << " " << C[i][1] << " " << C[i][2] << endl;
		else
			os << "v " << P[i][0] << " " << P[i][1] << " " << P[i][2] << endl;
	}
	for (i=0; i<N.size(); ++i)
		os << "vn " << N[i][0] << " " << N[i][1] << " " << N[i][2] << endl;
	return !os.fail();
}

bool point_cloud::has_colors() const
{
	return has_clrs;
}

///
void point_cloud::create_colors()
{
	has_clrs = true;
	C.resize(P.size());
}

///
void point_cloud::destruct_colors()
{
	has_clrs = false;
	C.clear();
}

bool point_cloud::has_normals() const 
{
	return has_nmls; 
}

///
void point_cloud::create_normals()
{
	has_nmls = true;
	N.resize(P.size());
}

///
void point_cloud::destruct_normals()
{
	has_nmls = false;
	N.clear();
}

/// return box
const point_cloud::Box& point_cloud::box() const
{
	if (box_out_of_date) {
		B.invalidate();
		for (Idx i = 0; i < (Idx)get_nr_points(); ++i)
			B.add_point(pnt(i));
		box_out_of_date = false;
	}
	return B;
}
