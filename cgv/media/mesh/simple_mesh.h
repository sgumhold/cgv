#pragma once

#include <cstdint>
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

template <typename T>
class CGV_API obj_loader_generic;

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
	/// define index quadruple type
	typedef cgv::math::fvec<idx_type, 4> vec4i;
	/// define material type
	typedef illum::textured_surface_material mat_type;
protected:
	std::vector<idx_type> position_indices;
	std::vector<idx_type> normal_indices;
	std::vector<idx_type> tangent_indices;
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
	/// move constructor
	simple_mesh_base(simple_mesh_base&& smb);
	/// assignment operator
	simple_mesh_base& operator=(const simple_mesh_base& smb);
	/// move assignment operator
	simple_mesh_base& operator=(simple_mesh_base&& smb);
	/// position count
	virtual idx_type get_nr_positions() const = 0;
	/**
	 * Create a new empty face to which new corners are added.
	 * 
	 * \return The index to the newly created face.
	 */
	idx_type start_face();
	/**
	 * Create a new corner with the given attributes.
	 * 
	 * \param [in] position_index the corner's position as index into an attribute vector
	 * \param [in] normal_index the corner's normal as index into an attribute vector, or -1 if no normal
	 * \param [in] tex_coord_index the corner's texture coordinate as index into an attribute vector, or -1 if no texture coordinate
	 * \return the index of the newly created corner
	 */
	idx_type new_corner(idx_type position_index, idx_type normal_index = -1, idx_type tex_coord_index = -1); //FIXME: -1 underflows unsigned int!
	/// return position index of corner
	idx_type c2p(idx_type ci) const { return position_indices[ci]; }
	/// return normal index of corner
	idx_type c2n(idx_type ci) const { return normal_indices[ci]; }
	/// return whether normal indices are stored
	bool has_normal_indices() const { return normal_indices.size() > 0 && normal_indices.size() == position_indices.size(); }
	/// return texture index of corner
	idx_type c2t(idx_type ci) const { return tex_coord_indices[ci]; }
	/// return whether texture coordinate indices are stored
	bool has_tex_coord_indices() const { return tex_coord_indices.size() > 0 && tex_coord_indices.size() == position_indices.size(); }
	/// return the number of faces
	idx_type get_nr_faces() const { return idx_type(faces.size()); }
	/// return the number of corners
	idx_type get_nr_corners() const { return idx_type(position_indices.size()); }
	/**
	 * Retrieve the vertex index of the first corner of a face.
	 *
	 * \param [in] fi Which face of simple_mesh\<T\>::faces.
	 * \return Index of the face's first vertex.
	 */
	idx_type begin_corner(idx_type fi) const { return faces[fi]; }
	/**
	 * Retrieve index of the vertex which follows the end corner of a face.
	 * 
	 * \param [in] fi Which face of simple_mesh\<T\>::faces.
	 * \return Index of the face's last vertex + 1.
	 */
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
	/**
	 * Calculate a permutation of face indices which sorts them by group and/or material.
	 * 
	 * \param [out] perm The sorting permutation.
	 * \param [in] by_group True if the faces should be sorted by group, false otherwise.
	 * \param [in] by_material True if the faces should be sorted by material, false otherwise.
	 * 
	 * \see simple_mesh::extract_triangle_element_buffer() for applying the sorting permutation
	 */
	void sort_faces(std::vector<idx_type>& perm, bool by_group = true, bool by_material = true) const;
	/**
	 * Transforms n individual vertex attribute indices into one list of unique index n-tuples.
	 * 
	 * Picture for example a cube where each quadratic face has a color. There would be only
	 * eight vertex positions. However, each of these vertices belongs to three faces and would therefore need to carry
	 * three distinct colors. To solve this issue all the unique combinations of attribute indicies get recorded
	 * into a list. The new face primitives are then referenced by indexing into this list of unique index n-tuples.
	 * 
	 * See https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/ for further details.
	 *
	 * \param [out] vertex_indices will be filled with indicies into the unique tuple list.
	 * \param [out] unique_tuples will be filled with all the unique n-tuples.
	 * \param [in,out] include_tex_coords_ptr if nullptr then texture coordinates won't be included in the n-tuples.
	 * Otherwise the pointed to bool will be set to true if the mesh even contains texture coordinates or false if not.
	 * \param [in,out] include_normals_ptr if nullptr then normals won't be included in the n-tuples.
	 * Otherwise the pointed to bool will be set to true if the mesh even contains normals or false if not.
	 * \param [in,out] include_tangents_ptr if nullptr then tangents won't be included in the n-tuples.
	 * Otherwise the pointed to bool will be set to true if the mesh even contains texture coordinates or false if not.
	 * 
	 * \see simple_mesh::extract_triangle_element_buffer()
	 * \see simple_mesh::extract_wireframe_element_buffer()
	 */
	void merge_indices(std::vector<idx_type>& vertex_indices, std::vector<vec4i>& unique_tuples,
					   bool* include_tex_coords_ptr = 0, bool* include_normals_ptr = 0, bool* include_tangents_ptr = 0) const;
	/**
	 * Extract element array buffers for triangulation.
	 *
	 * \param [in] vertex_indices Contains indices into a list of vertices.
	 * \param [out] triangle_element_buffer Stores the vertex indices which make up a triangulated mesh.
	 * \param [in] face_permutation_ptr If nullptr the faces will be traversed in successive fashion. Otherwise the given index-permutation is used.
	 * \param [out] material_group_start_ptr Will be filled with material group indices if not nullptr.
	 * 
	 * \see simple_mesh::merge_indices()
	 * \see simple_mesh::sort_faces() for getting a permutation
	 */
	void extract_triangle_element_buffer(const std::vector<idx_type>& vertex_indices,
										 std::vector<idx_type>& triangle_element_buffer,
										 const std::vector<idx_type>* face_permutation_ptr = 0,
										 std::vector<vec3i>* material_group_start_ptr = 0) const;
	/**
	 * Extract element array buffers for edges in wireframe.
	 * 
	 * \param [in] vertex_indices Contains indices into a list of vertices.
	 * \param [out] edge_element_buffer Stores the vertex indices which make up a wireframed mesh.
	 * 
	 * \see simple_mesh::merge_indices()
	 */
	void extract_wireframe_element_buffer(const std::vector<idx_type>& vertex_indices,
										  std::vector<idx_type>& edge_element_buffer) const;
	/// compute a index vector storing the inv corners per corner and optionally index vectors with per position corner index, per corner next and or prev corner index (implementation assumes closed manifold connectivity)
	void compute_inv(std::vector<uint32_t>& inv, std::vector<uint32_t>* p2c_ptr = 0, std::vector<uint32_t>* next_ptr = 0, std::vector<uint32_t>* prev_ptr = 0) const;
	/// given the inv corners compute index vector per corner its edge index and optionally per edge its corner index and return edge count (implementation assumes closed manifold connectivity)
	uint32_t compute_c2e(const std::vector<uint32_t>& inv, std::vector<uint32_t>& c2e, std::vector<uint32_t>* e2c_ptr = 0) const;
	/// compute index vector with per corner its face index
	void compute_c2f(std::vector<uint32_t>& c2f) const;
};

/// the simple_mesh class is templated over the coordinate type that defaults to float
template <typename T = float>
class CGV_API simple_mesh : public simple_mesh_base
{
public:
	using numeric_type = T;
	
	/// type of axis aligned 3d box
	typedef simple_mesh<T> mesh_type;
	/// type of axis aligned 3d box
	typedef typename cgv::media::axis_aligned_box<T, 3> box_type;
	/// type of 4d vector
	typedef typename cgv::math::fvec<T, 4> vec4;
	/// type of 3d vector
	typedef typename cgv::math::fvec<T, 3> vec3;
	/// type of 2d vector
	typedef typename cgv::math::fvec<T, 2> vec2;
	/// linear transformation 
	typedef typename cgv::math::fmat<T, 3, 3> mat3;
	/// linear transformation 
	typedef typename cgv::math::fmat<T, 4, 4> mat4;
	/// color type used in surface materials
	typedef typename illum::surface_material::color_type clr_type;
protected:
	friend class simple_mesh_obj_reader<T>;
	std::vector<vec3>  positions;
	std::vector<vec3>  normals;
	std::vector<vec3>  tangents;
	std::vector<vec2>  tex_coords;

	vec3 compute_normal(const vec3& p0, const vec3& p1, const vec3& p2);
public:
	/// copy constructor
	simple_mesh(const simple_mesh<T>& sm);
	/// move constructor
	simple_mesh(simple_mesh<T>&& sm);
	/// assignment operator
	simple_mesh(const std::string& conway_notation = "");
	/// assignment operator
	simple_mesh<T>& operator= (const simple_mesh<T>& sm);
	/// move assignment operator
	simple_mesh<T>& operator= (simple_mesh<T>&& sm);
	/// clear simple mesh
	void clear();

	/// add a new position and return position index
	idx_type new_position(const vec3& p) { positions.push_back(p); return idx_type(positions.size()-1); }
	/// access to positions
	idx_type get_nr_positions() const { return idx_type(positions.size()); }
	vec3& position(idx_type pi) { return positions[pi]; }
	const vec3& position(idx_type pi) const { return positions[pi]; }
	const std::vector<vec3>& get_positions() const { return positions; }
	std::vector<vec3>& ref_positions() { return positions; }

	/// add a new normal and return normal index
	idx_type new_normal(const vec3& n) { normals.push_back(n); return idx_type(normals.size()-1); }
	/// access to normals
	bool has_normals() const { return get_nr_normals() > 0; }
	idx_type get_nr_normals() const { return idx_type(normals.size()); }
	vec3& normal(idx_type ni) { return normals[ni]; }
	const vec3& normal(idx_type ni) const { return normals[ni]; }
	const std::vector<vec3>& get_normals() const { return normals; }

	/// add a new tangent and return tangent index
	idx_type new_tangent(const vec3& tc) { tangents.push_back(tc); return idx_type(tangents.size() - 1); }
	/// access to tangents
	bool has_tangents() const { return get_nr_tangents() > 0; }
	idx_type get_nr_tangents() const { return idx_type(tangents.size()); }
	vec3& tangent(idx_type ti) { return tangents[ti]; }
	const vec3& tangent(idx_type ti) const { return tangents[ti]; }

	/// add a new texture coordinate and return texture coordinate index
	idx_type new_tex_coord(const vec2& tc) { tex_coords.push_back(tc); return idx_type(tex_coords.size() - 1); }
	/// access to texture coordinates
	bool has_tex_coords() const { return get_nr_tex_coords() > 0; }
	idx_type get_nr_tex_coords() const { return idx_type(tex_coords.size()); }
	vec2& tex_coord(idx_type ti) { return tex_coords[ti]; }
	const vec2& tex_coord(idx_type ti) const { return tex_coords[ti]; }

	/// compute face center
	vec3 compute_face_center(idx_type fi) const;
	/// compute per face normals (ensure that per corner normal indices are set correspondingly)
	void compute_face_normals(bool construct_normal_indices = true);
	/// compute per face tangents (ensure that per corner tangent indices are set correspondingly)
	void compute_face_tangents(bool construct_tangent_indices = true);
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
	/// construct from obj loader
	void construct(const obj_loader_generic<T>& loader, bool copy_grp_info, bool copy_material_info);
	/// read simple mesh from file (currently only obj and stl are supported)
	bool read(const std::string& file_name);
	/// write simple mesh to file (currently only obj is supported)
	bool write(const std::string& file_name) const;
	/**
	 * Extract vertex attribute array and element array buffers for triangulation and edges in wireframe.
	 * 
	 * \param unique_quadruples A list of unique n-tuples where each entry is an index into attribute vectors of simple_mesh.
	 * \param include_tex_coords True if texture coordinates should be written into the vertex buffer, false otherwise.
	 * \param include_normals True if normals should be written into the vertex buffer, false otherwise.
	 * \param include_tangents True if tangents should be written into the vertex buffer, false otherwise.
	 * \param attrib_buffer will contain the vertex attribute data in interleaved form.
	 * \param include_colors_ptr If nullptr then the vertex buffer won't contain colors, otherwise it will be set to true if the mesh even contains colors or false if not.
	 * \param num_floats_in_vertex If not nullptr will be set to the number of floats which make up one vertex with all its attributes.
	 * \return The size of one color in bytes.
	 */
	unsigned extract_vertex_attribute_buffer(const std::vector<vec4i>& unique_quadruples, bool include_tex_coords,
											 bool include_normals, bool include_tangents, std::vector<T>& attrib_buffer,
											 bool* include_colors_ptr = 0, int* num_floats_in_vertex = nullptr) const;
	/// apply transformation to mesh
	void transform(const mat3& linear_transformation, const vec3& translation);
	/// apply transformation to mesh with given inverse linear transformation
	void transform(const mat3& linear_transform, const vec3& translation, const mat3& inverse_linear_transform);
};

		}
	}
}

#include <cgv/config/lib_end.h>