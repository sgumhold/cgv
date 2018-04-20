#include "line_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {

		line_render_style::line_render_style()
		{
			line_width = 1.0f;
			line_color = cgv::media::illum::phong_material::color_type(0, 1, 1);
		}

		line_renderer::line_renderer()
		{
		}

		bool line_renderer::enable(context& ctx)
		{
			const line_render_style& lrs = get_style<line_render_style>();
			glLineWidth(lrs.line_width);
			glColor4fv(&lrs.line_color[0]);
			return group_renderer::enable(ctx);
		}
	}
}

namespace cgv {
	namespace reflect {
		namespace render {
			bool line_render_style::self_reflect(cgv::reflect::reflection_handler& rh)
			{
				return
					rh.reflect_base(*static_cast<group_render_style*>(this)) &&
					rh.reflect_member("line_width", line_width) &&
					rh.reflect_member("line_color", line_color);
			}

		}
		cgv::reflect::extern_reflection_traits<cgv::render::line_render_style, cgv::reflect::render::line_render_style> get_reflection_traits(const cgv::render::line_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<cgv::render::line_render_style, cgv::reflect::render::line_render_style>();
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

		extern CGV_API cgv::gui::gui_creator_registration<line_render_style_gui_creator> frs_gc_reg("line_render_style_gui_creator");

	}
}