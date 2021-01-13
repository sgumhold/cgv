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
			float CLOD = 1.f;
			float spacing = 1.f; //root spacing
			float scale = 1.f;

		protected:

			void draw_and_compute_impl(context& ctx, PrimitiveType type, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index);
		public:
			render_style* create_render_style() const;

			bool init(context& ctx);

			bool enable(context& ctx);
			
			bool disable(context& ctx);

			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		private:
			void add_shader(context& ctx, const std::string& sf, const cgv::render::ShaderType st);

		};
	}
}
#include <cgv/config/lib_end.h>