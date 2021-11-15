#include "obj_loader.h"
#include <cgv/utils/file.h>
#include <cgv/type/standard_types.h>

using namespace cgv::utils::file;
using namespace cgv::type;
using namespace cgv::media::illum;

#ifdef WIN32
#pragma warning (disable:4996)
#endif

namespace cgv {
	namespace media {
		namespace mesh {

			line_info::line_info(unsigned _nr, unsigned _vi0, int _ti0, int _ni0, unsigned _gi)
				: length(_nr),
				first_vertex_index(_vi0),
				first_texcoord_index(_ti0),
				first_normal_index(_ni0),
				group_index(_gi)
			{
			}

			face_info::face_info(unsigned _nr, unsigned _vi0, int _ti0, int _ni0, unsigned _gi, unsigned _mi)
				: degree(_nr),
				first_vertex_index(_vi0),
				first_texcoord_index(_ti0),
				first_normal_index(_ni0),
				group_index(_gi),
				material_index(_mi)
			{
			}

			/// overide this function to process a vertex
template <typename T>
void obj_loader_generic<T>::process_vertex(const v3d_type& p)
{
	vertices.push_back(p);
}

/// overide this function to process a texcoord
template <typename T>
void obj_loader_generic<T>::process_texcoord(const v2d_type& t)
{
	texcoords.push_back(t);
}
/// overide this function to process a normal
template <typename T>
void obj_loader_generic<T>::process_normal(const v3d_type& n)
{
	normals.push_back(n);
}

template <typename T>
void obj_loader_generic<T>::process_color(const color_type& c)
{
	colors.push_back(c);
}

/// overide this function to process a line
template <typename T>
void obj_loader_generic<T>::process_line(unsigned vcount, int* vertices,
	int* texcoords, int* normals)
{
	this->convert_to_positive(vcount, vertices, texcoords, normals,
		(unsigned)this->vertices.size(), (unsigned)this->normals.size(), (unsigned)this->texcoords.size());
	lines.push_back(line_info(vcount, (unsigned)vertex_indices.size(),
		texcoords == 0 ? -1 : (int)texcoord_indices.size(),
		normals == 0 ? -1 : (int)normal_indices.size(),
		this->get_current_group()));
	unsigned i;
	for (i = 0; i < vcount; ++i)
		vertex_indices.push_back(vertices[i]);
	if (normals)
		for (i = 0; i < vcount; ++i)
			normal_indices.push_back(normals[i]);
	if (texcoords)
		for (i = 0; i < vcount; ++i)
			texcoord_indices.push_back(texcoords[i]);
}

/// overide this function to process a face
template <typename T>
void obj_loader_generic<T>::process_face(unsigned vcount, int *vertices,
							  int *texcoords, int *normals)
{
	this->convert_to_positive(vcount,vertices,texcoords,normals,
		(unsigned)this->vertices.size(), (unsigned)this->normals.size(), (unsigned)this->texcoords.size());
	faces.push_back(face_info(vcount,(unsigned)vertex_indices.size(),
		texcoords == 0 ? -1 : (int)texcoord_indices.size(),
		normals == 0 ? -1 : (int)normal_indices.size(),
		this->get_current_group(),this->get_current_material()));
	unsigned i;
	for (i=0; i<vcount; ++i)
		vertex_indices.push_back(vertices[i]);
	if (normals)
		for (i=0; i<vcount; ++i)
			normal_indices.push_back(normals[i]);
	if (texcoords)
		for (i=0; i<vcount; ++i)
			texcoord_indices.push_back(texcoords[i]);
}


/// overide this function to process a group given by name
template <typename T>
void obj_loader_generic<T>::process_group(const std::string& name, const std::string& parameters)
{
	group_info gi;
	gi.name = name;
	gi.parameters = parameters;
	groups.push_back(gi);
}

/// process a material definition
template <typename T>
void obj_loader_generic<T>::process_material(const obj_material& mtl, unsigned idx)
{
	if (idx >= materials.size())
		materials.push_back(mtl);
	else
		materials[idx] = mtl;
}

template <typename T>
const char* get_bin_extension()
{
	return  ".bin_obj";
}

template <>
const char* get_bin_extension<float>()
{
	return  ".bin_objf";
}

/// overloads reading to support binary file format
template <typename T>
bool obj_loader_generic<T>::read_obj(const std::string& file_name)
{
	// check if binary file exists
	std::string bin_fn = drop_extension(file_name) + get_bin_extension<T>();
	if (exists(bin_fn) &&
		get_last_write_time(bin_fn) > get_last_write_time(file_name) &&
		read_obj_bin(bin_fn)) {
			this->path_name = get_path(file_name);
			if (!this->path_name.empty())
				this->path_name += "/";
			return true;
	}
	
	vertices.clear();
	normals.clear();
	texcoords.clear(); 
	faces.clear(); 
	colors.clear();

	if (!obj_reader_generic<T>::read_obj(file_name))
		return false;

	// correct colors in case of 8bit colors
	unsigned i;
	bool do_correct = false;
	for (i = 0; i < colors.size(); ++i) {
		if (colors[i][0] > 5 ||colors[i][1] > 5 ||colors[i][2] > 5) {
			do_correct = true;
			break;
		}
	}
	if (do_correct) {
		for (i = 0; i < colors.size(); ++i) {
			colors[i] *= 1.0f/255;
		}
	}
	write_obj_bin(bin_fn);
	return true;
}

template <typename T>
bool obj_loader_generic<T>::read_obj_bin(const std::string& file_name)
{
	// open binary file
	FILE* fp = fopen(file_name.c_str(), "rb");
	if (!fp)
		return false;

	// read element count
	uint32_type v, n, t, f, h, g, m, l = 0;
	if (1!=fread(&v, sizeof(uint32_type), 1, fp) ||
		1!=fread(&n, sizeof(uint32_type), 1, fp) ||
		1!=fread(&t, sizeof(uint32_type), 1, fp) ||
		1!=fread(&f, sizeof(uint32_type), 1, fp) ||
		1!=fread(&h, sizeof(uint32_type), 1, fp) ||
		1!=fread(&g, sizeof(uint32_type), 1, fp) ||
		1!=fread(&m, sizeof(uint32_type), 1, fp)) {
		fclose(fp);
		return false;
	}
	bool has_colors = false;
	if (v > 0x7FFFFFFF) {
		v = 0xFFFFFFFF - v;
		has_colors = true;
	}
	bool has_lines = false;
	if (f > 0x7FFFFFFF) {
		f = 0xFFFFFFFF - f;
		has_lines = true;
	}
	if (has_lines) {
		if (1 != fread(&l, sizeof(uint32_type), 1, fp)) {
			fclose(fp);
			return false;
		}
	}
	// reserve space
	vertices.resize(v);
	if (has_colors)
		colors.resize(v);

	vertex_indices.resize(h);
	if (v != fread(&vertices[0], sizeof(v3d_type), v, fp) ||
		h > 0 && h != fread(&vertex_indices[0], sizeof(unsigned), h, fp) )
	{
		fclose(fp);
		return false;
	}
	if (has_colors) {
		if (v != fread(&colors[0], sizeof(color_type), v, fp))
		{
			fclose(fp);
			return false;
		}
	}
	if (n > 0) {
		normals.resize(n);
		normal_indices.resize(h);
		if (n != fread(&normals[0], sizeof(v3d_type), n, fp) ||
			h > 0 && h != fread(&normal_indices[0], sizeof(unsigned), h, fp) )
		{
			fclose(fp);
			return false;
		}
	}
	if (t > 0) {
		texcoords.resize(t);
		texcoord_indices.resize(h);
		if (t != fread(&texcoords[0], sizeof(v2d_type), t, fp) ||
			h > 0 && h != fread(&texcoord_indices[0], sizeof(unsigned), h, fp) )
		{
			fclose(fp);
			return false;
		}
	}
	lines.resize(l);
	if (l > 0 && l != fread(&lines[0], sizeof(line_info), l, fp))
	{
		fclose(fp);
		return false;
	}
	faces.resize(f);
	if (f > 0 && f != fread(&faces[0], sizeof(face_info), f, fp))
	{
		fclose(fp);
		return false;
	}
	groups.resize(g);
	for (unsigned gi=0; gi<g; ++gi) {
		if (!read_string_bin(groups[gi].name, fp) ||
			!read_string_bin(groups[gi].parameters, fp))
		{
			fclose(fp);
			return false;
		}
	}
	if (1 != fread(&this->have_default_material, sizeof(bool), 1, fp))
	{
		fclose(fp);
		return false;
	}
	if (this->have_default_material)
		materials.push_back(obj_material());
	
	for (unsigned mi=0; mi<m; ++mi) {
		std::string s;
		if (!read_string_bin(s, fp)) {
			fclose(fp);
			return false;
		}
		obj_reader_generic<T>::read_mtl(s);
	}
	fclose(fp);
	return true;
}

/// prepare for reading another file
template <typename T>
void obj_loader_generic<T>::clear()
{
	obj_reader_generic<T>::clear();

	vertices.clear(); 
	normals.clear(); 
	texcoords.clear(); 

	vertex_indices.clear();
	normal_indices.clear();
	texcoord_indices.clear();

	faces.clear(); 
	groups.clear();
	materials.clear();
}

template <typename T>
bool obj_loader_generic<T>::write_obj_bin(const std::string& file_name) const
{
	// open binary file
	FILE* fp = fopen(file_name.c_str(), "wb");
	if (!fp)
		return false;



	// read element count
	uint32_type v = (unsigned) vertices.size(), 
		        n = (unsigned) normals.size(),
				t = (unsigned) texcoords.size(), 
				f = (unsigned) faces.size(),
				l = (unsigned) lines.size(),
				h = (unsigned) vertex_indices.size(),
				g = (unsigned) groups.size(),
				m = (unsigned) this->mtl_lib_files.size();
	uint32_type v_write = v;
	bool has_colors = (colors.size() == vertices.size());
	if (has_colors)
		v_write = 0xFFFFFFFF - v;

	uint32_type f_write = f;
	if (l > 0)
		f_write = 0xFFFFFFFF - f;

	if (1!=fwrite(&v_write, sizeof(uint32_type), 1, fp) ||
		1!=fwrite(&n, sizeof(uint32_type), 1, fp) ||
		1!=fwrite(&t, sizeof(uint32_type), 1, fp) ||
		1!=fwrite(&f_write, sizeof(uint32_type), 1, fp) ||
		1!=fwrite(&h, sizeof(uint32_type), 1, fp) ||
		1!=fwrite(&g, sizeof(uint32_type), 1, fp) ||
		1!=fwrite(&m, sizeof(uint32_type), 1, fp)) {
		fclose(fp);
		return false;
	}
	if (l > 0) {
		if (1 != fwrite(&l, sizeof(uint32_type), 1, fp)) {
			fclose(fp);
			return false;
		}
	}
	if (v != fwrite(&vertices[0], sizeof(v3d_type), v, fp) ||
		h > 0 && h != fwrite(&vertex_indices[0], sizeof(unsigned), h, fp) )
	{
		fclose(fp);
		return false;
	}

	if (has_colors) {
		if (v != fwrite(&colors[0], sizeof(color_type), v, fp) )
		{
			fclose(fp);
			return false;
		}
	}
	if (n > 0) {
		if (n != fwrite(&normals[0], sizeof(v3d_type), n, fp) ||
			h > 0 && h != fwrite(&normal_indices[0], sizeof(unsigned), h, fp) )
		{
			fclose(fp);
			return false;
		}
	}
	if (t > 0) {
		if (t != fwrite(&texcoords[0], sizeof(v2d_type), t, fp) ||
			h > 0 && h != fwrite(&texcoord_indices[0], sizeof(unsigned), h, fp) )
		{
			fclose(fp);
			return false;
		}
	}
	if (l > 0 && l != fwrite(&lines[0], sizeof(line_info), l, fp))
	{
		fclose(fp);
		return false;
	}
	if (f > 0 && f != fwrite(&faces[0], sizeof(face_info), f, fp))
	{
		fclose(fp);
		return false;
	}
	for (unsigned gi=0; gi<g; ++gi) {
		if (!write_string_bin(groups[gi].name, fp) ||
			!write_string_bin(groups[gi].parameters, fp))
		{
			fclose(fp);
			return false;
		}
	}

	if (1 != fwrite(&this->have_default_material, sizeof(bool), 1, fp))
	{
		fclose(fp);
		return false;
	}

	std::set<std::string>::const_iterator mi = this->mtl_lib_files.begin();
	for (; mi != this->mtl_lib_files.end(); ++mi) {
		if (!write_string_bin(*mi, fp)) 
		{
			fclose(fp);
			return false;
		}
	}
	fclose(fp);
	return true;
}

template <typename T>
void obj_loader_generic<T>::show_stats() const
{
	std::cout << "num vertices "<<vertices.size()<<std::endl;
	std::cout << "num normals "<<normals.size()<<std::endl;
	std::cout << "num texcoords "<<texcoords.size()<<std::endl;
	std::cout << "num faces "<<faces.size()<<std::endl;
	std::cout << "num materials "<<materials.size()<<std::endl;
	std::cout << "num groups "<<groups.size()<<std::endl;
}

template class obj_loader_generic < float >;
template class obj_loader_generic < double >;


		}
	}
}