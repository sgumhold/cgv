#pragma once

#include <vector>
#include <cgv/math/fvec.h>
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
	typedef typename cgv::math::fvec<idx_type, 2> vec2i;
	/// define index triple type
	typedef typename cgv::math::fvec<idx_type, 3> vec3i;
	/// define material type
	typedef typename illum::textured_surface_material mat_type;
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
	/// return the number of faces
	idx_type get_nr_faces() const { return faces.size(); }
	idx_type begin_corner(idx_type fi) const { return faces[fi]; }
	idx_type end_corner(idx_type fi) const { return fi + 1 == faces.size() ? position_indices.size() : faces[fi + 1]; }
	idx_type face_degree(idx_type fi) const { return end_corner(fi) - begin_corner(fi); }
	/// access to materials
	size_t get_nr_materials() const { return materials.size(); }
	const mat_type& get_material(size_t i) const { return materials[i]; }
	///
	size_t get_nr_groups() const { return group_names.size(); }
	const std::string& get_group_name(size_t i) const { return group_names[i]; }
	/// sort faces by group and material indices with two bucket sorts
	void sort_faces(std::vector<idx_type>& perm, bool by_group = true, bool by_material = true) const;
	/// merge the three indices into one index into a vector of unique index triples
	void merge_indices(std::vector<idx_type>& vertex_indices, std::vector<vec3i>& unique_triples, bool* include_tex_coords_ptr = 0, bool* include_normals_ptr = 0) const;
	/// extract element array buffers for triangulation
	void extract_triangle_element_buffer(const std::vector<idx_type>& vertex_indices, std::vector<idx_type>& triangle_element_buffer, 
		const std::vector<idx_type>* face_perm_ptr = 0, std::vector<vec3i>* material_group_start_ptr = 0) const;
	/// extract element array buffers for edges in wireframe
	void extract_wireframe_element_buffer(const std::vector<idx_type>& vertex_indices, std::vector<idx_type>& edge_element_buffer) const;
};

template <typename T = float>
class CGV_API simple_mesh : public simple_mesh_base
{
public:
	typedef typename cgv::media::axis_aligned_box<T, 3> box_type;
	typedef typename cgv::math::fvec<T, 3> vec3;
	typedef typename cgv::math::fvec<T, 2> vec2;
	typedef typename illum::surface_material::color_type clr_type;
	typedef typename illum::textured_surface_material mat_type;
	typedef cgv::type::uint32_type idx_type;
protected:
	friend class simple_mesh_obj_reader<T>;
	std::vector<vec3>  positions;
	std::vector<vec3>  normals;
	std::vector<vec2>  tex_coords;
public:
	vec3& position(idx_type pi) { return positions[pi]; }
	const vec3& position(idx_type pi) const { return positions[pi]; }
	vec3& normal(idx_type ni) { return normals[ni]; }
	const vec3& normal(idx_type ni) const { return normals[ni]; }
	vec2& tex_coord(idx_type ti) { return tex_coords[ti]; }
	const vec2& tex_coord(idx_type ti) const { return tex_coords[ti]; }
	idx_type get_nr_positions() const { return positions.size(); }
	idx_type get_nr_normals() const { return normals.size(); }
	idx_type get_nr_tex_coords() const { return tex_coords.size(); }
	bool has_tex_coords() const { return get_nr_tex_coords() > 0; }
	bool has_normals() const { return get_nr_normals() > 0; }
	/// compute the axis aligned bounding box
	box_type compute_box() const;

	/// clear simple mesh
	void clear();
	/// read simple mesh from file
	bool read(const std::string& file_name);
	/// extract vertex attribute array, return size of color in bytes
	unsigned extract_vertex_attribute_buffer(
		const std::vector<idx_type>& vertex_indices,
		const std::vector<vec3i>& unique_triples,
		bool include_tex_coords, bool include_normals, 
		std::vector<T>& attrib_buffer, bool *include_colors_ptr = 0) const;
};

		}
	}
}

#include <cgv/config/lib_end.h>