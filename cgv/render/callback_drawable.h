#pragma once
 
#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv/signal/bool_signal.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {

		/** as a drawable can manage only a single context, this callback drawable is a work around for
			the case when a drawable should draw in several contexts (i.e. two windows). Then the 
			callback drawable can be registered in the second window and its callback signals can be 
			connected to methods in the primary drawable. */
		struct CGV_API callback_drawable : public cgv::base::node, public cgv::render::drawable
		{
		public:
			///
			cgv::signal::signal<unsigned, unsigned> resize_callback;
			///
			cgv::signal::bool_signal<cgv::render::context&> init_callback;
			///
			cgv::signal::signal<cgv::render::context&> clear_callback;
			///
			cgv::signal::signal<cgv::render::context&> init_frame_callback;
			///
			cgv::signal::signal<cgv::render::context&> draw_callback;
			///
			cgv::signal::signal<cgv::render::context&> finish_draw_callback;
			///
			cgv::signal::signal<cgv::render::context&> finish_frame_callback;
			///
			cgv::signal::signal<cgv::render::context&> after_finish_callback;
			///
			callback_drawable(const std::string& name);
			///
			void resize(unsigned w, unsigned h);
			///
			bool init(cgv::render::context& ctx);
			///
			void clear(cgv::render::context& ctx);
			///
			void init_frame(cgv::render::context& ctx);
			///
			void draw(cgv::render::context& ctx);
			///
			void finish_draw(cgv::render::context& ctx);
			///
			void finish_frame(cgv::render::context& ctx);
			///
			void after_finish(cgv::render::context& ctx);
		};
	}
}

#include <cgv/config/lib_end.h>