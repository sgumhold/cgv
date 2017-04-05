#include "surface_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv_gl/gl/gl_context.h>

namespace cgv {
	namespace render {

		surface_render_style::surface_render_style()
		{
			surface_color = cgv::media::illum::phong_material::color_type(0, 1, 1, 1);
			culling_mode = CM_OFF;
			illumination_mode = IM_ONE_SIDED;
			map_color_to_material = MS_FRONT_AND_BACK;

			front_material.set_ambient(cgv::media::illum::phong_material::color_type(0.2f, 0.2f, 0.2f, 1.0f));
			front_material.set_diffuse(cgv::media::illum::phong_material::color_type(0.6f, 0.4f, 0.4f, 1.0f));
			front_material.set_specular(cgv::media::illum::phong_material::color_type(0.4f, 0.4f, 0.4f, 1.0f));
			front_material.set_shininess(20.0f);

			back_material.set_ambient(cgv::media::illum::phong_material::color_type(0.1f, 0.1f, 0.1f, 1.0f));
			back_material.set_diffuse(cgv::media::illum::phong_material::color_type(0.3f, 0.2f, 0.2f, 1.0f));
			back_material.set_specular(cgv::media::illum::phong_material::color_type(0.2f, 0.2f, 0.2f, 1.0f));
			back_material.set_shininess(20.0f);
		}

		surface_renderer::surface_renderer()
		{
			has_normals = false;
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

		bool surface_renderer::enable(cgv::render::context& ctx)
		{
			bool res = group_renderer::enable(ctx);
			const surface_render_style& srs = get_style<surface_render_style>();
			set_gl_material(srs.front_material, cgv::render::MS_FRONT);
			set_gl_material(srs.back_material, cgv::render::MS_BACK);
			if (srs.culling_mode == CM_OFF) {
				glDisable(GL_CULL_FACE);
			}
			else {
				glCullFace(srs.culling_mode == CM_FRONTFACE ? GL_FRONT : GL_BACK);
				glEnable(GL_CULL_FACE);
			}
			if (ref_prog().is_linked()) {
				cgv::render::gl::set_lighting_parameters(ctx, ref_prog());
				ref_prog().set_uniform(ctx, "use_group_color", srs.use_group_color);
				ref_prog().set_uniform(ctx, "use_group_transformation", srs.use_group_transformation);
				ref_prog().set_uniform(ctx, "map_color_to_material", has_colors ? int(srs.map_color_to_material) : 0);
				ref_prog().set_uniform(ctx, "culling_mode", int(srs.culling_mode));
				ref_prog().set_uniform(ctx, "illumination_mode", int(srs.illumination_mode));
			}
			else
				res = false;
			return res;
		}

		bool surface_renderer::disable(cgv::render::context& ctx)
		{
			const surface_render_style& srs = get_style<surface_render_style>();
			if (srs.culling_mode != CM_OFF)
				glDisable(GL_CULL_FACE);
			return group_renderer::disable(ctx);
		}
	}
}

namespace cgv {
	namespace reflect {
		namespace render {
			bool surface_render_style::self_reflect(cgv::reflect::reflection_handler& rh)
			{
				return
					rh.reflect_base<group_render_style>(*this) &&
					rh.reflect_member("surface_color", surface_color) &&
					rh.reflect_member("culling_mode", culling_mode) &&
					rh.reflect_member("illumination_mode", illumination_mode) &&
					rh.reflect_member("map_color_to_material", map_color_to_material) &&
					rh.reflect_member("front_material", front_material) &&
					rh.reflect_member("back_material", back_material);
			}

		}
		cgv::reflect::extern_reflection_traits<cgv::render::surface_render_style, cgv::reflect::render::surface_render_style> get_reflection_traits(const cgv::render::surface_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<cgv::render::surface_render_style, cgv::reflect::render::surface_render_style>();
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
				p->add_member_control(b, "surface_color", srs_ptr->surface_color);
				p->add_member_control(b, "culling_mode", srs_ptr->culling_mode, "dropdown", "enums='off,backface,frontface'");
				p->add_member_control(b, "illumination_mode", srs_ptr->illumination_mode, "dropdown", "enums='off,onesided,twosided'");
				p->add_member_control(b, "map_color_to_material", (cgv::type::DummyEnum&)srs_ptr->map_color_to_material, "dropdown", "enums='OFF,FRONT,BACK,FRONT_AND_BACK'");
				p->add_gui("front_material", srs_ptr->front_material);
				p->add_gui("back_material", srs_ptr->back_material);
				p->add_gui("group render style", *static_cast<cgv::render::group_render_style*>(srs_ptr));
				return true;
			}
		};

#include "gl/lib_begin.h"

		extern CGV_API cgv::gui::gui_creator_registration<surface_render_style_gui_creator> frs_gc_reg("surface_render_style_gui_creator");

	}
}