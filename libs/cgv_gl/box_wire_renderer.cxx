#include "box_wire_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		box_wire_renderer& ref_box_wire_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static box_wire_renderer r;
			r.manage_singelton(ctx, "box_wire_renderer", ref_count, ref_count_change);
			return r;
		}

		/// overload to allow instantiation of box_wire_renderer
		render_style* box_wire_renderer::create_render_style() const
		{
			return new line_render_style();
		}

		box_wire_renderer::box_wire_renderer()
		{
			has_extents = false;
			position_is_center = true;
			has_translations = false;
			has_rotations = false;
		}
		void box_wire_renderer::set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr)
		{
			line_renderer::set_attribute_array_manager(ctx, _aam_ptr);
			if (aam_ptr) {
				if (aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, "extent")))
					has_extents = true;
				if (aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, "translation")))
					has_translations = true;
				if (aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, "rotation")))
					has_rotations = true;
			}
			else {
				has_extents = false;
				has_translations = false;
				has_rotations = false;
			}
		}
		/// set the flag, whether the position is interpreted as the box center
		void box_wire_renderer::set_position_is_center(bool _position_is_center)
		{
			position_is_center = _position_is_center;
		}


		bool box_wire_renderer::init(context& ctx)
		{
			bool res = renderer::init(ctx);
			if (!ref_prog().is_created()) {
				if (!ref_prog().build_program(ctx, "box_wire.glpr", true)) {
					std::cerr << "ERROR in box_wire_renderer::init() ... could not build program box_wire.glpr" << std::endl;
					return false;
				}
			}
			return res;
		}
		bool box_wire_renderer::validate_attributes(const context& ctx) const
		{
			// validate set attributes
			bool res = line_renderer::validate_attributes(ctx);
			if (!has_extents) {
				ctx.error("box_renderer::enable() extent attribute not set");
				res = false;
			}
			return res;
		}
		/// 
		bool box_wire_renderer::enable(context& ctx)
		{
			if (!line_renderer::enable(ctx))
				return false;
			ref_prog().set_uniform(ctx, "position_is_center", position_is_center);
			ref_prog().set_uniform(ctx, "has_rotations", has_rotations);
			ref_prog().set_uniform(ctx, "has_translations", has_translations);
			return true;
		}
		///
		bool box_wire_renderer::disable(context& ctx)
		{
			if (!attributes_persist()) {
				has_extents = false;
				position_is_center = true;
				has_rotations = false;
				has_translations = false;
			}

			return line_renderer::disable(ctx);
		}
		
		void box_wire_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_POINTS, start, count, false, false, -1);
		}
	}
}