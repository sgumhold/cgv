#pragma once

#include <cgv_glutil/overlay.h>
#include <libs/plot/plot_base.h>
#include "lib_begin.h"

namespace stream_vis {

	struct view2d_update_handler : public cgv::render::render_types
	{
		virtual void handle_view2d_update(int pi, const vec2& pixel_scales) = 0;
	};

	class CGV_API view2d_overlay : public cgv::glutil::overlay
	{
	protected:
		int pan_start_x = 0, pan_start_y = 0;
		vec2 pan_start_pos = vec2(0.0f);
		float zoom_factor = 1.0f;
		float view_width = 2.0f;
		vec2 pan_pos = vec2(0.0f);
		vec2 initial_pan_pos = vec2(0.0f);
		float initial_zoom_factor = 1.0f;
		float zoom_sensitivity = 0.1f;
		bool mouse_is_on_overlay = false;
		std::vector<std::pair<int,cgv::plot::plot_base*>> plots;
		view2d_update_handler* handler = 0;
		void update_views();
	public:
		view2d_overlay();
		void set_update_handler(view2d_update_handler* _handler);
		void add_plot(int pi, cgv::plot::plot_base* plot_ptr);
		std::string get_type_name() const { return "view2d_overlay"; }
		void on_set(void* member_ptr);
		void init_frame(cgv::render::context& ctx);
		void draw(cgv::render::context& ctx);
		bool handle_event(cgv::gui::event& e);
		void create_gui();
	};
}

#include <cgv/config/lib_end.h>