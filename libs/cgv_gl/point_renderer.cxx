#include "point_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {

		render_style* point_renderer::create_render_style() const
		{
			return new point_render_style();
		}

		point_render_style::point_render_style() : halo_color(1,1,1,1)
		{
			point_size = 1.0f;
			use_group_point_size = false;
			measure_point_size_in_pixel = true;

			smooth_points = true;
			blend_points = true;
			use_point_shader = true;
			orient_splats = true;
			outline_width_from_pixel = 0.0f;
			percentual_outline_width = 0.0f;
			percentual_halo = 0.0f;			
		}

		point_renderer::point_renderer()
		{
			has_point_sizes = false;
			has_group_point_sizes = false;
			has_indexed_colors = false;
			///
			reference_point_size = 0.01f;
			y_view_angle = 45;
		}
		///
		void point_renderer::set_reference_point_size(float _reference_point_size)
		{
			reference_point_size = _reference_point_size;
		}
		///
		void point_renderer::set_y_view_angle(float _y_view_angle)
		{
			y_view_angle = _y_view_angle;
		}

		bool point_renderer::init(cgv::render::context& ctx)
		{
			bool res = renderer::init(ctx);
			if (!ref_prog().is_created()) {
				if (!ref_prog().build_program(ctx, "point.glpr", true)) {
					std::cerr << "ERROR in point_renderer::init() ... could not build program pc.glpr" << std::endl;
					return false;
				}
			}
			return res;
		}
		
		bool point_renderer::validate_attributes(context& ctx) const
		{
			const point_render_style& prs = get_style<point_render_style>();
			bool res;
			if (!prs.use_group_color) {
				if (has_indexed_colors) {
					if (has_colors)
						ctx.error("point_renderer::validate_attributes() both point color and color index attributes set, using color index");
					bool tmp = has_colors;
					has_colors = true;
					res = surface_renderer::validate_attributes(ctx);
					has_colors = tmp;
				}
				else
					res = surface_renderer::validate_attributes(ctx);
			}
			else
				res = surface_renderer::validate_attributes(ctx);
			if (!has_group_point_sizes && prs.use_group_point_size) {
				ctx.error("point_renderer::validate_attributes() group_point_sizes not set");
				res = false;
			}
			return res;
		}
		bool point_renderer::enable(cgv::render::context& ctx)
		{
			const point_render_style& prs = get_style<point_render_style>();

			bool res;
			if (!prs.use_group_color && has_indexed_colors) {
				bool tmp = has_colors;
				has_colors = true;
				res = surface_renderer::enable(ctx);
				has_colors = tmp;
			}
			else
				res = surface_renderer::enable(ctx);

			if (!prs.use_point_shader)
				ref_prog().disable(ctx);

			glPointSize(prs.point_size);
			if (prs.blend_points) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			if (prs.use_point_shader && ref_prog().is_linked()) {
				if (!has_point_sizes)
					ref_prog().set_attribute(ctx, "point_size", prs.point_size);
				ref_prog().set_uniform(ctx, "use_color_index", has_indexed_colors);
				ref_prog().set_uniform(ctx, "measure_point_size_in_pixel", prs.measure_point_size_in_pixel);
				ref_prog().set_uniform(ctx, "reference_point_size", reference_point_size);
				ref_prog().set_uniform(ctx, "use_group_point_size", prs.use_group_point_size);
				ref_prog().set_uniform(ctx, "smooth_points", prs.smooth_points);
				ref_prog().set_uniform(ctx, "orient_splats", has_normals ? prs.orient_splats : false);
				float pixel_extent_per_depth = (float)(2.0*tan(0.5*0.0174532925199*y_view_angle) / ctx.get_height());
				ref_prog().set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);
				ref_prog().set_uniform(ctx, "outline_width_from_pixel", prs.outline_width_from_pixel);
				ref_prog().set_uniform(ctx, "percentual_outline_width", 0.01f*prs.percentual_outline_width);
				ref_prog().set_uniform(ctx, "percentual_halo", 0.01f*prs.percentual_halo);
				ref_prog().set_uniform(ctx, "halo_color", prs.halo_color);
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
			return res;
		}

		bool point_renderer::disable(cgv::render::context& ctx)
		{
			const point_render_style& prs = get_style<point_render_style>();
			if (!prs.use_point_shader) {
				if (prs.point_size > 1 && prs.smooth_points) {
					glDisable(GL_POINT_SMOOTH);
				}
				if (prs.illumination_mode == 0 || !has_normals)
					glEnable(GL_LIGHTING);
				else {
					glEnable(GL_COLOR_MATERIAL);
					glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
				}
				if (ref_prog().is_linked())
					ref_prog().enable(ctx);
			}
			if (prs.blend_points) {
				glDisable(GL_BLEND);
			}
			if (prs.culling_mode != CM_OFF) {
				glDisable(GL_CULL_FACE);
			}
			if (!attributes_persist()) {
				has_indexed_colors = false;
				has_point_sizes = false;
			}
			return surface_renderer::disable(ctx);
		}
	}
}

namespace cgv {
	namespace reflect {
		namespace render {
			bool point_render_style::self_reflect(cgv::reflect::reflection_handler& rh)
			{
				return
					rh.reflect_base(*static_cast<cgv::render::surface_render_style*>(this)) &&
					rh.reflect_member("point_size", point_size) &&
					rh.reflect_member("use_group_point_size", use_group_point_size) &&
					rh.reflect_member("measure_point_size_in_pixel", measure_point_size_in_pixel) &&
					rh.reflect_member("smooth_points", smooth_points) &&
					rh.reflect_member("blend_points", blend_points) &&
					rh.reflect_member("use_point_shader", use_point_shader) &&
					rh.reflect_member("orient_splats", orient_splats) &&
					rh.reflect_member("outline_width_from_pixel", outline_width_from_pixel) &&
					rh.reflect_member("percentual_halo", percentual_halo) &&
					rh.reflect_member("halo_color", halo_color) &&
					rh.reflect_member("percentual_outline_width", percentual_outline_width);
			}

		}
		cgv::reflect::extern_reflection_traits<cgv::render::point_render_style, cgv::reflect::render::point_render_style> get_reflection_traits(const cgv::render::point_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<cgv::render::point_render_style, cgv::reflect::render::point_render_style>();
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
				p->add_member_control(b, "measure_point_size_in_pixel", prs_ptr->measure_point_size_in_pixel, "toggle");
				p->add_member_control(b, "smooth", prs_ptr->smooth_points, "toggle");
				p->add_member_control(b, "blend", prs_ptr->blend_points, "toggle");
				p->add_member_control(b, "use_shader", prs_ptr->use_point_shader, "check");
				p->add_member_control(b, "orient_splats", prs_ptr->orient_splats, "check");
				p->add_member_control(b, "outline_width_from_pixel", prs_ptr->outline_width_from_pixel, "value_slider", "min=0;max=10;ticks=true");
				p->add_member_control(b, "percentual_outline_width", prs_ptr->percentual_outline_width, "value_slider", "min=0;max=100;ticks=true");
				p->add_member_control(b, "percentual_halo", prs_ptr->percentual_halo, "value_slider", "min=0;max=100;ticks=true");
				p->add_member_control(b, "halo_color", prs_ptr->halo_color);
				p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style*>(prs_ptr));
				return true;
			}
		};

#include "gl/lib_begin.h"

		extern CGV_API cgv::gui::gui_creator_registration<point_render_style_gui_creator> prs_gc_reg("point_render_style_gui_creator");

	}
}