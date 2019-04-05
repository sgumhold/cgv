#pragma once

#include "group_renderer.h"
#include <cgv/media/color.h>

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<

		/** style of a line */
		struct CGV_API line_render_style : public group_render_style
		{
			typedef cgv::media::color<float, cgv::media::RGB, cgv::media::OPACITY> color_type;
			float line_width;
			color_type line_color;
			line_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API line_renderer : public group_renderer
		{
		public:
			line_renderer();
			bool enable(context& ctx);
		};
	
		struct CGV_API line_render_style_reflect : public line_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<line_render_style, line_render_style_reflect> get_reflection_traits(const line_render_style&);
	}
}

#include <cgv/config/lib_end.h>