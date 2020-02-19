#include "simple_mesh.h"
#include <cgv/math/inv.h>
#include <cgv/media/mesh/obj_reader.h>
#include <cgv/math/bucket_sort.h>
#include <fstream>

namespace cgv {
	namespace media {
		namespace mesh {

/// create a new empty face to which new corners are added and return face index
simple_mesh_base::idx_type simple_mesh_base::start_face()
{
	faces.push_back((cgv::type::uint32_type)position_indices.size());
	if (!materials.empty())
		material_indices.push_back(idx_type(materials.size()) - 1);
	if (!group_names.empty())
		group_indices.push_back(idx_type(group_names.size()) - 1);
	return idx_type(faces.size() - 1);
}
/// create a new corner from position, optional normal and optional tex coordinate indices and return corner index
simple_mesh_base::idx_type simple_mesh_base::new_corner(idx_type position_index, idx_type normal_index, idx_type tex_coord_index)
{
	position_indices.push_back(position_index);
	if (normal_index != -1)
		normal_indices.push_back(normal_index);
	if (tex_coord_index != -1)
		tex_coord_indices.push_back(tex_coord_index);
	return idx_type(position_indices.size());
}

/// revert face orientation
void simple_mesh_base::revert_face_orientation()
{
	bool nmls = position_indices.size() == normal_indices.size();
	bool tcs  = position_indices.size() == tex_coord_indices.size();
	for (idx_type fi = 0; fi < get_nr_faces(); ++fi) {
		idx_type ci = begin_corner(fi);
		idx_type cj = end_corner(fi);
		while (ci + 1 < cj) {
			--cj;
			std::swap(position_indices[ci], position_indices[cj]);
			if (nmls)
				std::swap(normal_indices[ci], normal_indices[cj]);
			if (tcs)
				std::swap(tex_coord_indices[ci], tex_coord_indices[cj]);
			++ci;
		}
	}
}

/// sort faces by group and material indices with two bucket sorts
void simple_mesh_base::sort_faces(std::vector<idx_type>& perm, bool by_group, bool by_material) const
{
	if (by_group && by_material) {
		std::vector<idx_type> perm0;
		cgv::math::bucket_sort(group_indices, get_nr_groups(), perm0);
		cgv::math::bucket_sort(material_indices, get_nr_materials(), perm, &perm0);
	}
	else if (by_group)
		cgv::math::bucket_sort(group_indices, get_nr_groups(), perm);
	else
		cgv::math::bucket_sort(material_indices, get_nr_materials(), perm);
}

/// merge the three indices into one index into a vector of unique index triples
void simple_mesh_base::merge_indices(std::vector<idx_type>& indices, std::vector<vec3i>& unique_triples, bool* include_tex_coords_ptr, bool* include_normals_ptr) const
{
	bool include_tex_coords = false;
	if (include_tex_coords_ptr)
		*include_tex_coords_ptr = include_tex_coords = (tex_coord_indices.size() > 0) && *include_tex_coords_ptr;

	bool include_normals    = false;
	if (include_normals_ptr)
		*include_normals_ptr = include_normals = (normal_indices.size() > 0) && *include_normals_ptr;

	std::map<std::tuple<idx_type, idx_type, idx_type>, idx_type> corner_to_index;
	for (idx_type ci = 0; ci < position_indices.size(); ++ci) {
		// construct corner
		vec3i c(position_indices[ci], 
			    (include_tex_coords && ci < tex_coord_indices.size()) ? tex_coord_indices[ci] : 0, 
			    (include_normals && ci < normal_indices.size()) ? normal_indices[ci] : 0);
		std::tuple<idx_type, idx_type, idx_type> triple(c(0),c(1),c(2));
		// look corner up in map
		auto iter = corner_to_index.find(triple);
		// determine vertex index
		idx_type vi;
		if (iter == corner_to_index.end()) {
			vi = idx_type(unique_triples.size());
			corner_to_index[triple] = vi;
			unique_triples.push_back(c);
		}
		else
			vi = iter->second;

		indices.push_back(vi);
	}
}

/// extract element array buffers for triangulation
void simple_mesh_base::extract_triangle_element_buffer(
	const std::vector<idx_type>& vertex_indices, std::vector<idx_type>& triangle_element_buffer, 
	const std::vector<idx_type>* face_perm_ptr, std::vector<vec3i>* material_group_start_ptr) const
{
	idx_type mi = idx_type(-1);
	idx_type gi = idx_type(-1);
	// construct triangle element buffer
	for (idx_type fi = 0; fi < faces.size(); ++fi) {
		idx_type fj = face_perm_ptr ? face_perm_ptr->at(fi) : fi;
		if (material_group_start_ptr) {
			if (mi != material_indices[fj] || gi != group_indices[fj]) {
				mi = material_indices[fj];
				gi = group_indices[fj];
				material_group_start_ptr->push_back(vec3i(mi, gi, idx_type(triangle_element_buffer.size())));
			}
		}
		if (face_degree(fj) == 3) {
			for (idx_type ci = begin_corner(fj); ci < end_corner(fj); ++ci)
				triangle_element_buffer.push_back(vertex_indices.at(ci));
		}
		else {
			// in case of non triangular faces do simplest triangulation approach that assumes convexity of faces
			for (idx_type ci = begin_corner(fj) + 2; ci < end_corner(fj); ++ci) {
				triangle_element_buffer.push_back(vertex_indices.at(begin_corner(fj)));
				triangle_element_buffer.push_back(vertex_indices.at(ci - 1));
				triangle_element_buffer.push_back(vertex_indices.at(ci));
			}
		}
	}
}

/// extract element array buffers for edges in wireframe
void simple_mesh_base::extract_wireframe_element_buffer(const std::vector<idx_type>& vertex_indices, std::vector<idx_type>& edge_element_buffer) const
{
	// map stores for each halfedge the number of times it has been seen before
	std::map<std::tuple<idx_type, idx_type>, idx_type> halfedge_to_count;
	for (idx_type fi = 0; fi < faces.size(); ++fi) {
		idx_type last_vi = vertex_indices.at(end_corner(fi) - 1);
		for (idx_type ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
			// construct halfedge with sorted vertex indices
			idx_type vi = vertex_indices.at(ci);
			std::tuple<idx_type, idx_type> halfedge(last_vi, vi);
			if (vi < last_vi)
				std::swap(std::get<0>(halfedge), std::get<1>(halfedge));

			// lookup corner in map
			auto iter = halfedge_to_count.find(halfedge);

			// determine vertex index
			if (iter == halfedge_to_count.end()) {
				halfedge_to_count[halfedge] = 1;
				edge_element_buffer.push_back(last_vi);
				edge_element_buffer.push_back(vi);
			}
			else
				++halfedge_to_count[halfedge];
			last_vi = vi;
		}
	}
}

template <typename T>
class simple_mesh_obj_reader : public obj_reader_generic<T>
{
public:
	/// type of coordinates
	typedef T crd_type;
	/// type used to store texture coordinates
	typedef cgv::math::fvec<T,2> v2d_type;
	/// type used to store positions and normal vectors
	typedef cgv::math::fvec<T,3> v3d_type;
	/// type used for rgba colors
	typedef illum::obj_material::color_type color_type;

	typedef typename simple_mesh<T>::idx_type idx_type;
protected:
	simple_mesh<T> &mesh;
public:
	simple_mesh_obj_reader(simple_mesh<T>& _mesh) : mesh(_mesh) {}
	/// overide this function to process a vertex
	void process_vertex(const v3d_type& p) { mesh.positions.push_back(p); }
	/// overide this function to process a texcoord
	void process_texcoord(const v2d_type& t) { mesh.tex_coords.push_back(v2d_type(t(0),t(1))); }
	/// overide this function to process a color (this called for vc prefixes which is is not in the standard but for example used in pobj-files)
	void process_color(const color_type& c) { mesh.resize_colors(mesh.get_nr_colors() + 1); mesh.set_color(mesh.get_nr_colors()-1, c); }
	/// overide this function to process a normal
	void process_normal(const v3d_type& n) { mesh.normals.push_back(n); }
	/// overide this function to process a face, the indices start with 0
	void process_face(unsigned vcount, int *vertices, int *texcoords, int *normals)
	{
		convert_to_positive(vcount, vertices, texcoords, normals, unsigned(mesh.positions.size()), unsigned(mesh.normals.size()), unsigned(mesh.tex_coords.size()));
		mesh.faces.push_back(idx_type(mesh.position_indices.size()));
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
	group_indices.clear();
	group_names.clear();
	material_indices.clear();
	materials.clear();
	destruct_colors();
}

/// read simple mesh from file
template <typename T>
bool simple_mesh<T>::read(const std::string& file_name)
{ 
	simple_mesh_obj_reader<T> reader(*this);
	return reader.read_obj(file_name);
}

/// write simple mesh to file (currently only obj is supported)
template <typename T>
bool simple_mesh<T>::write(const std::string& file_name) const
{
	std::ofstream os(file_name);
	if (os.fail())
		return false;
	for (const auto& p : positions)
		os << "v " << p << std::endl;
	for (const auto& t : tex_coords)
		os << "vt " << t << std::endl;
	for (const auto& n : normals)
		os << "vn " << n << std::endl;

	bool nmls = position_indices.size() == normal_indices.size();
	bool tcs = position_indices.size() == tex_coord_indices.size();

	for (idx_type fi = 0; fi < faces.size(); ++fi) {
		os << "f";
		for (idx_type ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
			os << " " << position_indices[ci] + 1;
			if (!nmls && !tcs)
				continue;
			os << "/";
			if (tcs)
				os << tex_coord_indices[ci] + 1;
			if (nmls)
				os << "/" << normal_indices[ci] + 1;
		}
		os << "\n";
	}
	return true;
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
/// compute vertex normals by averaging triangle normals
template <typename T>
void simple_mesh<T>::compute_vertex_normals()
{
	// clear previous normal info
	if (has_normals())
		normals.clear();
	// copy position indices to normals
	normal_indices = position_indices;
	// initialize normals to null vectors
	normals.resize(positions.size(), vec3(0.0f));
	// iterate faces
	for (idx_type fi = 0; fi < get_nr_faces(); ++fi) {
		idx_type c0 = begin_corner(fi);
		idx_type ce = end_corner(fi);
		vec3 p0 = position(position_indices[c0]);
		vec3 dj = position(position_indices[c0 + 1]) - p0;
		vec3 nml(0.0f);
		for (idx_type ci = c0+2; ci < ce; ++ci) {
			vec3 di = position(position_indices[ci]) - p0;
			nml += cross(dj, di);
			dj = di;
		}
		T nl = nml.length();
		if (nl > 1e-8f) {
			nml *= 1.0f/nl;
			for (idx_type ci = c0; ci < ce; ++ci)
				normal(normal_indices[ci]) += nml;
		}
	}
	for (auto& n : normals)
		n.normalize();
}

/// extract vertex attribute array and element array buffers for triangulation and edges in wireframe
template <typename T>
unsigned simple_mesh<T>::extract_vertex_attribute_buffer(
	const std::vector<idx_type>& vertex_indices, 
	const std::vector<vec3i>& unique_triples, 
	bool include_tex_coords, bool include_normals, 
	std::vector<T>& attrib_buffer, bool* include_colors_ptr) const
{
	// correct inquiry in case data is missing
	include_tex_coords = include_tex_coords && !tex_coord_indices.empty() && !tex_coords.empty();
	include_normals = include_normals && !normal_indices.empty() && !normals.empty();
	bool include_colors = false;
	if (include_colors_ptr)
		*include_colors_ptr = include_colors = has_colors() && *include_colors_ptr;

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
	return color_increment;
}

/// apply transformation to mesh
template <typename T>
void simple_mesh<T>::transform(const mat3& linear_transformation, const vec3& translation)
{
	mat3 inverse_linear_transform = inv(linear_transformation);
	transform(linear_transformation, translation, inverse_linear_transform);
}

/// apply transformation to mesh with given inverse linear transformation
template <typename T>
void simple_mesh<T>::transform(const mat3& linear_transform, const vec3& translation, const mat3& inverse_linear_transform)
{
	for (auto& p : positions)
		p = linear_transform * p + translation;
	for (auto& n : normals)
		n = n * inverse_linear_transform;
}

template class simple_mesh<float>;
template class simple_mesh<double>;


		}
	}
}
