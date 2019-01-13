#include <cgv/base/base.h>
#include "mesh_render_info.h"
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>

namespace cgv {
	namespace render {
		mesh_render_info::mesh_render_info() : vbe(cgv::render::VBT_INDICES)
		{
			nr_triangle_elements = 0;
			nr_edge_elements = 0;
		}
		///
		void mesh_render_info::destruct(cgv::render::context& ctx)
		{
			while (mesh_mats.size() > 0) {
				mesh_mats.back()->destruct_textures(ctx);
				delete mesh_mats.back();
				mesh_mats.pop_back();
			}
			vbo.destruct(ctx);
			vbe.destruct(ctx);
			aab.destruct(ctx);
			material_group_start.clear();
			nr_triangle_elements = 0;
			nr_edge_elements = 0;
		}

		///
		void mesh_render_info::construct_base(cgv::render::context& ctx, const cgv::media::mesh::simple_mesh_base& mesh,
			std::vector<idx_type>& vertex_indices, std::vector<vec3i>& unique_triples,
			bool& include_tex_coords, bool& include_normals, bool& include_colors,
			std::vector<idx_type>& triangle_element_buffer, std::vector<idx_type>& edge_element_buffer)
		{
			// load material textures
			for (unsigned i = 0; i < mesh.get_nr_materials(); ++i) {
				mesh_mats.push_back(new cgv::render::textured_material(mesh.get_material(i)));
				mesh_mats.back()->ensure_textures(ctx);
			}

			std::vector<idx_type> perm;
			mesh.sort_faces(perm);
			mesh.merge_indices(vertex_indices, unique_triples, &include_tex_coords, &include_normals);
			mesh.extract_triangle_element_buffer(vertex_indices, triangle_element_buffer, &perm, &material_group_start);
			nr_triangle_elements = triangle_element_buffer.size();
			mesh.extract_wireframe_element_buffer(vertex_indices, edge_element_buffer);
			nr_edge_elements = edge_element_buffer.size();
		}

		///
		void mesh_render_info::finish_construct_base(cgv::render::context& ctx, size_t element_size,
			bool include_tex_coords, bool include_normals,
			const std::vector<idx_type>& triangle_element_buffer, const std::vector<idx_type>& edge_element_buffer,
			cgv::render::type_descriptor vec3_descr, cgv::render::type_descriptor vec2_descr, size_t nr_vertices, 
			unsigned color_increment, cgv::media::colored_model::ColorType ct)
		{
			unsigned stride = 3;
			if (include_tex_coords)
				stride += 2;
			if (include_normals)
				stride += 3;
			stride += color_increment;
			if (stride == 3)
				stride = 0;
			else
				stride *= element_size;
			
			cgv::render::shader_program& prog = ctx.ref_surface_shader_program(true);
			vbe.create(ctx, (nr_triangle_elements + nr_edge_elements) * sizeof(idx_type));
			vbe.replace(ctx, 0, &triangle_element_buffer.front(), triangle_element_buffer.size());
			vbe.replace(ctx, triangle_element_buffer.size() * sizeof(idx_type), &edge_element_buffer.front(), edge_element_buffer.size());
			aab.create(ctx);
			aab.set_element_array(ctx, vbe);
			aab.set_attribute_array(ctx,
				prog.get_position_index(),
				vec3_descr,
				vbo, 0, nr_vertices, stride);

			unsigned offset = 3 * element_size;
			if (include_tex_coords) {
				aab.set_attribute_array(ctx,
					prog.get_texcoord_index(),
					vec2_descr,
					vbo, offset, nr_vertices, stride);
				offset += 2 * element_size;
			}
			if (include_normals) {
				aab.set_attribute_array(ctx,
					prog.get_normal_index(),
					vec3_descr,
					vbo, offset, nr_vertices, stride);
				offset += 3 * element_size;
			}
			if (color_increment > 0) {
				static int nr_comps[] = { 4,4,3,4 };
				static cgv::type::info::TypeId type_ids[] = { cgv::type::info::TI_UINT8,cgv::type::info::TI_UINT8,cgv::type::info::TI_FLT32,cgv::type::info::TI_FLT32 };
				aab.set_attribute_array(ctx,
					prog.get_color_index(),
					cgv::render::type_descriptor(type_ids[ct], nr_comps[ct], true),
					vbo, offset, nr_vertices, stride);
				offset += color_increment * element_size;
			}
		}

		///
		void mesh_render_info::render_wireframe(cgv::render::context& c)
		{
			if (nr_edge_elements > 0) {
				aab.enable(c);
				glDrawElements(GL_LINES, nr_edge_elements, GL_UNSIGNED_INT, (void*)(nr_triangle_elements * sizeof(idx_type)));
				aab.disable(c);
			}
		}

		///
		void mesh_render_info::render_material_part(cgv::render::context& c, size_t i, bool opaque)
		{
			cgv::render::textured_material& mat = *mesh_mats[material_group_start[i](0)];
			if (mat.get_transparency_index() != -1) {
				if (opaque)
					return;
			}
			else {
				if ((mat.get_transparency() < 0.01f) != opaque)
					return;
			}
			size_t offset = material_group_start[i](2);
			size_t count = (i + 1 == material_group_start.size() ? nr_triangle_elements : material_group_start[i + 1](2)) - offset;

			cgv::render::shader_program& prog = c.ref_surface_shader_program(true);
			if (!opaque) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.1f);
			}
			prog.enable(c);
			prog.set_uniform(c, "illumination_mode", 2);
			c.enable_material(mat);
			aab.enable(c);
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(offset * sizeof(idx_type)));
			aab.disable(c);
			c.disable_material(mat);
			prog.disable(c);
			if (!opaque) {
				glDisable(GL_BLEND);
				glDisable(GL_ALPHA_TEST);
			}
		}

		void mesh_render_info::render_mesh(cgv::render::context& c)
		{
			for (size_t i = 0; i < material_group_start.size(); ++i)
				render_material_part(c, i, true);
			for (size_t i = 0; i < material_group_start.size(); ++i)
				render_material_part(c, i, false);
		}
	}
}