#include <cgv/base/base.h>
#include "mesh_render_info.h"
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>

namespace cgv {
	namespace render {
		mesh_render_info::mesh_render_info() : 
			vbe(cgv::render::VBT_INDICES), 
			position_descr(cgv::render::element_descriptor_traits<vec3>::get_type_descriptor(vec3())), 
			tex_coords_descr(cgv::render::element_descriptor_traits<vec2>::get_type_descriptor(vec2()))
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
		void mesh_render_info::construct_vbos_base(
			cgv::render::context& ctx, 
			const cgv::media::mesh::simple_mesh_base& mesh,
			std::vector<idx_type>& vertex_indices, 
			std::vector<vec3i>& unique_triples,
			std::vector<idx_type>& triangle_element_buffer, 
			std::vector<idx_type>& edge_element_buffer)
		{
			include_tex_coords = include_normals = include_colors = true;

			// load material textures
			for (unsigned i = 0; i < mesh.get_nr_materials(); ++i) {
				mesh_mats.push_back(new cgv::render::textured_material(mesh.get_material(i)));
				mesh_mats.back()->ensure_textures(ctx);
			}

			std::vector<idx_type>* perm_ptr = 0;
			bool sort_by_groups = mesh.get_nr_groups() > 0;
			bool sort_by_materials = mesh.get_nr_materials() > 0;
			if (sort_by_groups || sort_by_materials) {
				perm_ptr = new std::vector<idx_type>();
				mesh.sort_faces(*perm_ptr, sort_by_groups, sort_by_materials);
			}
			mesh.merge_indices(vertex_indices, unique_triples, &include_tex_coords, &include_normals);
			nr_vertices = unique_triples.size();
			mesh.extract_triangle_element_buffer(vertex_indices, triangle_element_buffer, perm_ptr, mesh.get_nr_materials() > 0 ? &material_group_start : 0);
			if (perm_ptr) {
				delete perm_ptr;
				perm_ptr = 0;
			}
			nr_triangle_elements = triangle_element_buffer.size();
			mesh.extract_wireframe_element_buffer(vertex_indices, edge_element_buffer);
			nr_edge_elements = edge_element_buffer.size();
			ct = mesh.get_color_storage_type();
		}

		///
		void mesh_render_info::finish_construct_vbos_base(cgv::render::context& ctx, 
			const std::vector<idx_type>& triangle_element_buffer, 
			const std::vector<idx_type>& edge_element_buffer)
		{
			vbe.create(ctx, (nr_triangle_elements + nr_edge_elements) * sizeof(idx_type));
			vbe.replace(ctx, 0, 
				&triangle_element_buffer.front(), triangle_element_buffer.size());
			vbe.replace(ctx, triangle_element_buffer.size() * sizeof(idx_type), 
				&edge_element_buffer.front(), edge_element_buffer.size());
		}
		/// bind the attribute array to the given shader program
		void mesh_render_info::bind(cgv::render::context& ctx, cgv::render::shader_program& prog)
		{
			size_t stride = 3;
			if (include_tex_coords)
				stride += 2;
			if (include_normals)
				stride += 3;
			stride += color_increment;
			if (stride == 3)
				stride = 0;
			else
				stride *= element_size;

			aab.create(ctx);
			aab.set_element_array(ctx, vbe);
			aab.set_attribute_array(ctx,
				prog.get_position_index(),
				position_descr,
				vbo, 0, nr_vertices, (unsigned)stride);

			size_t offset = 3 * element_size;
			if (include_tex_coords) {
				aab.set_attribute_array(ctx,
					prog.get_texcoord_index(),
					tex_coords_descr,
					vbo, offset, nr_vertices, (unsigned)stride);
				offset += 2 * element_size;
			}
			if (include_normals) {
				aab.set_attribute_array(ctx,
					prog.get_normal_index(),
					position_descr,
					vbo, offset, nr_vertices, (unsigned)stride);
				offset += 3 * element_size;
			}
			if (color_increment > 0) {
				static int nr_comps[] = { 4,4,3,4 };
				static cgv::type::info::TypeId type_ids[] = { cgv::type::info::TI_UINT8,cgv::type::info::TI_UINT8,cgv::type::info::TI_FLT32,cgv::type::info::TI_FLT32 };
				aab.set_attribute_array(ctx,
					prog.get_color_index(),
					cgv::render::type_descriptor(type_ids[ct], nr_comps[ct], true),
					vbo, offset, nr_vertices, (unsigned)stride);
				offset += color_increment * element_size;
			}
		}
		/// draw triangles of given mesh part or whole mesh in case part_index is not given (=-1)
		void mesh_render_info::draw_surface(cgv::render::context& c, size_t part_index)
		{
			if (nr_triangle_elements == 0)
				return;

			aab.enable(c);
			if (part_index == -1) 
				glDrawElements(GL_TRIANGLES, GLsizei(nr_triangle_elements), GL_UNSIGNED_INT, 0);
			else {
				size_t offset = material_group_start[part_index](2);
				size_t count = (part_index + 1 == material_group_start.size() ? nr_triangle_elements : material_group_start[part_index + 1](2)) - offset;
				glDrawElements(GL_TRIANGLES, GLsizei(count), GL_UNSIGNED_INT, (void*)(offset * sizeof(idx_type)));
			}
			aab.disable(c);
		}

		///
		void mesh_render_info::draw_wireframe(cgv::render::context& c)
		{
			if (nr_edge_elements > 0) {
				aab.enable(c);
				glDrawElements(GL_LINES, GLsizei(nr_edge_elements), GL_UNSIGNED_INT, (void*)(nr_triangle_elements * sizeof(idx_type)));
				aab.disable(c);
			}
		}

		///
		void mesh_render_info::render_mesh_part(cgv::render::context& c, cgv::render::shader_program& prog, size_t i, bool opaque)
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
			if (!opaque) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.1f);
			}
			prog.enable(c);
				c.enable_material(mat);
				draw_surface(c, i);
			c.disable_material(mat);
			prog.disable(c);
			if (!opaque) {
				glDisable(GL_BLEND);
				glDisable(GL_ALPHA_TEST);
			}
		}
		///
		void mesh_render_info::render_mesh(
			cgv::render::context& c, cgv::render::shader_program& prog,
			const cgv::media::illum::surface_material& material)
		{
			if (material.get_transparency() > 0.0f) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.1f);
			}
			prog.enable(c);
				c.set_material(material);
				draw_surface(c);
			prog.disable(c);
			if (material.get_transparency() > 0.0f) {
				glDisable(GL_BLEND);
				glDisable(GL_ALPHA_TEST);
			}
		}

		void mesh_render_info::render_mesh(
			cgv::render::context& c, cgv::render::shader_program& prog,
			bool render_opaque, bool render_transparent)
		{
			if (material_group_start.empty()) {
				if (c.get_current_material())
					render_mesh(c, prog, *c.get_current_material());
				return;
			}
			if (render_opaque) {
				for (size_t i = 0; i < material_group_start.size(); ++i)
					render_mesh_part(c, prog, i, true);
			}
			if (render_transparent) {
				for (size_t i = 0; i < material_group_start.size(); ++i)
					render_mesh_part(c, prog, i, false);
			}
		}
	}
}