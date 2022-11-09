#include "normal_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		normal_renderer& ref_normal_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static normal_renderer r;
			r.manage_singleton(ctx, "normal_renderer", ref_count, ref_count_change);
			return r;
		}

		normal_render_style::normal_render_style()
		{
			normal_length = 1.0f;
		}

		normal_renderer::normal_renderer()
		{
			normal_scale = 1.0f;
		}
		/// the normal scale is multiplied to the normal length of the normal render style
		void normal_renderer::set_normal_scale(float _normal_scale)
		{
			normal_scale = _normal_scale;
		}

		render_style* normal_renderer::create_render_style() const
		{
			return new line_render_style();
		}
		bool normal_renderer::validate_attributes(const context& ctx) const
		{
			// validate set attributes
			bool res = line_renderer::validate_attributes(ctx);
			if (!has_normals) {
				ctx.error("normal_renderer::enable() normal attribute not set");
				res = false;
			}
			return res;
		}
		bool normal_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines)
		{
			return prog.build_program(ctx, "normal.glpr", true, defines);
		}

		bool normal_renderer::enable(context& ctx)
		{
			const normal_render_style& nrs = get_style<normal_render_style>();
			if (!line_renderer::enable(ctx))
				return false;
			ref_prog().set_uniform(ctx, "normal_length", nrs.normal_length * normal_scale);
			return true;
		}

		void normal_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_POINTS, start, count, false, false, -1);
		}

		bool normal_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<line_render_style*>(this)) &&
				rh.reflect_member("normal_length", normal_length);
		}

		cgv::reflect::extern_reflection_traits<normal_render_style, normal_render_style_reflect> get_reflection_traits(const normal_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<normal_render_style, normal_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct normal_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::normal_render_style>::get_name())
					return false;
				cgv::render::normal_render_style* nrs_ptr = reinterpret_cast<cgv::render::normal_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
				p->add_member_control(b, "Normal Length", nrs_ptr->normal_length, "value_slider", "min=0.0001;step=0.0000001;max=10;log=true;ticks=true");
				p->add_gui("line render style", *static_cast<cgv::render::line_render_style*>(nrs_ptr));
				return true;
			}
		};

#include "gl/lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<normal_render_style_gui_creator> normal_rs_gc_reg("normal_render_style_gui_creator");

	}
}