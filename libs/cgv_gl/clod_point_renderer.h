#pragma once
#include <cgv/render/context.h>
#include <cgv_gl/point_renderer.h>

#include "gl/lib_begin.h"

// [WIP] clod point cloud renderer

namespace cgv {
	namespace render {

		/** render style for sphere rendere */
		struct CGV_API clod_point_render_style : public render_style
		{
			/*@name clod rendering attributes*/
			//@{
			
			//@}

			/// construct with default values
			clod_point_render_style();
		};


		class CGV_API clod_point_renderer : public cgv::render::renderer{
			shader_program prog;
		protected:
		public:
			render_style* create_render_style() const;

			bool init(context& ctx);

			bool enable(context& ctx);
			
			bool disable(context& ctx);
		};
	}
}
#include <cgv/config/lib_end.h>