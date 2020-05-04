#include "box_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		box_renderer& ref_box_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static box_renderer r;
			r.manage_singelton(ctx, "box_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* box_renderer::create_render_style() const
		{
			return new surface_render_style();
		}

		box_renderer::box_renderer()
		{
			has_extents = false;
			position_is_center = true;
			has_translations = false;
			has_rotations = false;
		}
		void box_renderer::set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr)
		{
			surface_renderer::set_attribute_array_manager(ctx, _aam_ptr);
			if (aam_ptr) {
				if (aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, "extent")))
					has_extents = true;
				if (aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, "translation")))
					has_translations= true;
				if (aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, "rotation")))
					has_rotations= true;
			}
			else {
				has_extents = false;
				has_translations = false;
				has_rotations = false;
			}
		}

		/// set the flag, whether the position is interpreted as the box center
		void box_renderer::set_position_is_center(bool _position_is_center)
		{
			position_is_center = _position_is_center;
		}

		bool box_renderer::validate_attributes(const context& ctx) const
		{
			// validate set attributes
			const surface_render_style& srs = get_style<surface_render_style>();
			bool res = surface_renderer::validate_attributes(ctx);
			if (!has_extents) {
				ctx.error("box_renderer::enable() extent attribute not set");
				res = false;
			}
			return res;
		}
		bool box_renderer::init(cgv::render::context& ctx)
		{
			bool res = renderer::init(ctx);
			if (!ref_prog().is_created()) {
				if (!ref_prog().build_program(ctx, "box.glpr", true)) {
					std::cerr << "ERROR in box_renderer::init() ... could not build program box.glpr" << std::endl;
					return false;
				}
			}
			return res;
		}

		/// 
		bool box_renderer::enable(context& ctx)
		{
			if (!surface_renderer::enable(ctx))
				return false;
			ref_prog().set_uniform(ctx, "position_is_center", position_is_center);
			ref_prog().set_uniform(ctx, "has_rotations", has_rotations);
			ref_prog().set_uniform(ctx, "has_translations", has_translations);
			return true;
		}
		///
		bool box_renderer::disable(context& ctx)
		{
			if (!attributes_persist()) {
				has_extents = false;
				position_is_center = true;
				has_rotations = false;
				has_translations = false;
			}

			return surface_renderer::disable(ctx);
		}

		void box_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_POINTS, start, count, false, false, -1);
		}
	}
}
