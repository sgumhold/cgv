#include "line_renderer.h"
#include <cgv_reflect_types/media/color.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {

		line_render_style::line_render_style() : line_color(0, 1, 1, 1)
		{
			line_width = 1.0f;
		}

		/// check manager for line width array
		void line_renderer::set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr)
		{
			group_renderer::set_attribute_array_manager(ctx, _aam_ptr);
			if (aam_ptr) {
				if (aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, "line_width")))
					has_line_widths = true;
			}
			else {
				has_line_widths = false;
			}

		}

		line_renderer::line_renderer()
		{
			has_line_widths = false;
		}

		bool line_renderer::enable(context& ctx)
		{
			const line_render_style& lrs = get_style<line_render_style>();
			glLineWidth(lrs.line_width);
			if (!group_renderer::enable(ctx))
				return false;
			ctx.set_color(lrs.line_color);
			return true;
		}

		void line_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{	
			draw_impl(ctx, PT_LINES, start, count, false, false, -1);
		}

		bool line_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<cgv::render::group_render_style*>(this)) &&
				rh.reflect_member("line_width", line_width) &&
				rh.reflect_member("line_color", line_color);
		}

		cgv::reflect::extern_reflection_traits<line_render_style, line_render_style_reflect> get_reflection_traits(const line_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<line_render_style, line_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct line_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::line_render_style>::get_name())
					return false;
				cgv::render::line_render_style* lrs_ptr = reinterpret_cast<cgv::render::line_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
				p->add_member_control(b, "line_color", lrs_ptr->line_color);
				p->add_member_control(b, "line_width", lrs_ptr->line_width, "value_slider", "min=1;max=20;log=true;ticks=true");
				p->add_gui("group render style", *static_cast<cgv::render::group_render_style*>(lrs_ptr));
				return true;
			}
		};

#include "gl/lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<line_render_style_gui_creator> line_rs_gc_reg("line_render_style_gui_creator");

	}
}