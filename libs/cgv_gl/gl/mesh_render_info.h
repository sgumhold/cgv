#pragma once

#include <vector>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/textured_material.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {
		/** the mesh_render_info structure manages vertex buffer objects for 
		    attribute and element buffers as well as an attribute array binding
			object. The vertex buffer can be constructed from a simple mesh and
			the attribute array binding is bound to a specific
			shader program which defines the attribute locations.  */
		class CGV_API mesh_render_info : public render_types
		{
		public:
			/// define index type
			typedef cgv::type::uint32_type idx_type;
			/// index triple type
			typedef cgv::math::fvec<idx_type, 3> vec3i;
		protected:
			/// store mesh materials
			std::vector<cgv::render::textured_material*> mesh_mats;
			/// index triple storing the material index, the group index and the offset (element index) in the triangle element buffer
			std::vector<vec3i> material_group_start;
			/// element buffer used to store indices for triangle and edge rendering
			cgv::render::vertex_buffer vbo, vbe;
			/// attribute array binding used to store vertex attribute pointers
			cgv::render::attribute_array_binding aab;
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
		public:
			/// set vbo and vbe types
			mesh_render_info();
			/// give write access to materials
			std::vector<cgv::render::textured_material*>& ref_materials() { return mesh_mats; }
			/// check whether vbos are constructed
			bool is_constructed() const { return vbo.is_created(); }
			/// check whether attribute array binding is bound
			bool is_bound() const { return aab.is_created(); }
			/// construct mesh render info from a given simple mesh and store attributes in vertex buffer objects
			template <typename T>
			void construct_vbos(cgv::render::context& ctx, cgv::media::mesh::simple_mesh<T>& mesh)
			{
				// construct render buffers
				std::vector<idx_type> vertex_indices;
				std::vector<vec3i> unique_triples;
				std::vector<idx_type> triangle_element_buffer;
				std::vector<idx_type> edge_element_buffer;
				construct_vbos_base(ctx, mesh, vertex_indices, unique_triples, triangle_element_buffer, edge_element_buffer);
				std::vector<T> attrib_buffer;
				color_increment = mesh.extract_vertex_attribute_buffer(vertex_indices, unique_triples, include_tex_coords, include_normals, attrib_buffer, &include_colors);
				vbo.create(ctx, attrib_buffer);
				element_size = sizeof(T);
				position_descr   = cgv::render::element_descriptor_traits<typename cgv::media::mesh::simple_mesh<T>::vec3>::get_type_descriptor(mesh.position(0));
				tex_coords_descr = cgv::render::element_descriptor_traits<typename cgv::media::mesh::simple_mesh<T>::vec2>::get_type_descriptor(typename cgv::media::mesh::simple_mesh<T>::vec2());
				finish_construct_vbos_base(ctx, triangle_element_buffer, edge_element_buffer);
			}
			/// bind the attribute array to the given shader program
			void bind(cgv::render::context& c, cgv::render::shader_program& prog);
			/// return number of mesh parts
			size_t get_nr_mesh_parts() const { return material_group_start.size(); }
			/// return material index of i-th part
			size_t get_material_index(size_t i) const { return material_group_start[i](0); }
			/// return group index of i-th part
			size_t get_group_index(size_t i) const { return material_group_start[i](1); }
			/// draw triangles of given mesh part or whole mesh in case part_index is not given (=-1)
			void draw_surface(cgv::render::context& c, size_t part_index = -1);
			/// draw array elements forming the edges of the wireframe
			void draw_wireframe(cgv::render::context& c);
			/// helper function for rendering
			void render_mesh_part(cgv::render::context& c, cgv::render::shader_program& prog, size_t i, bool opaque);
			//! use given shader program to render mesh with own materials.
			/*! If no material is specified in mesh render info, use default 
			    material of context. The parameters render_opaque and 
				render_transparent allow to ignore mesh parts based on 
				material opaqueness. */
			void render_mesh(cgv::render::context& c, cgv::render::shader_program& prog,
							 bool render_opaque = true, bool render_transparent = true);
			/// use given shader program to render mesh with given material
			void render_mesh(cgv::render::context& c, cgv::render::shader_program& prog,
							 const cgv::media::illum::surface_material& material);
			/// destruct render mesh info and free vertex buffer objects
			void destruct(cgv::render::context& ctx);
		};


	}
}

#include <cgv/config/lib_end.h>