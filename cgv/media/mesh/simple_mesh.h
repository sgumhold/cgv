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
	typedef cgv::math::fvec<idx_type,2> idx2_type;
	/// define index triple type
	typedef cgv::math::fvec<idx_type,3> idx3_type;
	/// define index quadruple type
	typedef cgv::math::fvec<idx_type,4> idx4_type;
	/// define material type
	typedef illum::textured_surface_material mat_type;
	/// different mesh attributes
	enum class attribute_type {
		begin=0, position=0, texcoords=1, normal=2, tangent=3, color=4, end=5
	};
	// flags to define a selection of attributes
	enum AttributeFlags {
		AF_position = 1, /// vertex position 
		AF_texcoords = 2, /// texture coordinates
		AF_normal = 4, /// surface normal
		AF_tangent = 8, /// tangent vectors for u coordinate
		AF_color = 16  /// vertex colors (uses position indexing)
	};
	static std::string get_attribute_name(attribute_type attr);
	static AttributeFlags get_attribute_flag(attribute_type attr);
	virtual          bool  has_attribute(attribute_type attr) const = 0;
	virtual const uint8_t* get_attribute_ptr(attribute_type attr, idx_type ai = 0) const = 0;
	virtual        size_t  get_attribute_size(attribute_type attr) const = 0;
	virtual        size_t  get_attribute_offset(attribute_type attr) const = 0;
protected:
	std::vector<idx_type> position_indices;
	std::vector<idx_type> tex_coord_indices;
	std::vector<idx_type> normal_indices;
	std::vector<idx_type> tangent_indices;
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
	/// return the size of one coordinate in bytes
	virtual uint32_t get_coord_size() const = 0;
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
	 * \param [out] vertex_indices will be filled per corner with index into the unique tuple list.
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
	void merge_indices(std::vector<idx_type>& vertex_indices, std::vector<idx4_type>& unique_tuples,
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
										 std::vector<idx3_type>* material_group_start_ptr = 0) const;
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
	/// extract vertex attribute buffer for the given flags and return size of vertex in bytes
	idx_type extract_vertex_attribute_buffer_base(const std::vector<idx4_type>& unique_quadruples, AttributeFlags& flags, std::vector<uint8_t>& attrib_buffer) const;
	//! Do inverse matching of half-edges.
	/*! For this corners are converted to the half - edges that point in winding order
		away from the corner. The inverse matching result is stored in the vector \c inv
		with one index per corner. This index is -1 for unmatched half-edges and the 
		index of the corner corresponding to matched half-edges. For non-manifold edges
		two strategies are supported: cyclic linking (\c link_non_manifold_edges = true)
		or cutting into unmatched half-edges (\c link_non_manifold_edges = false).
		The function fills the optionally provided vectors
		- \c p2c ... per position the index of one incident corner
		- \c next ... per corner the index of the next corner in the face
		- \c prev ... per corner the index of the prev corner in the face
		- \c unmatched ... corners with unmatched half-edges 
		- \c non_manifold ... one corner index per non-manifold edge
		- \c unmatched_elements ... position indices of unmatched half-edges
		- \c non_manifold_elements ... position indices of non-manifold edges 
		The function returns the number of interior manifold edges */
	idx_type compute_inv(
		std::vector<idx_type>& inv,
		bool link_non_manifold_edges = false,
		std::vector<idx_type>* p2c_ptr = 0, 
		std::vector<idx_type>* next_ptr = 0, 
		std::vector<idx_type>* prev_ptr = 0,
		std::vector<idx_type>* unmatched = 0,
		std::vector<idx_type>* non_manifold = 0,
		std::vector<idx_type>* unmatched_elements = 0,
		std::vector<idx_type>* non_manifold_elements = 0) const;
	/// given the inv corners compute vector storing per corner the edge index and optionally per edge one corner index and return edge count (implementation assumes closed manifold connectivity)
	idx_type compute_c2e(const std::vector<idx_type>& inv, std::vector<idx_type>& c2e, std::vector<idx_type>* e2c_ptr = 0) const;
	/// compute index vector with per corner its face index
	void compute_c2f(std::vector<idx_type>& c2f) const;
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
	/// type of 2d vector
	typedef typename cgv::math::fvec<T, 2> vec2_type;
	/// type of 3d vector
	typedef typename cgv::math::fvec<T, 3> vec3_type;
	/// linear transformation 
	typedef typename cgv::math::fmat<T, 3, 3> mat3_type;
protected:
	friend class simple_mesh_obj_reader<T>;
	std::vector<vec3_type>  positions;
	std::vector<vec3_type>  normals;
	std::vector<vec3_type>  tangents;
	std::vector<vec2_type>  tex_coords;
	bool  has_attribute(attribute_type attr) const {
		switch (attr) {
		case attribute_type::position:  return true;
		case attribute_type::texcoords: return has_tex_coords() && has_tex_coord_indices();
		case attribute_type::normal:    return has_normals() && has_normal_indices();
		case attribute_type::tangent:   return has_tangents();
		case attribute_type::color:     return has_colors();
		default: /* see below */;
		}
		return false;
	}
	const uint8_t* get_attribute_ptr(attribute_type attr, idx_type ai = 0) const {
		switch (attr) {
		case attribute_type::position:  return reinterpret_cast<const uint8_t*>( &positions[ai]);
		case attribute_type::texcoords: return reinterpret_cast<const uint8_t*>(&tex_coords[ai]);
		case attribute_type::normal:    return reinterpret_cast<const uint8_t*>(   &normals[ai]);
		case attribute_type::tangent:   return reinterpret_cast<const uint8_t*>(  &tangents[ai]);
		case attribute_type::color:     return reinterpret_cast<const uint8_t*>(get_color_data_ptr()) + ai * get_color_size();
		default: /* see below */;
		}
		return nullptr;
	}
	size_t get_attribute_size(attribute_type attr) const {
		switch (attr) {
		case attribute_type::position:  return sizeof(vec3_type);
		case attribute_type::texcoords: return sizeof(vec2_type);
		case attribute_type::normal:    return sizeof(vec3_type);
		case attribute_type::tangent:   return sizeof(vec3_type);
		case attribute_type::color:     return get_color_size();
		default: /* see below */;
		}
		return 0;
	}
	size_t get_attribute_offset(attribute_type attr) const
	{
		size_t s = get_attribute_size(attr);
		if (s < sizeof(T))
			s = sizeof(T);
		return s;
	}
	vec3_type compute_normal(const vec3_type& p0, const vec3_type& p1, const vec3_type& p2);
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
	/// return the size of one coordinate in bytes
	uint32_t get_coord_size() const { return sizeof(T); }
	/// clear simple mesh
	void clear();

	/// add a new position and return position index
	idx_type new_position(const vec3_type& p) { positions.push_back(p); return idx_type(positions.size()-1); }
	/// access to positions
	idx_type get_nr_positions() const { return idx_type(positions.size()); }
	vec3_type& position(idx_type pi) { return positions[pi]; }
	const vec3_type& position(idx_type pi) const { return positions[pi]; }
	const std::vector<vec3_type>& get_positions() const { return positions; }
	std::vector<vec3_type>& ref_positions() { return positions; }

	/// add a new normal and return normal index
	idx_type new_normal(const vec3_type& n) { normals.push_back(n); return idx_type(normals.size()-1); }
	/// access to normals
	bool has_normals() const { return get_nr_normals() > 0; }
	idx_type get_nr_normals() const { return idx_type(normals.size()); }
	vec3_type& normal(idx_type ni) { return normals[ni]; }
	const vec3_type& normal(idx_type ni) const { return normals[ni]; }
	const std::vector<vec3_type>& get_normals() const { return normals; }

	/// add a new tangent and return tangent index
	idx_type new_tangent(const vec3_type& tc) { tangents.push_back(tc); return idx_type(tangents.size() - 1); }
	/// access to tangents
	bool has_tangents() const { return get_nr_tangents() > 0; }
	idx_type get_nr_tangents() const { return idx_type(tangents.size()); }
	vec3_type& tangent(idx_type ti) { return tangents[ti]; }
	const vec3_type& tangent(idx_type ti) const { return tangents[ti]; }

	/// add a new texture coordinate and return texture coordinate index
	idx_type new_tex_coord(const vec2_type& tc) { tex_coords.push_back(tc); return idx_type(tex_coords.size() - 1); }
	/// access to texture coordinates
	bool has_tex_coords() const { return get_nr_tex_coords() > 0; }
	idx_type get_nr_tex_coords() const { return idx_type(tex_coords.size()); }
	vec2_type& tex_coord(idx_type ti) { return tex_coords[ti]; }
	const vec2_type& tex_coord(idx_type ti) const { return tex_coords[ti]; }

	/// compute the normal nml of a face and return whether this was possible
	bool compute_face_normal(idx_type fi, vec3_type& nml, bool normalize = true) const;
	/// compute face center
	vec3_type compute_face_center(idx_type fi) const;
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
	void compute_vertex_normals(bool use_parallel_implementation = true);
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
	unsigned extract_vertex_attribute_buffer(const std::vector<idx4_type>& unique_quadruples, bool include_tex_coords,
											 bool include_normals, bool include_tangents, std::vector<T>& attrib_buffer,
											 bool* include_colors_ptr = 0, int* num_floats_in_vertex = nullptr) const;
	/// apply transformation to mesh
	void transform(const mat3_type& linear_transformation, const vec3_type& translation);
	/// apply transformation to mesh with given inverse linear transformation
	void transform(const mat3_type& linear_transform, const vec3_type& translation, const mat3_type& inverse_linear_transform);
};

		}
	}
}

#include <cgv/config/lib_end.h>