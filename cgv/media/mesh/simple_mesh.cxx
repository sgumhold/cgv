#include "simple_mesh.h"
#include <cgv/media/mesh/obj_reader.h>

namespace cgv {
	namespace media {
		namespace mesh {

/// merge the three indices into one index into a vector of unique index triples
void simple_mesh_base::merge_indices(std::vector<idx_type>& indices, std::vector<vec3i>& unique_triples, bool* include_tex_coords_ptr, bool* include_normals_ptr) const
{
	bool include_tex_coords = tex_coord_indices.size() == position_indices.size();
	if (include_tex_coords_ptr)
		*include_tex_coords_ptr = include_tex_coords = include_tex_coords && *include_tex_coords_ptr;

	bool include_normals    = normal_indices.size() == position_indices.size();
	if (include_normals_ptr)
		*include_normals_ptr = include_normals = include_normals && *include_normals_ptr;

	std::map<std::tuple<idx_type, idx_type, idx_type>, idx_type> corner_to_index;
	for (idx_type ci = 0; ci < position_indices.size(); ++ci) {
		// construct corner
		vec3i c(position_indices[ci], include_tex_coords ? tex_coord_indices[ci] : 0, include_normals ? normal_indices[ci] : 0);
		std::tuple<idx_type, idx_type, idx_type> triple(c(0),c(1),c(2));
		// look corner up in map
		auto iter = corner_to_index.find(triple);
		// determine vertex index
		idx_type vi;
		if (iter == corner_to_index.end()) {
			vi = unique_triples.size();
			corner_to_index[triple] = vi;
			unique_triples.push_back(c);
		}
		else
			vi = iter->second;

		indices.push_back(vi);
	}
}

/// extract element array buffers for triangulation
void simple_mesh_base::extract_triangle_element_buffer(const std::vector<idx_type>& vertex_indices, std::vector<idx_type>& triangle_element_buffer)
{
	// construct triangle element buffer
	for (idx_type fi = 0; fi < faces.size(); ++fi) {
		if (face_degree(fi) == 3) {
			for (idx_type ci = begin_corner(fi); ci < end_corner(fi); ++ci)
				triangle_element_buffer.push_back(vertex_indices.at(ci));
		}
		else {
			// in case of non triangular faces do simplest triangulation approach that assumes convexity of faces
			for (idx_type ci = begin_corner(fi) + 2; ci < end_corner(fi); ++ci) {
				triangle_element_buffer.push_back(vertex_indices.at(begin_corner(fi)));
				triangle_element_buffer.push_back(vertex_indices.at(ci - 1));
				triangle_element_buffer.push_back(vertex_indices.at(ci));
			}
		}
	}
}

/// extract element array buffers for edges in wireframe
void simple_mesh_base::extract_wireframe_element_buffer(const std::vector<idx_type>& vertex_indices, std::vector<idx_type>& edge_element_buffer)
{
	// map stores for each halfedge the number of times it has been seen before
	std::map<std::tuple<idx_type, idx_type>, idx_type> halfedge_to_count;
	for (idx_type fi = 0; fi < faces.size(); ++fi) {
		idx_type last_ci = end_corner(fi) - 1;
		for (idx_type ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
			last_ci = ci;
			// construct halfedge with sorted vertex indices
			std::tuple<idx_type, idx_type> halfedge(last_ci, ci);
			if (ci < last_ci)
				std::swap(std::get<0>(halfedge), std::get<1>(halfedge));

			// lookup corner in map
			auto iter = halfedge_to_count.find(halfedge);

			// determine vertex index
			if (iter == halfedge_to_count.end()) {
				halfedge_to_count[halfedge] = 1;
				edge_element_buffer.push_back(vertex_indices.at(last_ci));
				edge_element_buffer.push_back(vertex_indices.at(ci));
			}
			else
				++halfedge_to_count[halfedge];
		}
	}
}

template <typename T>
class simple_mesh_obj_reader : public obj_reader_generic<T>
{
public:
	typedef typename simple_mesh<T>::idx_type idx_type;
protected:
	simple_mesh<T> &mesh;
public:
	simple_mesh_obj_reader(simple_mesh<T>& _mesh) : mesh(_mesh) {}
	/// overide this function to process a vertex
	void process_vertex(const v3d_type& p) { mesh.positions.push_back(p); }
	/// overide this function to process a texcoord
	void process_texcoord(const v2d_type& t) { mesh.tex_coords.push_back(t); }
	/// overide this function to process a color (this called for vc prefixes which is is not in the standard but for example used in pobj-files)
	void process_color(const color_type& c) { mesh.resize_colors(mesh.get_nr_colors() + 1); mesh.set_color(mesh.get_nr_colors()-1, c); }
	/// overide this function to process a normal
	void process_normal(const v3d_type& n) { mesh.normals.push_back(n); }
	/// overide this function to process a face, the indices start with 0
	void process_face(unsigned vcount, int *vertices, int *texcoords, int *normals)
	{
		convert_to_positive(vcount, vertices, texcoords, normals, mesh.positions.size(), mesh.normals.size(), mesh.tex_coords.size());
		mesh.faces.push_back(mesh.position_indices.size());
		if (get_current_group() != -1)
			mesh.group_indices.push_back(get_current_group());
		if (get_current_material() != -1)
			mesh.material_indices.push_back(get_current_material());
		for (idx_type i = 0; i < vcount; ++i) {
			mesh.position_indices.push_back(idx_type(vertices[i]));
			if (texcoords)
				mesh.tex_coord_indices.push_back(idx_type(texcoords[i]));
			if (normals)
				mesh.normal_indices.push_back(idx_type(normals[i]));
		}
	}
	/// overide this function to process a group given by name and parameter string
	void process_group(const std::string& name, const std::string& parameters)
	{
		mesh.group_names.push_back(name);
	}
	/// process a material definition. If a material with a certain name is overwritten, it will receive the same index
	void process_material(const cgv::media::illum::obj_material& mtl, unsigned idx)
	{
		if (idx >= mesh.materials.size())
			mesh.materials.resize(idx+1);
		mesh.materials[idx] = mtl;
	}
};

/// clear simple mesh
template <typename T>
void simple_mesh<T>::clear() 
{
	positions.clear(); 
	normals.clear(); 
	tex_coords.clear(); 
	position_indices.clear(); 
	tex_coord_indices.clear(); 
	normal_indices.clear(); 
	faces.clear(); 
}

/// read simple mesh from file
template <typename T>
bool simple_mesh<T>::read(const std::string& file_name)
{ 
	simple_mesh_obj_reader<T> reader(*this);
	return reader.read_obj(file_name);
}

/// compute the axis aligned bounding box
template <typename T>
typename simple_mesh<T>::box_type simple_mesh<T>::compute_box() const
{
	box_type box;
	for (const auto& p : positions)
		box.add_point(p);
	return box;
}

/// extract vertex attribute array and element array buffers for triangulation and edges in wireframe
template <typename T>
void simple_mesh<T>::extract_vertex_attribute_buffer(
	const std::vector<idx_type>& vertex_indices, 
	const std::vector<vec3i>& unique_triples, 
	bool include_tex_coords, bool include_normals, bool include_colors, 
	std::vector<T>& attrib_buffer)
{
	// correct inquiry in case data is missing
	include_tex_coords = include_tex_coords && !tex_coord_indices.empty() && !tex_coords.empty();
	include_normals = include_normals && !normal_indices.empty() && !normals.empty();
	include_colors = include_colors && has_colors();

	// determine number floats per vertex
	unsigned nr_floats = 3;
	nr_floats += include_tex_coords ? 2 : 0;
	nr_floats += include_normals ? 3 : 0;
	unsigned color_increment = 0;
	if (include_colors) {
		color_increment = (int)ceil((float)get_color_size() / sizeof(T));
		nr_floats += color_increment;
	}

	attrib_buffer.resize(nr_floats*unique_triples.size());
	T* data_ptr = &attrib_buffer.front();
	for (auto t : unique_triples) {
		*reinterpret_cast<vec3*>(data_ptr) = positions[t[0]];
		data_ptr += 3;
		if (include_tex_coords) {
			*reinterpret_cast<vec2*>(data_ptr) = tex_coords[t[1]];
			data_ptr += 2;
		}
		if (include_normals) {
			*reinterpret_cast<vec3*>(data_ptr) = normals[t[2]];
			data_ptr += 3;
		}
		if (include_colors) {
			put_color(t[0], data_ptr);
			data_ptr += color_increment;
		}
	}
}

template class simple_mesh<float>;
template class simple_mesh<double>;


		}
	}
}
