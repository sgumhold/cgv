#include "box_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {

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

		/// set the flag, whether the position is interpreted as the box center
		void box_renderer::set_position_is_center(bool _position_is_center)
		{
			position_is_center = _position_is_center;
		}

		bool box_renderer::validate_attributes(context& ctx)
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
			return true;
		}

	}
}
