#include "obj_reader.h"
#include <cgv/utils/file.h>
#include <cgv/type/standard_types.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/base/import.h>

using namespace cgv::math;
using namespace cgv::type;
using namespace cgv::utils;
using namespace cgv::media::illum;

#ifdef WIN32
#pragma warning(disable:4996)
#endif

namespace cgv {
	namespace media {
		namespace mesh {

			bool is_double_impl(const char* begin, const char* end, float& value)
			{
				double valued;
				bool res = cgv::utils::is_double(begin, end, valued);
				value = (float)valued;
				return res;
			}

			bool is_double_impl(const char* begin, const char* end, double& value)
			{
				return cgv::utils::is_double(begin, end, value);
			}

///
template <typename T>
bool obj_reader_generic<T>::is_double(const char* begin, const char* end, crd_type& value)
{
	return is_double_impl(begin, end, value);
}

template <typename T>
typename obj_reader_generic<T>::v2d_type obj_reader_generic<T>::parse_v2d(const std::vector<token>& t) const
{
	v2d_type v(0.0);
	t.size() > 2 && 
	is_double(t[1].begin,t[1].end, v(0)) && 
	is_double(t[2].begin,t[2].end, v(1));
	return v;
}

template <typename T>
typename obj_reader_generic<T>::v3d_type obj_reader_generic<T>::parse_v3d(const std::vector<token>& t) const
{
	v3d_type v(0,0,0);
	t.size() > 3 && 
	is_double(t[1].begin,t[1].end, v(0)) && 
	is_double(t[2].begin,t[2].end, v(1)) && 
	is_double(t[3].begin,t[3].end, v(2));
	return v;
}

template <typename T>
typename obj_reader_generic<T>::color_type obj_reader_generic<T>::parse_color(const std::vector<token>& t, unsigned off) const
{
	crd_type v[4] = {0,0,0,1};
	(t.size() > 3+off) && 
	is_double(t[1+off].begin,t[1+off].end, v[0]) && 
	is_double(t[2+off].begin,t[2+off].end, v[1]) && 
	is_double(t[3+off].begin,t[3+off].end, v[2]);
	if (t.size() > 4+off)
		is_double(t[4+off].begin,t[4+off].end, v[3]);
	return color_type((float)v[0],(float)v[1],(float)v[2],(float)v[3]);
}

/// return the index of the currently selected group or -1 if no group is defined
template <typename T>
unsigned obj_reader_generic<T>::get_current_group() const
{
	return group_index;
}

/// return the index of the currently selected material or -1 if no material is defined
template <typename T>
unsigned obj_reader_generic<T>::get_current_material() const
{
	return material_index;
}

template <typename T>
obj_reader_generic<T>::obj_reader_generic()
{
	clear();
}

template <typename T>
void obj_reader_generic<T>::clear()
{
	mtl_lib_files.clear();
	material_index_lut.clear();
	nr_materials = 0;
	nr_groups = 0;
	minus = 1;
	nr_normals = nr_texcoords = 0;
	material_index = -1;
	have_default_material = false;
}

/// overide this function to process a comment
template <typename T>
void obj_reader_generic<T>::process_comment(const std::string& comment)
{
}

/// overide this function to process a vertex
template <typename T>
void obj_reader_generic<T>::process_vertex(const v3d_type& p)
{
}

/// overide this function to process a texcoord
template <typename T>
void obj_reader_generic<T>::process_texcoord(const v2d_type& t)
{
}

/// overide this function to process a normal
template <typename T>
void obj_reader_generic<T>::process_normal(const v3d_type& n)
{
}

/// overide this function to process a normal
template <typename T>
void obj_reader_generic<T>::process_color(const color_type& c)
{
}

/// convert negative indices to positive ones by adding the number of elements
template <typename T>
void obj_reader_generic<T>::convert_to_positive(unsigned vcount, int *vertices,
						 int *texcoords, int *normals,
						 unsigned v, unsigned n, unsigned t)
{
	for (unsigned int i=0; i<vcount; ++i) {
		if (vertices[i] < 0)
			vertices[i] += v;
		if (texcoords) {
			if (texcoords[i] < 0)
				texcoords[i] += t;
		}
		if (normals) {
			if (normals[i] < 0)
				normals[i] += n;
		}
	}
}

/// overide this function to process a face
template <typename T>
void obj_reader_generic<T>::process_face(unsigned vcount, int *vertices, int *texcoords, int *normals)
{
}

/// overide this function to process a group given by name and parameter string
template <typename T>
void obj_reader_generic<T>::process_group(const std::string& name, const std::string& parameters)
{
}

/// process a material definition
template <typename T>
void obj_reader_generic<T>::process_material(const cgv::media::illum::obj_material& mtl, unsigned)
{
}

template <typename T>
bool obj_reader_generic<T>::read_obj(const std::string& file_name)
{
	std::string content;
	if (!file::read(file_name, content, true))
		return false;

	path_name = file::get_path(file_name);
	if (!path_name.empty())
		path_name += "/";

	std::vector<line> lines;
	split_to_lines(content,lines);
	
	minus = 1;
	material_index = -1;
	group_index = -1;
	nr_groups = 0;
	nr_normals = nr_texcoords = 0;
	std::map<std::string,unsigned> group_index_lut;
	std::vector<token> tokens;
	for (unsigned li=0; li<lines.size(); ++li) {
		if(li % 1000 == 0)
			printf("%d Percent done.\r", (int)(100.0*li/(lines.size()-1)) );

		tokenizer(lines[li]).bite_all(tokens);
		if (tokens.size() == 0)
			continue;

		switch (tokens[0][0]) {
		case 'v' :
			if (tokens[0].size() == 1) {
				process_vertex(parse_v3d(tokens));
				if (tokens.size() >= 7)
					process_color(parse_color(tokens, 3));
			}
			else {
				switch (tokens[0][1]) {
				case 'n' :
					process_normal(parse_v3d(tokens));
					++nr_normals;
					break;
				case 't' : 
					process_texcoord(parse_v2d(tokens));
					++nr_texcoords;
					break;
				case 'c' : 
					process_color(parse_color(tokens));
					break;
				}
			}
			break;
		case 'f' :
			if (group_index == -1) {
				group_index = 0;
				nr_groups = 1;
				process_group("main","");
				group_index_lut["main"] = group_index;
			}
			if (material_index == -1) {
				obj_material m;
				m.set_name("default");
				material_index = 0;
				nr_materials = 1;
				process_material(m, 0);
				material_index_lut[m.get_name()] = material_index;
				have_default_material = true;
			}
			parse_face(tokens); 
			break;
		case 'g' : 
			if (tokens.size() > 1) {
				std::string name = to_string(tokens[1]);
				std::string parameters;
				if (tokens.size() > 2)
					parameters.assign(tokens[2].begin, tokens.back().end - tokens[2].begin);

				std::map<std::string,unsigned>::iterator it = 
					group_index_lut.find(name);

				if (it != group_index_lut.end())
					group_index = it->second;
				else {
					group_index = nr_groups;
					++nr_groups;
					process_group(name, parameters);
					group_index_lut[name] = group_index;
				}
			}
			break;
		default:
			if (to_string(tokens[0]) == "usemtl")
				parse_material(tokens);
			else if (to_string(tokens[0]) == "mtllib") {
				if (tokens.size() > 1)
					read_mtl(to_string(tokens[1]));
			}
		}
		tokens.clear();
	}
	printf("\n");
	return true;
}

template <typename T>
bool obj_reader_generic<T>::read_mtl(const std::string& file_name)
{
	std::string fn = cgv::base::find_data_file(file_name, "McpD", "", path_name);
	if (path_name.empty()) {
		path_name = file::get_path(file_name);
		if (!path_name.empty())
			path_name += "/";
	}
	if (mtl_lib_files.find(fn) != mtl_lib_files.end())
		return true;

	std::string content;
	if (!file::read(fn, content, true))
		return false;

	mtl_lib_files.insert(fn);

	std::vector<line> lines;
	split_to_lines(content,lines);

	std::vector<token> tokens;
	obj_material mtl;
	bool in_mtl = false;

	for (unsigned li=0; li<lines.size(); ++li) {
		tokens.clear();
		tokenizer(lines[li]).bite_all(tokens);
		if (tokens.size() == 0)
			continue;
		if (tokens[0] == "newmtl") {
			if (in_mtl) {
				// check if material name is new
				if (material_index_lut.find(mtl.get_name()) == material_index_lut.end()) {
					material_index_lut[mtl.get_name()] = nr_materials;
					process_material(mtl, nr_materials);
					++nr_materials;
				}
				// if not overwrite old definition
				else 
					process_material(mtl, material_index_lut[mtl.get_name()]);
			}
			in_mtl = true;
			mtl = obj_material();
			if (tokens.size() > 1)
				mtl.set_name(to_string(tokens[1]));
		}
		else if (tokens[0] == "map_Ka" && tokens.size() > 1)
			mtl.set_ambient_texture_name(path_name+to_string(tokens[1]));
		else if (tokens[0] == "map_Kd" && tokens.size() > 1)
			mtl.set_diffuse_texture_name(path_name + to_string(tokens[1]));
		else if (tokens[0] == "map_d" && tokens.size() > 1)
			mtl.set_opacity_texture_name(path_name + to_string(tokens[1]));
		else if (tokens[0] == "map_Ks" && tokens.size() > 1)
			mtl.set_specular_texture_name(path_name + to_string(tokens[1]));
		else if (tokens[0] == "map_Ke" && tokens.size() > 1)
			mtl.set_emission_texture_name(path_name + to_string(tokens[1]));
		else if (tokens[0] == "Ka")
			mtl.set_ambient(parse_color(tokens));
		else if (tokens[0] == "Kd")
			mtl.set_diffuse(parse_color(tokens));
		else if (tokens[0] == "Ks")
			mtl.set_specular(parse_color(tokens));
		else if (tokens[0] == "Ke")
			mtl.set_emission(parse_color(tokens));
		else if (tokens[0] == "Ns")
			mtl.set_shininess((float)atof(to_string(tokens.back()).c_str()));
		else if (tokens[0] == "d")
			mtl.set_opacity((float)atof(to_string(tokens.back()).c_str()));
		else if (tokens[0] == "bump") {
			for (unsigned i=1; i<tokens.size(); ++i) {
				if (tokens[i] == "-bm") {
					if (i+1 < tokens.size()) {
						mtl.set_bump_scale((float)atof(to_string(tokens[i+1]).c_str()));
						++i;
					}
				}
				else {
					mtl.set_bump_texture_name(path_name+to_string(tokens[i]));
				}
			}
		}
	}
	if (in_mtl) {
		// check if material name is new
		if (material_index_lut.find(mtl.get_name()) == material_index_lut.end()) {
			material_index_lut[mtl.get_name()] = nr_materials;
			process_material(mtl, nr_materials);
			++nr_materials;
		}
		// if not overwrite old definition
		else 
			process_material(mtl, material_index_lut[mtl.get_name()]);
	}
	return true;
}

template <typename T>
void obj_reader_generic<T>::parse_material(const std::vector<token>& tokens)
{
	if (tokens.size() < 2)
		return;

	std::map<std::string,unsigned>::iterator it = 
		material_index_lut.find(to_string(tokens[1]));
	
	if(it != material_index_lut.end())
		material_index = it->second;
}

template <typename T>
void obj_reader_generic<T>::parse_face(const std::vector<token>& tokens)
{
	std::vector<int> vertex_indices;
	std::vector<int> normal_indices;
	std::vector<int> texcoord_indices;

	for(unsigned i = 1; i < tokens.size(); i++)	{ 
		std::vector<token> smaller_tokens;
		tokenizer(tokens[i]).set_sep("/").bite_all(smaller_tokens);
		if (smaller_tokens.size() < 1)
			continue;
		int vi = atoi(to_string(smaller_tokens[0]).c_str());
		if (vi > 0)
			vi -= minus;
		vertex_indices.push_back(vi);
		if (smaller_tokens.size() == 1) {
			if ((int)nr_normals > vi)
				normal_indices.push_back(vi);
			if ((int)nr_texcoords > vi)
				texcoord_indices.push_back(vi);
			continue;
		}
		if (smaller_tokens.size() < 3)
			continue;
		unsigned j = 2;
		if (smaller_tokens[j] != "/") {
			int ti = atoi(to_string(smaller_tokens[j]).c_str());
			if (ti > 0)
				ti -= minus;
			if ((int)nr_texcoords > ti)
				texcoord_indices.push_back(ti);
			++j;
		}
		if (smaller_tokens.size() < j+2)
			continue;
		int ni = atoi(to_string(smaller_tokens[j+1]).c_str());
		if (ni > 0)
			ni -= minus;
		if ((int)nr_normals > ni)
			normal_indices.push_back(ni);
	}
	int* nml_ptr = 0;
	if (normal_indices.size() == vertex_indices.size())
		nml_ptr = &normal_indices[0];
	int* tex_ptr = 0;
	if (texcoord_indices.size() == vertex_indices.size())
		tex_ptr = &texcoord_indices[0];
	process_face((unsigned) vertex_indices.size(), &vertex_indices[0], tex_ptr, nml_ptr);
}


template class obj_reader_generic < float >;
template class obj_reader_generic < double >;

		}
	}
}
