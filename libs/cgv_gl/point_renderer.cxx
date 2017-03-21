#include "point_renderer.h"
#include <cgv_reflect_types/media/illum/phong_material.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {

		point_render_style::point_render_style() : point_color(0,0,0,1)
		{
			point_size = 1.0f;

			smooth_points = true;
			blend_points = true;
			culling_mode = CM_OFF;
			illumination_mode = IM_ONE_SIDED;
			use_point_shader = true;
			orient_splats = true;
			outline_width_from_pixel = 1;
			percentual_outline_width = 0.0f;

			map_color_to_material = cgv::render::MS_FRONT_AND_BACK;

			front_material.set_ambient(cgv::media::illum::phong_material::color_type(0.2f, 0.2f, 0.2f));
			front_material.set_diffuse(cgv::media::illum::phong_material::color_type(0.6f, 0.4f, 0.4f));
			front_material.set_specular(cgv::media::illum::phong_material::color_type(0.4f, 0.4f, 0.4f));
			front_material.set_shininess(20.0f);

			back_material.set_ambient(cgv::media::illum::phong_material::color_type(0.1f, 0.1f, 0.1f));
			back_material.set_diffuse(cgv::media::illum::phong_material::color_type(0.3f, 0.2f, 0.2f));
			back_material.set_specular(cgv::media::illum::phong_material::color_type(0.2f, 0.2f, 0.2f));
			back_material.set_shininess(20.0f);
		}

		point_renderer::point_renderer()
		{
		}

		bool point_renderer::init(cgv::render::context& ctx)
		{
			if (!point_prog.is_created()) {
				if (!point_prog.build_program(ctx, "point.glpr", true)) {
					std::cerr << "ERROR in point_renderer::init() ... could not build program pc.glpr" << std::endl;
					return false;
				}
			}
			return true;
		}

		void point_renderer::enable(cgv::render::context& ctx, const point_render_style& prs, float reference_point_size, float y_view_angle, bool has_normals, bool has_colors, bool use_group_point_size, bool use_group_color)
		{
			ctx.enable_material(prs.front_material, cgv::render::MS_FRONT);
			ctx.enable_material(prs.back_material, cgv::render::MS_BACK);

			glPointSize(prs.point_size);
			glColor4fv(&prs.point_color[0]);

			if (prs.culling_mode == CM_OFF) {
				glDisable(GL_CULL_FACE);
			}
			else {
				glCullFace(prs.culling_mode == CM_FRONTFACE ? GL_FRONT : GL_BACK);
				glEnable(GL_CULL_FACE);
			}

			if (prs.blend_points) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			if (prs.use_point_shader && point_prog.is_linked()) {
				cgv::render::gl::set_lighting_parameters(ctx, point_prog);
				point_prog.set_uniform(ctx, "point_size", prs.point_size * reference_point_size);
				point_prog.set_uniform(ctx, "use_group_point_size", use_group_point_size);
				point_prog.set_uniform(ctx, "use_group_color", use_group_color);
				point_prog.set_uniform(ctx, "map_color_to_material", has_colors ? int(prs.map_color_to_material) : 0);
				point_prog.set_uniform(ctx, "culling_mode", has_normals ? prs.culling_mode : 0);
				point_prog.set_uniform(ctx, "smooth_points", prs.smooth_points);
				point_prog.set_uniform(ctx, "illumination_mode", has_normals ? prs.illumination_mode : 0);
				point_prog.set_uniform(ctx, "orient_splats", has_normals ? prs.orient_splats : false);
				point_prog.set_uniform(ctx, "width", ctx.get_width());
				point_prog.set_uniform(ctx, "height", ctx.get_height());
				float pixel_extent_per_depth = (float)(2.0*tan(0.5*0.0174532925199*y_view_angle) / ctx.get_height());
				point_prog.set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);
				point_prog.set_uniform(ctx, "outline_width_from_pixel", prs.outline_width_from_pixel);
				point_prog.set_uniform(ctx, "percentual_outline_width", 0.01f*prs.percentual_outline_width);
				point_prog.enable(ctx);
			}
			else {
				if (prs.point_size > 1 && prs.smooth_points) {
					glEnable(GL_POINT_SMOOTH);
					glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
				}
				if (prs.illumination_mode == 0 || !has_normals)
					glDisable(GL_LIGHTING);
				else {
					if (prs.map_color_to_material > 0) {
						glEnable(GL_COLOR_MATERIAL);
						glColorMaterial(cgv::render::gl::map_to_gl(prs.map_color_to_material), GL_DIFFUSE);
					}
					else
						glDisable(GL_COLOR_MATERIAL);
					glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, prs.illumination_mode);
				}
			}
		}

		void point_renderer::disable(cgv::render::context& ctx, const point_render_style& prs, bool has_normals)
		{
			if (prs.use_point_shader && point_prog.is_linked())
				point_prog.disable(ctx);
			else {
				if (prs.point_size > 1 && prs.smooth_points) {
					glDisable(GL_POINT_SMOOTH);
				}
				if (prs.illumination_mode == 0 || !has_normals)
					glEnable(GL_LIGHTING);
				else {
					glEnable(GL_COLOR_MATERIAL);
					glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
				}
			}
			if (prs.blend_points) {
				glDisable(GL_BLEND);
			}
			if (prs.culling_mode != CM_OFF) {
				glDisable(GL_CULL_FACE);
			}
			ctx.disable_material(prs.front_material);
		}

		void point_renderer::clear(cgv::render::context& ctx)
		{
			point_prog.destruct(ctx);
		}
	}
}

namespace cgv {
	namespace reflect {
		namespace render {

			bool point_render_style::self_reflect(cgv::reflect::reflection_handler& rh)
			{
				return
					rh.reflect_member("point_size", point_size) &&
					rh.reflect_member("point_color", point_color) &&
					rh.reflect_member("smooth_points", smooth_points) &&
					rh.reflect_member("blend_points", blend_points) &&
					rh.reflect_member("culling_mode", culling_mode) &&
					rh.reflect_member("illumination_mode", illumination_mode) &&
					rh.reflect_member("use_point_shader", use_point_shader) &&
					rh.reflect_member("orient_splats", orient_splats) &&
					rh.reflect_member("outline_width_from_pixel", outline_width_from_pixel) &&
					rh.reflect_member("percentual_outline_width", percentual_outline_width) &&
					rh.reflect_member("map_color_to_material", map_color_to_material) &&
					rh.reflect_member("front_material", front_material) &&
					rh.reflect_member("back_material", back_material);
			}

		}
		cgv::reflect::extern_reflection_traits<cgv::render::point_render_style, cgv::reflect::render::point_render_style> get_reflection_traits(const cgv::render::point_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<cgv::render::point_render_style, cgv::reflect::render::point_render_style>();
		}
		cgv::reflect::enum_reflection_traits<cgv::render::IlluminationMode> get_reflection_traits(const cgv::render::IlluminationMode&)
		{
			return cgv::reflect::enum_reflection_traits<cgv::render::IlluminationMode>("OFF,ONESIDED,TWOSIDED");
		}
		cgv::reflect::enum_reflection_traits<cgv::render::CullingMode> get_reflection_traits(const cgv::render::CullingMode&)
		{
			return cgv::reflect::enum_reflection_traits<cgv::render::CullingMode>("OFF,BACKFACE,FRONTFACE");
		}
		cgv::reflect::enum_reflection_traits<cgv::render::MaterialSide> get_reflection_traits(const cgv::render::MaterialSide&)
		{
			return cgv::reflect::enum_reflection_traits<cgv::render::MaterialSide>("NONE,BACK,FRONT,FRONT_AND_BACK");
		}

	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct point_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::point_render_style>::get_name())
					return false;
				cgv::render::point_render_style* prs_ptr = reinterpret_cast<cgv::render::point_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				p->add_member_control(b, "point_size", prs_ptr->point_size, "value_slider", "min=1;max=20;log=true;ticks=true");
				p->add_member_control(b, "point_color", prs_ptr->point_color);
				p->add_member_control(b, "smooth", prs_ptr->smooth_points, "check");
				p->add_member_control(b, "blend", prs_ptr->blend_points, "check");
				p->add_member_control(b, "culling_mode", prs_ptr->culling_mode, "dropdown", "enums='off,backface,frontface'");
				p->add_member_control(b, "illumination_mode", prs_ptr->illumination_mode, "dropdown", "enums='off,onesided,twosided'");
				p->add_member_control(b, "use_shader", prs_ptr->use_point_shader, "check");
				p->add_member_control(b, "orient_splats", prs_ptr->orient_splats, "check");
				p->add_member_control(b, "outline_width_from_pixel", prs_ptr->outline_width_from_pixel, "value_slider", "min=0;max=10;ticks=true");
				p->add_member_control(b, "percentual_outline_width", prs_ptr->percentual_outline_width, "value_slider", "min=0;max=100;ticks=true");
				p->add_member_control(b, "map_color_to_material", (cgv::type::DummyEnum&)prs_ptr->map_color_to_material, "dropdown", "enums='OFF,BACK,FRONT,FRONT_AND_BACK'");
				p->add_gui("front_material", prs_ptr->front_material);
				p->add_gui("back_material", prs_ptr->back_material);
				return true;
			}
		};

#include "gl/lib_begin.h"

		extern CGV_API cgv::gui::gui_creator_registration<point_render_style_gui_creator> prs_gc_reg("point_render_style_gui_creator");

	}
}