#include "rounded_cone_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		rounded_cone_renderer& ref_rounded_cone_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static rounded_cone_renderer r;
			r.manage_singelton(ctx, "rounded_cone_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* rounded_cone_renderer::create_render_style() const
		{
			return new surface_render_style();
		}

		rounded_cone_renderer::rounded_cone_renderer()
		{
			eye_pos = vec3(0.0f);
		}

		void rounded_cone_renderer::set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr)
		{
			surface_renderer::set_attribute_array_manager(ctx, _aam_ptr);
		}

		void rounded_cone_renderer::set_eye_position(vec3 eye_position)
		{
			eye_pos = eye_position;
		}

		bool rounded_cone_renderer::validate_attributes(const context& ctx) const
		{
			// validate set attributes
			const surface_render_style& srs = get_style<surface_render_style>();
			bool res = surface_renderer::validate_attributes(ctx);
			return res;
		}
		bool rounded_cone_renderer::init(cgv::render::context& ctx)
		{
			bool res = renderer::init(ctx);
			if (!ref_prog().is_created()) {
				if (!ref_prog().build_program(ctx, "rounded_cone.glpr", true)) {
					std::cerr << "ERROR in rounded_cone_renderer::init() ... could not build program rounded_cone.glpr" << std::endl;
					return false;
				}
			}
			return res;
		}

		/// 
		bool rounded_cone_renderer::enable(context& ctx)
		{
			if (!surface_renderer::enable(ctx))
				return false;
			ref_prog().set_uniform(ctx, "eye", eye_pos);
			return true;
		}
		///
		bool rounded_cone_renderer::disable(context& ctx)
		{
			return surface_renderer::disable(ctx);
		}
	}
}
