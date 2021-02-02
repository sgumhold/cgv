#pragma once

#include <vector>
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/utils/file.h>
#include <cgv/media/illum/textured_surface_material.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/media/colored_model.h>

#include "../lib_begin.h"

namespace cgv {
	namespace media {
		namespace mesh {
			
template <typename T>
class simple_mesh_obj_reader;

/** coordinate type independent base class of simple mesh data structure that handles indices and colors. */
class CGV_API simple_mesh_base : public colored_model
{
public:
	/// define index type
	typedef cgv::type::uint32_type idx_type;
	/// define index pair type
	typedef cgv::math::fvec<idx_type, 2> vec2i;
	/// define index triple type
	typedef cgv::math::fvec<idx_type, 3> vec3i;
	/// define material type
	typedef illum::textured_surface_material mat_type;
protected:
	std::vector<idx_type> position_indices;
	std::vector<idx_type> normal_indices;
	std::vector<idx_type> tex_coord_indices;
	std::vector<idx_type> faces;
	std::vector<idx_type> group_indices;
	std::vector<std::string> group_names;
	std::vector<idx_type> material_indices;
	std::vector<mat_type> materials;
public:
	/// default constructor
	simple_mesh_base();
	/// copy constructor
	simple_mesh_base(const simple_mesh_base& smb);
	/// assignment operator
	simple_mesh_base& operator=(const simple_mesh_base& smb);
	/// position count
	virtual idx_type get_nr_positions() const = 0;
	/// create a new empty face to which new corners are added and return face index
	idx_type start_face();
	/// create a new corner from position, optional normal and optional tex coordinate indices and return corner index
	idx_type new_corner(idx_type position_index, idx_type normal_index = -1, idx_type tex_coord_index = -1);
	/// return position index of corner
	idx_type c2p(idx_type ci) const { return position_indices[ci]; }
	/// return normal index of corner
	idx_type c2n(idx_type ci) const { return normal_indices[ci]; }
	/// return texture index of corner
	idx_type c2t(idx_type ci) const { return tex_coord_indices[ci]; }
	/// return the number of faces
	idx_type get_nr_faces() const { return idx_type(faces.size()); }
	/// return the number of corners
	idx_type get_nr_corners() const { return idx_type(position_indices.size()); }
	/// return index of first corner of face with index fi
	idx_type begin_corner(idx_type fi) const { return faces[fi]; }
	/// return index of end corner (one after the last one) of face with index fi
	idx_type end_corner(idx_type fi) const { return fi + 1 == faces.size() ? idx_type(position_indices.size()) : faces[fi + 1]; }
	/// return number of edges/corners of face with index fi
	idx_type face_degree(idx_type fi) const { return end_corner(fi) - begin_corner(fi); }
	/// return number of materials in mesh
	size_t get_nr_materials() const { return materials.size(); }
	/// add a new material and return its index
	idx_type new_material() { materials.push_back(mat_type()); return idx_type(materials.size() - 1); }
	/// return const reference to i-th material
	const mat_type& get_material(size_t i) const { return materials[i]; }
	/// return reference to i-th material
	mat_type& ref_material(size_t i) { return materials[i]; }
	/// return material index of given face
	const idx_type& material_index(idx_type fi) const { return material_indices[fi]; }
	/// return reference to material index of given face
	idx_type& material_index(idx_type fi) { return material_indices[fi]; }
	/// return number of face groups
	size_t get_nr_groups() const { return group_names.size(); }
	/// return the name of the i-th face group
	const std::string& group_name(size_t i) const { return group_names[i]; }
	/// set a new group name
	std::string& group_name(size_t i) { return group_names[i]; }
	/// add a new group and return its index
	idx_type new_group(const std::string& name) { group_names.push_back(name); return idx_type(group_names.size() - 1); }
	/// return group index of given face
	const idx_type& group_index(idx_type fi) const { return group_indices[fi]; }
	/// return reference to group index of given face
	idx_type& group_index(idx_type fi) { return group_indices[fi]; }
	/// revert face orientation
	void revert_face_orientation();
	/// sort faces by group and material indices with two bucket sorts
	void sort_faces(std::vector<idx_type>& perm, bool by_group = true, bool by_material = true) const;
	/// merge the three indices into one index into a vector of unique index triples
	void merge_indices(std::vector<idx_type>& vertex_indices, std::vector<vec3i>& unique_triples, bool* include_tex_coords_ptr = 0, bool* include_normals_ptr = 0) const;
	/// extract element array buffers for triangulation
	void extract_triangle_element_buffer(const std::vector<idx_type>& vertex_indices, std::vector<idx_type>& triangle_element_buffer, 
		const std::vector<idx_type>* face_perm_ptr = 0, std::vector<vec3i>* material_group_start_ptr = 0) const;
	/// extract element array buffers for edges in wireframe
	void extract_wireframe_element_buffer(const std::vector<idx_type>& vertex_indices, std::vector<idx_type>& edge_element_buffer) const;
	/// compute a index vector storing the inv corners per corner and optionally index vectors with per position corner index, per corner next and or prev corner index (implementation assumes closed manifold connectivity)
	void compute_inv(std::vector<uint32_t>& inv, std::vector<uint32_t>* p2c_ptr = 0, std::vector<uint32_t>* next_ptr = 0, std::vector<uint32_t>* prev_ptr = 0);
	/// given the inv corners compute index vector per corner its edge index and optionally per edge its corner index and return edge count (implementation assumes closed manifold connectivity)
	uint32_t compute_c2e(const std::vector<uint32_t>& inv, std::vector<uint32_t>& c2e, std::vector<uint32_t>* e2c_ptr = 0);
	/// compute index vector with per corner its face index
	void compute_c2f(std::vector<uint32_t>& c2f);
};

/// the simple_mesh class is templated over the coordinate type that defaults to float
template <typename T = float>
class CGV_API simple_mesh : public simple_mesh_base
{
public:
	/// type of axis aligned 3d box
	typedef typename simple_mesh<T> mesh_type;
	/// type of axis aligned 3d box
	typedef typename cgv::media::axis_aligned_box<T, 3> box_type;
	/// type of 3d vector
	typedef typename cgv::math::fvec<T, 3> vec3;
	/// type of 2d vector
	typedef typename cgv::math::fvec<T, 2> vec2;
	/// linear transformation 
	typedef typename cgv::math::fmat<T, 3, 3> mat3;
	/// color type used in surface materials
	typedef typename illum::surface_material::color_type clr_type;
	/// textured surface materials are supported by mat_type
	typedef typename illum::textured_surface_material mat_type;
	/// 32bit index
	typedef cgv::type::uint32_type idx_type;
protected:
	friend class simple_mesh_obj_reader<T>;
	std::vector<vec3>  positions;
	std::vector<vec3>  normals;
	std::vector<vec2>  tex_coords;

	vec3 compute_normal(const vec3& p0, const vec3& p1, const vec3& p2);
public:
	/// construct from string corresponding to Conway notation (defaults to empty mesh)
	simple_mesh(const simple_mesh<T>& sm);
	/// construct from string corresponding to Conway notation (defaults to empty mesh)
	simple_mesh(const std::string& conway_notation = "");
	/// construct from string corresponding to Conway notation (defaults to empty mesh)
	simple_mesh<T>& operator = (const simple_mesh<T>& sm);
	/// clear simple mesh
	void clear();

	/// add a new position and return position index
	idx_type new_position(const vec3& p) { positions.push_back(p); return idx_type(positions.size()-1); }
	/// access to positions
	idx_type get_nr_positions() const { return idx_type(positions.size()); }
	vec3& position(idx_type pi) { return positions[pi]; }
	const vec3& position(idx_type pi) const { return positions[pi]; }
	const std::vector<vec3>& get_positions() const { return positions; }

	/// add a new normal and return normal index
	idx_type new_normal(const vec3& n) { normals.push_back(n); return idx_type(normals.size()-1); }
	/// access to normals
	bool has_normals() const { return get_nr_normals() > 0; }
	idx_type get_nr_normals() const { return idx_type(normals.size()); }
	vec3& normal(idx_type ni) { return normals[ni]; }
	const vec3& normal(idx_type ni) const { return normals[ni]; }

	/// add a new normal and return normal index
	idx_type new_tex_coord(const vec2& tc) { tex_coords.push_back(tc); return idx_type(tex_coords.size() - 1); }
	/// access to texture coordinates
	bool has_tex_coords() const { return get_nr_tex_coords() > 0; }
	idx_type get_nr_tex_coords() const { return idx_type(tex_coords.size()); }
	vec2& tex_coord(idx_type ti) { return tex_coords[ti]; }
	const vec2& tex_coord(idx_type ti) const { return tex_coords[ti]; }
	/// compute per face normals (ensure that per corner normal indices are set correspondingly)
	void compute_face_normals();
	/// Conway ambo operator
	void ambo();
	/// Conway truncate operator
	void truncate(T lambda = 0.33333f);
	/// Conway snub operator
	void snub(T lambda = 0.33333f);
	/// Conway dual operator
	void dual();
	/// Conway gyro operator
	void gyro(T lambda = 0.3333f);
	/// Conway join operator
	void join();
	/// Conway ortho operator
	void ortho();
	/// construct new mesh according to Conway polyhedron notation: [a|t|s|d|g|j|o]*[T|C|O|D|I] which is evaluated from right to left and last capital letter is Platonic solid and lowercase letters are Conway operations
	void construct_conway_polyhedron(const std::string& conway_notation);

	/// compute the axis aligned bounding box
	box_type compute_box() const;
	/// compute vertex normals by averaging triangle normals
	void compute_vertex_normals();
	/// read simple mesh from file (currently only obj is supported)
	bool read(const std::string& file_name);
	/// write simple mesh to file (currently only obj is supported)
	bool write(const std::string& file_name) const;
	/// extract vertex attribute array, return size of color in bytes
	unsigned extract_vertex_attribute_buffer(
		const std::vector<idx_type>& vertex_indices,
		const std::vector<vec3i>& unique_triples,
		bool include_tex_coords, bool include_normals, 
		std::vector<T>& attrib_buffer, bool *include_colors_ptr = 0) const;
	/// apply transformation to mesh
	void transform(const mat3& linear_transformation, const vec3& translation);
	/// apply transformation to mesh with given inverse linear transformation
	void transform(const mat3& linear_transform, const vec3& translation, const mat3& inverse_linear_transform);
};

		}
	}
}

#include <cgv/config/lib_end.h>