#include "surface_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv_gl/gl/gl_context.h>

namespace cgv {
	namespace render {


		void set_material_uniform(cgv::render::shader_program& prog, cgv::render::context& ctx, const std::string& name, const cgv::media::illum::surface_material& material)
		{
			prog.set_uniform(ctx, name + ".brdf_type", (int)material.get_brdf_type());
			prog.set_uniform(ctx, name + ".diffuse_reflectance", material.get_diffuse_reflectance());
			prog.set_uniform(ctx, name + ".roughness", material.get_roughness());
			prog.set_uniform(ctx, name + ".ambient_occlusion", material.get_ambient_occlusion());
			prog.set_uniform(ctx, name + ".emission", material.get_emission());
			prog.set_uniform(ctx, name + ".specular_reflectance", material.get_specular_reflectance());
			prog.set_uniform(ctx, name + ".roughness_anisotropy", material.get_roughness_anisotropy());
			prog.set_uniform(ctx, name + ".roughness_orientation", material.get_roughness_orientation());
			prog.set_uniform(ctx, name + ".propagation_slow_down", cgv::math::fvec<float, 2>(material.get_propagation_slow_down().real(), material.get_propagation_slow_down().imag()));
			prog.set_uniform(ctx, name + ".transparency", material.get_transparency());
			prog.set_uniform(ctx, name + ".metalness", material.get_metalness());
		}

		surface_render_style::surface_render_style() : material("default")
		{
			surface_color = cgv::media::illum::surface_material::color_type(0.4f, 0.1f, 0.7f);
			surface_opacity = 1.0f;
			culling_mode = CM_OFF;
			illumination_mode = IM_ONE_SIDED;
			map_color_to_material = CM_COLOR;
			material.ref_brdf_type() = cgv::media::illum::BrdfType(cgv::media::illum::BT_STRAUSS_DIFFUSE + cgv::media::illum::BT_STRAUSS);
		}

		surface_renderer::surface_renderer()
		{
			cull_per_primitive = true;
			has_normals = false;
			has_texcoords = false;
		}
		/// call this before setting attribute arrays to manage attribute array in given manager
		void surface_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			group_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "normal"))
				has_normals = true;
			if (has_attribute(ctx, "texcoord"))
				has_texcoords = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void surface_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			group_renderer::disable_attribute_array_manager(ctx, aam);
			has_normals = false;
			has_texcoords = false;
		}

		void set_gl_material_color(GLenum side, const cgv::media::illum::phong_material::color_type& c, float alpha, GLenum type)
		{
			GLfloat v[4] = { c[0], c[1], c[2], c[3] * alpha };
			glMaterialfv(side, type, v);
		}

		void set_gl_material(const cgv::media::illum::phong_material& mat, MaterialSide ms, float alpha = 1.0f)
		{
			if (ms == MS_NONE)
				return;
			unsigned side = gl::map_to_gl(ms);
			set_gl_material_color(side, mat.get_ambient(), alpha, GL_AMBIENT);
			set_gl_material_color(side, mat.get_diffuse(), alpha, GL_DIFFUSE);
			set_gl_material_color(side, mat.get_specular(), alpha, GL_SPECULAR);
			set_gl_material_color(side, mat.get_emission(), alpha, GL_EMISSION);
			glMaterialf(side, GL_SHININESS, mat.get_shininess());
		}

		/// method to set the normal attribute from a vertex buffer object, the element type must be given as explicit template parameter
		void surface_renderer::set_normal_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes)
		{
			has_normals = true;
			set_attribute_array(ctx, "normal", element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);

		}
		/// template method to set the texcoord attribute from a vertex buffer object, the element type must be given as explicit template parameter
		void surface_renderer::set_texcoord_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes)
		{
			has_texcoords = true;
			set_attribute_array(ctx, "texcoord", element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);

		}
		bool surface_renderer::enable(context& ctx)
		{
			bool res = group_renderer::enable(ctx);
			const surface_render_style& srs = get_style<surface_render_style>();
			if (cull_per_primitive) {
				if (srs.culling_mode == CM_OFF) {
					glDisable(GL_CULL_FACE);
				}
				else {
					glCullFace(srs.culling_mode == CM_FRONTFACE ? GL_FRONT : GL_BACK);
					glEnable(GL_CULL_FACE);
				}
			}
			if (ref_prog().is_linked()) {
				if (!has_colors)
					ctx.set_color(srs.surface_color, srs.surface_opacity);
				ctx.set_material(srs.material);
				ref_prog().set_uniform(ctx, "map_color_to_material", int(srs.map_color_to_material));
				ref_prog().set_uniform(ctx, "culling_mode", int(srs.culling_mode));
				ref_prog().set_uniform(ctx, "illumination_mode", int(srs.illumination_mode));
			}
			else
				res = false;
			return res;
		}

		bool surface_renderer::disable(context& ctx)
		{
			const surface_render_style& srs = get_style<surface_render_style>();
			if (cull_per_primitive) {
				if (srs.culling_mode != CM_OFF)
					glDisable(GL_CULL_FACE);
			}
			if (!attributes_persist()) {
				has_normals = false;
				has_texcoords = false;
			}
			return group_renderer::disable(ctx);
		}
		bool surface_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base<group_render_style>(*this) &&
				rh.reflect_member("culling_mode", culling_mode) &&
				rh.reflect_member("illumination_mode", illumination_mode) &&
				rh.reflect_member("map_color_to_material", map_color_to_material) &&
				rh.reflect_member("surface_color", surface_color) &&
				rh.reflect_member("material", material);
		}

		cgv::reflect::extern_reflection_traits<surface_render_style, surface_render_style_reflect> get_reflection_traits(const surface_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<surface_render_style, surface_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct surface_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::surface_render_style>::get_name())
					return false;
				cgv::render::surface_render_style* srs_ptr = reinterpret_cast<cgv::render::surface_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
				if (p->begin_tree_node("Color Mapping", srs_ptr->map_color_to_material, false, "level=3")) {
					p->align("\a");
					p->add_gui("Map Color to Material", srs_ptr->map_color_to_material, "bit_field_control",
						"enums='Color Front=1,Color Back=2,Opacity Front=4,Opacity Back=8'");
					p->align("\b");
					p->end_tree_node(srs_ptr->map_color_to_material);
				}
				p->add_member_control(b, "Illumination Mode", srs_ptr->illumination_mode, "dropdown", "enums='Off,One-Sided,Two-Sided'");
				p->add_member_control(b, "Culling Mode", srs_ptr->culling_mode, "dropdown", "enums='Off,Backface,Frontface'");
				if (p->begin_tree_node("Color and Materials", srs_ptr->surface_color, false, "level=3")) {
					p->align("\a");
					p->add_member_control(b, "Surface Color", srs_ptr->surface_color);
					p->add_member_control(b, "Surface Opacity", srs_ptr->surface_opacity, "value_slider", "min=0.0;step=0.01;max=1.0;log=false;ticks=true");
					if (p->begin_tree_node("Material", srs_ptr->material, false, "level=3")) {
						p->align("\a");
						p->add_gui("front_material", srs_ptr->material);
						p->align("\b");
						p->end_tree_node(srs_ptr->material);
					}
					p->align("\b");
					p->end_tree_node(srs_ptr->surface_color);
				}
				if (p->begin_tree_node("Use of Group Information", srs_ptr->illumination_mode, false, "level=3")) {
					p->align("\a");
					p->add_gui("group render style", *static_cast<cgv::render::group_render_style*>(srs_ptr));
					p->align("\b");
					p->end_tree_node(srs_ptr->illumination_mode);
				}
				return true;
			}
		};

#include "gl/lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<surface_render_style_gui_creator> surface_rs_gc_reg("surface_render_style_gui_creator");

	}
}