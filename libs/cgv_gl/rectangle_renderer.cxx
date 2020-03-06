#include "rectangle_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		rectangle_renderer& ref_rectangle_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static rectangle_renderer r;
			r.manage_singelton(ctx, "rectangle_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* rectangle_renderer::create_render_style() const
		{
			return new surface_render_style();
		}

		rectangle_renderer::rectangle_renderer() {
			has_extents = false;
			has_translations = false;
			has_rotations = false;
		}

		void rectangle_renderer::set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr)
		{
			surface_renderer::set_attribute_array_manager(ctx, _aam_ptr);
			if (aam_ptr) {
				if(aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, "extent")))
					has_extents = true;
				if(aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, "translation")))
					has_translations = true;
				if(aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, "rotation")))
					has_rotations = true;
			}
			else {
				has_extents = false;
				has_translations = false;
				has_rotations = false;
			}
		}
		
		bool rectangle_renderer::init(context& ctx)
		{
			bool res = renderer::init(ctx);
			if (!ref_prog().is_created()) {
				if (!ref_prog().build_program(ctx, "rectangle.glpr", true)) {
					std::cerr << "ERROR in rectangle_renderer::init() ... could not build program plane.glpr" << std::endl;
					return false;
				}
			}
			return res;
		}

		bool rectangle_renderer::validate_attributes(const context& ctx) const
		{
			const surface_render_style& srs = get_style<surface_render_style>();
			bool res = surface_renderer::validate_attributes(ctx);
			if(!has_extents) {
				ctx.error("rectangle_renderer::enable() extent attribute not set");
				res = false;
			}
			return res;
		}
		bool rectangle_renderer::enable(cgv::render::context& ctx)
		{
			if(!surface_renderer::enable(ctx))
				return false;
			ref_prog().set_uniform(ctx, "has_rotations", has_rotations);
			ref_prog().set_uniform(ctx, "has_translations", has_translations);
			return true;
		}

		bool rectangle_renderer::disable(cgv::render::context& ctx)
		{
			if(!attributes_persist()) {
				has_extents = false;
				has_rotations = false;
				has_translations = false;
			}

			return surface_renderer::disable(ctx);
		}

		void rectangle_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_POINTS, start, count, false, false, -1);
		}
	}
}
