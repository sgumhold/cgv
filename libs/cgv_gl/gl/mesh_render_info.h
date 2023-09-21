#pragma once

#include "render_info.h"

#include <vector>

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
	/// index quartuple type
	typedef cgv::math::fvec<idx_type, 4> vec4i;
protected:
	/// store list of primitive names
	std::vector<std::string> primitive_names;
	/// index triple storing the material index, the primitive index and the offset (element index) in the triangle element buffer
	std::vector<vec3i> material_primitive_start;
	/// store whether tex coords are in vbo
	bool include_tex_coords;
	/// store whether normals are in vbo
	bool include_normals;
	/// store whether tangents are in vbo
	bool include_tangents;
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
	/**
	 * Transform the given mesh into vectors which are suitable for upload into a VBO/EBO.
	 * 
	 * \param [in] c The CGV drawing context.
	 * \param [in] mesh The mesh which shall be transformed.
	 * \param [out] vertex_indices the list of indices into the unique n-tuples.
	 * \param [out] unique_quartuples the list of n-tuples which aggregate unique combinations of vertex attribute indices.
	 * \param [out] triangle_element_buffer the buffer with successive index triples, which make up a triangle.
	 * \param [out] edge_element_buffer the buffer with successive index pairs, which make up the edges of a face.
	 * 
	 * \see mesh_render_info::finish_construct_vbos_base()
	 */
	void construct_vbos_base(cgv::render::context& c, const cgv::media::mesh::simple_mesh_base& mesh,
							 std::vector<idx_type>& vertex_indices, std::vector<vec4i>& unique_quartuples,
							 std::vector<idx_type>& triangle_element_buffer, std::vector<idx_type>& edge_element_buffer);
	/**
	 * Uploads the given element buffers into EBOs on the GPU.
	 * 
	 * \param [in] ctx The CGV drawing context.
	 * \param [in] triangle_element_buffer The vector of indices which make up triangles.
	 * \param [in] edge_element_buffer The vector of indices which make up the edges of faces.
	 * 
	 * \see mesh_render_info::construct_vbos_base()
	 */
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
	/// @brief Construct mesh render info from a given simple mesh and store attributes in vertex buffer objects.
	/// @tparam T The coordinate type which is used in the cgv::media::mesh::simple_mesh
	/// @param ctx The CGV rendering context.
	/// @param mesh The general mesh which will be translated into appropriately formatted GPU buffers.
	/// @param tuple_pos_indices If not nullptr, will be filled the mapping from unique vertex attribute tuples to the
	/// original position buffers of the mesh.
	/// @param tuple_normal_indices If not nullptr, will be filled the mapping from unique vertex attribute tuples to
	/// the original normal buffers of the mesh.
	/// @param num_floats_in_vertex If not nullptr will be set to the number of floats which make up one vertex with all
	/// its attributes.
	template <typename T>
	void construct(cgv::render::context& ctx, cgv::media::mesh::simple_mesh<T>& mesh,
				   std::vector<idx_type>* tuple_pos_indices = nullptr,
				   std::vector<idx_type>* tuple_normal_indices = nullptr, int* num_floats_in_vertex = nullptr)
	{
		// construct render buffers
		std::vector<idx_type> vertex_indices;
		std::vector<vec4i> unique_quartuples;
		std::vector<idx_type> triangle_element_buffer;
		std::vector<idx_type> edge_element_buffer;
		construct_vbos_base(ctx, mesh, vertex_indices, unique_quartuples, triangle_element_buffer, edge_element_buffer);

		// Record which attributes each unique tuple was referencing
		for (const auto& tuple : unique_quartuples) {
			if (tuple_pos_indices)
				tuple_pos_indices->push_back(tuple[0]);
			if (tuple_normal_indices)
				tuple_normal_indices->push_back(tuple[2]);
		}

		std::vector<T> attrib_buffer;
		color_increment = mesh.extract_vertex_attribute_buffer(unique_quartuples, include_tex_coords, include_normals,
															   include_tangents, attrib_buffer, &include_colors,
															   num_floats_in_vertex);
		ref_vbos().push_back(new cgv::render::vertex_buffer(cgv::render::VBT_VERTICES));
		ref_vbos().back()->create(ctx, attrib_buffer);
		element_size = sizeof(T);
		position_descr = cgv::render::element_descriptor_traits<
			  typename cgv::media::mesh::simple_mesh<T>::vec3>::get_type_descriptor(mesh.position(0));
		tex_coords_descr = cgv::render::element_descriptor_traits<typename cgv::media::mesh::simple_mesh<T>::vec2>::
			  get_type_descriptor(typename cgv::media::mesh::simple_mesh<T>::vec2());
		finish_construct_vbos_base(ctx, triangle_element_buffer, edge_element_buffer);
		construct_draw_calls(ctx);
	}
	/// set the number of to be drawn instances - in case of 0, instanced drawing is turned off
	void set_nr_instances(unsigned nr);
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

	/// @brief Returns how many vertices the mesh has.
	size_t get_nr_vertices() const { return nr_vertices; };
	/// @brief Returns how many edge-indices are recorded in the element buffer.
	size_t get_nr_edge_elements() const { return nr_edge_elements; };
	/// @brief Returns how many triangle-indices are recorded in the element buffer.
	size_t get_nr_triangle_elements() const { return nr_triangle_elements; };
	/// @brief Returns how many bytes on index value takes up in the element buffer.
	size_t get_element_size() const { return element_size; };

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