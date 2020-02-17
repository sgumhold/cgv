#pragma once

#include "render_info.h"
#include <cgv/media/mesh/simple_mesh.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {
/** the mesh_render_info structure manages vertex buffer objects for
	attribute and element buffers as well as an attribute array binding
	object. The vertex buffer can be constructed from a simple mesh and
	the attribute array binding is bound to a specific
	shader program which defines the attribute locations.  */
class CGV_API mesh_render_info : public render_info
{
public:
	/// index triple type
	typedef cgv::math::fvec<idx_type, 3> vec3i;
protected:
	/// store list of primitive names
	std::vector<std::string> primitive_names;
	/// index triple storing the material index, the primitive index and the offset (element index) in the triangle element buffer
	std::vector<vec3i> material_primitive_start;
	/// store whether tex coords are in vbo
	bool include_tex_coords;
	/// store whether normals are in vbo
	bool include_normals;
	/// store whether colors are in vbo
	bool include_colors;
	/// number of vertices
	size_t nr_vertices;
	/// number of edges in the wireframe representation
	size_t nr_edge_elements;
	/// number of triangles in the triangulation
	size_t nr_triangle_elements;
	/// size of single coordinate
	size_t element_size;
	/// draw call for wireframe rendering
	draw_call wire_draw_call;
	/// type description of position attribute
	cgv::render::type_descriptor position_descr;
	/// type description of tex coordinate attribute
	cgv::render::type_descriptor tex_coords_descr;
	/// color type size in bytes
	size_t color_increment;
	/// color type
	cgv::media::ColorType ct;
	/// helper function to construct vbos
	void construct_vbos_base(cgv::render::context& c, const cgv::media::mesh::simple_mesh_base& mesh,
		std::vector<idx_type>& vertex_indices, std::vector<vec3i>& unique_triples,
		std::vector<idx_type>& triangle_element_buffer, std::vector<idx_type>& edge_element_buffer);
	/// helper function for mesh render info consrtuctions
	void finish_construct_vbos_base(cgv::render::context& ctx,
		const std::vector<idx_type>& triangle_element_buffer,
		const std::vector<idx_type>& edge_element_buffer);
	/// 
	void construct_draw_calls(cgv::render::context& ctx);
public:
	/// set vbo and vbe types
	mesh_render_info();
	/// check whether vbos are constructed
	bool is_constructed() const { return get_vbos().size() > 0; }
	/// check whether attribute array binding is bound
	bool is_bound() const { return get_aas().size() > 0; }
	/// construct mesh render info from a given simple mesh and store attributes in vertex buffer objects
	template <typename T>
	void construct(cgv::render::context& ctx, cgv::media::mesh::simple_mesh<T>& mesh)
	{
		// construct render buffers
		std::vector<idx_type> vertex_indices;
		std::vector<vec3i> unique_triples;
		std::vector<idx_type> triangle_element_buffer;
		std::vector<idx_type> edge_element_buffer;
		construct_vbos_base(ctx, mesh, vertex_indices, unique_triples, triangle_element_buffer, edge_element_buffer);
		std::vector<T> attrib_buffer;
		color_increment = mesh.extract_vertex_attribute_buffer(vertex_indices, unique_triples, include_tex_coords, include_normals, attrib_buffer, &include_colors);
		ref_vbos().push_back(new cgv::render::vertex_buffer(cgv::render::VBT_VERTICES));
		ref_vbos().back()->create(ctx, attrib_buffer);
		element_size = sizeof(T);
		position_descr = cgv::render::element_descriptor_traits<typename cgv::media::mesh::simple_mesh<T>::vec3>::get_type_descriptor(mesh.position(0));
		tex_coords_descr = cgv::render::element_descriptor_traits<typename cgv::media::mesh::simple_mesh<T>::vec2>::get_type_descriptor(typename cgv::media::mesh::simple_mesh<T>::vec2());
		finish_construct_vbos_base(ctx, triangle_element_buffer, edge_element_buffer);
		construct_draw_calls(ctx);
	}
	/// return number of mesh primitives
	size_t get_nr_primitives() const { return primitive_names.size(); }
	/// return name of i-th mesh primitive
	const std::string& get_primitive_name(size_t i) const { return primitive_names[i]; }
	/// return number of mesh fragments that are of distinct primitive and material
	size_t get_nr_fragments() const { return material_primitive_start.size(); }
	/// return material index of i-th fragment
	size_t get_material_index(size_t i) const { return material_primitive_start[i](0); }
	/// return group index of i-th fragment
	size_t get_primitive_index(size_t i) const { return material_primitive_start[i](1); }

	/// draw triangles of given mesh part or whole mesh in case part_index is not given (=-1)
	void draw_primitive(cgv::render::context& ctx, size_t primitive_index, bool skip_opaque = false, bool skip_blended = false, bool use_materials = true);
	/// override to restrict bind function to first aa as second is used for wireframe rendering
	bool bind(context& ctx, shader_program& prog, bool force_success, int aa_index = -1);
	/// bind all or specific aa to the passed shader program
	bool bind_wireframe(context& ctx, shader_program& prog, bool force_success);
	/// draw array elements forming the edges of the wireframe
	void draw_wireframe(cgv::render::context& ctx);
	/// destruct render mesh info and free vertex buffer objects
	void destruct(cgv::render::context& ctx);
};
	}
}

#include <cgv/config/lib_end.h>