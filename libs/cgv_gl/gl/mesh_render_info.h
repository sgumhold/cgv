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
	/// redeclare attribute type for shorter name
	typedef cgv::media::mesh::simple_mesh_base::attribute_type attribute_type;
	/// index triple type
	typedef cgv::math::fvec<idx_type, 3> idx3_type;
	/// index quadruple type
	typedef cgv::math::fvec<idx_type, 4> idx4_type;
	/// type of map from mesh attribute to vbo index
	typedef std::map<attribute_type, uint32_t> attribute_map;
protected:
	/// store list of primitive names
	std::vector<std::string> primitive_names;
	/// index triple storing the material index, the primitive index and the offset (element index) in the triangle element buffer
	std::vector<idx3_type> material_primitive_start;
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
	
	/*@name support for flexible interleaving and dynamic updates of vertex attributes */
	//@{
	/// information stored per vbo
	struct vbo_configuration
	{
		bool is_dynamic = false;
		cgv::media::mesh::simple_mesh_base::AttributeFlags attributes = cgv::media::mesh::simple_mesh_base::AttributeFlags(0);
		uint32_t stride = 0;
	};
	/// store for each vbo its configuration
	std::vector<vbo_configuration> vbo_config;
	/// information needed per mesh attribute to know where it is stored in vbos
	struct attribute_configuration
	{
		uint32_t vbo_index = 0;
		uint32_t offset = 0;		
	};
	/// for each mesh attribute that is used, map attribute type to the vbo index
	std::map<attribute_type, attribute_configuration> per_attribute_vbo_config;
	/// in case that at least one vbo is replaceable, store per vertex the unique quadruple of attribute indices
	std::vector<idx4_type> unique_quadruples;
	/// check whether a mesh attribute is used
	bool attribute_is_used(attribute_type at) const;
	/// configure which attributes should be stored, in which vbos they go and which vbos should support dynamic updates
	bool configure_vbos(cgv::render::context& ctx, const cgv::media::mesh::simple_mesh_base& mesh, 
		const attribute_map& attribute_to_vbo_index_map, const std::vector<int>& dynamic_vbo_idx);
	//@}

	/**
	 * Prepare vertex attribute data creation by extraction of unique combinations of attribute indices
	 * 
	 * \param [in] ctx The render context.
	 * \param [in] mesh The mesh which shall be transformed.
	 * \param [out] vertex_indices per face corner the vertex index or index of attribute index tuple
	 * \param [out] unique_quadruples the list of attribute index tuples which aggregate unique combinations of vertex attribute indices
	 * \param [out] triangle_element_buffer the buffer with successive vertex index triples, which make up mesh triangles.
	 * \param [out] edge_element_buffer the buffer with successive vertex index pairs, which make up the mesh edges
	 * 
	 * \see mesh_render_info::finish_construct_vbos_base()
	 */
	void construct_index_buffers(cgv::render::context& ctx, const cgv::media::mesh::simple_mesh_base& mesh,
								 std::vector<idx4_type>& unique_quadruples, std::vector<idx_type>& per_corner_vertex_index,
		                         std::vector<idx_type>& edges_element_buffer, std::vector<idx_type>& triangles_element_buffer);
	/**
	 * Uploads the given element buffers into EBOs on the GPU.
	 * 
	 * \param [in] ctx The CGV drawing context.
	 * \param [in] edge_element_buffer The vector of indices which make up the edges of faces.
	 * \param [in] triangle_element_buffer The vector of indices which make up triangles.
	 * 
	 * \see mesh_render_info::configure_vbos()
	 */
	void construct_element_vbo(cgv::render::context& ctx, const std::vector<idx_type>& edge_element_buffer, const std::vector<idx_type>& triangle_element_buffer);
	/// for each combination of primitive (face group of mesh) and material create and store one draw call
	void construct_draw_calls(cgv::render::context& ctx);
public:
	/// set vbo and vbe types
	mesh_render_info();
	/// check whether vbos are constructed
	bool is_constructed() const { return get_vbos().size() > 0; }
	/// check whether attribute array binding is bound
	bool is_bound() const { return get_aas().size() > 0; }
	/// @brief Construct mesh render info from a given simple mesh and store all vertex attributes in interleaved in one vertex buffer object that does not allow dynamic updates.
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
				   std::vector<idx_type>* tuple_normal_indices = nullptr, 
				   int* num_floats_in_vertex = nullptr)
	{
		element_size = sizeof(T);
		position_descr = cgv::render::element_descriptor_traits<
			typename cgv::media::mesh::simple_mesh<T>::vec3_type>::get_type_descriptor(mesh.position(0));
		tex_coords_descr = cgv::render::element_descriptor_traits<typename cgv::media::mesh::simple_mesh<T>::vec2_type>::
			get_type_descriptor(typename cgv::media::mesh::simple_mesh<T>::vec2_type());
		attribute_map va_vbo_idx = { { attribute_type::position, 0 } };
		if (mesh.has_tex_coords())
			va_vbo_idx[attribute_type::texcoords] = 0;
		if (mesh.has_normals())
			va_vbo_idx[attribute_type::normal] = 0;
		if (mesh.has_tangents())
			va_vbo_idx[attribute_type::tangent] = 0;
		if (mesh.has_colors())
			va_vbo_idx[attribute_type::color] = 0;
		configure_vbos(ctx, mesh, va_vbo_idx, {});
		// construct render buffers
		std::vector<idx_type> vertex_indices, edge_element_buffer, triangle_element_buffer;
		std::vector<idx4_type> unique_quadruples;
		construct_index_buffers(ctx, mesh, unique_quadruples, vertex_indices, edge_element_buffer, triangle_element_buffer);
		// record which attributes each unique tuple was referencing
		for (const auto& tuple : unique_quadruples) {
			if (tuple_pos_indices)
				tuple_pos_indices->push_back(tuple[0]);
			if (tuple_normal_indices)
				tuple_normal_indices->push_back(tuple[2]);
		}
		std::vector<uint8_t> attrib_buffer;
		mesh.extract_vertex_attribute_buffer_base(unique_quadruples, vbo_config.front().attributes, attrib_buffer);
		ref_vbos().push_back(new cgv::render::vertex_buffer(cgv::render::VBT_VERTICES));
		ref_vbos().back()->create(ctx, attrib_buffer);
		construct_element_vbo(ctx, edge_element_buffer, triangle_element_buffer);
		construct_draw_calls(ctx);
	}
	/// Construct mesh render info from a given simple mesh and store vertex attributes flexibly in multiple vertex buffer objects which allow for dynamic updates.
	template <typename T>
	void construct_dynamic(cgv::render::context& ctx, cgv::media::mesh::simple_mesh<T>& mesh,
		const attribute_map& va_vbo_idx, const std::vector<int>& dynamic_vbo_idx)
	{
		element_size = sizeof(T);
		position_descr = cgv::render::element_descriptor_traits<
			typename cgv::media::mesh::simple_mesh<T>::vec3_type>::get_type_descriptor(mesh.position(0));
		tex_coords_descr = cgv::render::element_descriptor_traits<typename cgv::media::mesh::simple_mesh<T>::vec2_type>::
			get_type_descriptor(typename cgv::media::mesh::simple_mesh<T>::vec2_type());
		configure_vbos(ctx, mesh, va_vbo_idx, dynamic_vbo_idx);
		bool keep_unique_quadrupels = !dynamic_vbo_idx.empty();
		// construct unique attribute index tupels that form vertices in the render data together with
		// vector of per corner vertex indices, vertex index pairs forming edges and vertex index triples forming triangles
		std::vector<idx4_type> tmp_unique_quadruples;
		std::vector<idx4_type>& unique_quadruples_ref = keep_unique_quadrupels ? unique_quadruples : tmp_unique_quadruples;
		std::vector<idx_type> per_corner_vertex_index, edge_element_buffer, triangle_element_buffer;
		construct_index_buffers(ctx, mesh, unique_quadruples_ref, per_corner_vertex_index, edge_element_buffer, triangle_element_buffer);
		//
		for (const auto& vc : vbo_config) {
			std::vector<uint8_t> attrib_buffer;
			auto vca = vc.attributes;
			mesh.extract_vertex_attribute_buffer_base(unique_quadruples_ref, vca, attrib_buffer);
			ref_vbos().push_back(new cgv::render::vertex_buffer(cgv::render::VBT_VERTICES));
			ref_vbos().back()->create(ctx, attrib_buffer);
		}
		construct_element_vbo(ctx, edge_element_buffer, triangle_element_buffer);
		construct_draw_calls(ctx);
	}
	/// replace the content of one vbo from a mesh 
	template <typename T>
	void update_vbo(cgv::render::context& ctx, cgv::media::mesh::simple_mesh<T>& mesh, int vbo_index)
	{
		const auto& vc = vbo_config[vbo_index];
		std::vector<uint8_t> attrib_buffer;
		auto vca = vc.attributes;
		mesh.extract_vertex_attribute_buffer_base(unique_quadruples, vca, attrib_buffer);
		ref_vbos()[vbo_index]->replace(ctx, 0, &attrib_buffer[0], attrib_buffer.size());
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