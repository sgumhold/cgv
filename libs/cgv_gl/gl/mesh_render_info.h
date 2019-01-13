#pragma once

#include <vector>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/textured_material.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {

		struct CGV_API mesh_render_info
		{
			/// define index type
			typedef cgv::type::uint32_type idx_type;
			///
			typedef typename cgv::math::fvec<idx_type, 3> vec3i;
			///
			std::vector<cgv::render::textured_material*> mesh_mats;
			///
			std::vector<vec3i> material_group_start;
			///
			cgv::render::vertex_buffer vbo, vbe;
			/// 
			cgv::render::attribute_array_binding aab;
			///
			size_t nr_triangle_elements;
			///
			size_t nr_edge_elements;
			/// set vbo and vbe types
			mesh_render_info();
			///
			void destruct(cgv::render::context& ctx);
			///
			void construct_base(cgv::render::context& c, const cgv::media::mesh::simple_mesh_base& mesh,
				std::vector<idx_type>& vertex_indices, std::vector<vec3i>& unique_triples,
				bool& include_tex_coords, bool& include_normals, bool& include_colors,
				std::vector<idx_type>& triangle_element_buffer, std::vector<idx_type>& edge_element_buffer);
			///
			void finish_construct_base(cgv::render::context& ctx, size_t element_size, bool include_tex_coords, bool include_normals,
				const std::vector<idx_type>& triangle_element_buffer, const std::vector<idx_type>& edge_element_buffer,
				cgv::render::type_descriptor vec3_descr, cgv::render::type_descriptor vec2_descr, size_t nr_vertices, 
				unsigned color_increment, cgv::media::colored_model::ColorType ct);
			///
			template <typename T>
			void construct(cgv::render::context& ctx, cgv::media::mesh::simple_mesh<T>& mesh)
			{
				// construct render buffers
				std::vector<idx_type> vertex_indices;
				std::vector<vec3i> unique_triples;
				bool include_tex_coords = true;
				bool include_normals = true;
				bool include_colors = true;
				std::vector<idx_type> triangle_element_buffer;
				std::vector<idx_type> edge_element_buffer;
				construct_base(ctx, mesh, vertex_indices, unique_triples, include_tex_coords, include_normals, include_colors, triangle_element_buffer, edge_element_buffer);
				std::vector<T> attrib_buffer;
				unsigned color_increment = mesh.extract_vertex_attribute_buffer(vertex_indices, unique_triples, include_tex_coords, include_normals, attrib_buffer, &include_colors);
				vbo.create(ctx, attrib_buffer);
				finish_construct_base(ctx, sizeof(T), include_tex_coords, include_normals, triangle_element_buffer, edge_element_buffer,
					cgv::render::element_descriptor_traits<typename cgv::media::mesh::simple_mesh<T>::vec3>::get_type_descriptor(mesh.position(0)),
					cgv::render::element_descriptor_traits<typename cgv::media::mesh::simple_mesh<T>::vec2>::get_type_descriptor(mesh.tex_coord(0)), unique_triples.size(), 
					color_increment, mesh.get_color_storage_type());

			}
			///
			void render_mesh(cgv::render::context& c);
			///
			void render_material_part(cgv::render::context& c, size_t mi, bool opaque);
			///
			void render_wireframe(cgv::render::context& c);
		};


	}
}

#include <cgv/config/lib_end.h>