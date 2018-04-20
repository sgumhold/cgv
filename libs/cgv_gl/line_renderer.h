#pragma once

#include "group_renderer.h"
#include <cgv/media/illum/phong_material.hh>

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<

		/** style of a line */
		struct CGV_API line_render_style : public group_render_style
		{
			float line_width;
			cgv::media::illum::phong_material::color_type line_color;
			line_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API line_renderer : public group_renderer
		{
		public:
			line_renderer();
			bool enable(context& ctx);
		};
	}
}

namespace cgv {
	namespace reflect {
		namespace render {
			struct CGV_API line_render_style : public cgv::render::line_render_style
			{
				bool self_reflect(cgv::reflect::reflection_handler& rh);
			};
		}
		extern CGV_API cgv::reflect::extern_reflection_traits<cgv::render::line_render_style, cgv::reflect::render::line_render_style> get_reflection_traits(const cgv::render::line_render_style&);
	}
}

#include <cgv/config/lib_end.h>