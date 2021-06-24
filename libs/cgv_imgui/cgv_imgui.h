#pragma once

#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/gui/event_handler.h>
#include <chrono>

#include "lib_begin.h"

namespace cgv {
	namespace imgui {

		class CGV_API cgv_imgui : public cgv::render::drawable, public cgv::gui::event_handler
		{
			void* font_atlas;
			std::string ttf_font_file_name;
			float ttf_font_size;
		protected:
			unsigned width, height;
			cgv::render::texture tex;
			cgv::render::frame_buffer fbo;
			std::chrono::high_resolution_clock::time_point g_Time;
			bool mouse_down[3];
			bool has_focus;
			int cursor_x, cursor_y;
			bool show_demo_window;
			bool show_another_window;
			rgba clear_color;
			bool use_offline_rendering;
			void prepare_imgui_draw_data(int w, int h, float scale_x = 1.0f, float scale_y = 1.0f);
			void draw_imgui_data(cgv::render::context& ctx, bool do_clear = true);
		public:
			/// construct
			cgv_imgui(bool _use_offline_rendering, const char* _ttf_font_file_name = 0, float _ttf_font_size = 18.0f);
			/// destructor
			~cgv_imgui();
			/// overload to create ui
			virtual void create_imgui();
			/// update window size
			void resize(unsigned int w, unsigned int h);
			/// construct offline render target
			bool init(cgv::render::context& ctx);
			/// destruct offline render targets
			void clear(cgv::render::context& ctx);
			/// evaluate user interface
			void init_frame(cgv::render::context& ctx);
			/// draw user interface
			void after_finish(cgv::render::context& ctx);
			/// translate events to imgui
			bool handle(cgv::gui::event& e);
		};
	}
}

#include <cgv/config/lib_end.h>