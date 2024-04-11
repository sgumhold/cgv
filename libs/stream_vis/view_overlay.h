#pragma once

#include <cgv_app/overlay.h>
#include <libs/plot/plot_base.h>
#include <cgv/gui/mouse_event.h>
#include "lib_begin.h"

namespace stream_vis {

	struct axis_config
	{
		bool log_scaling;
		float log_minimum;
	};

	struct view_update_handler
	{
		virtual void handle_view2d_update(int pi, const cgv::vec2& pixel_scales) = 0;
		virtual void handle_view3d_update(int pi, const cgv::mat4& T, const cgv::ivec4& viewport) = 0;
		virtual void handle_plot_visibility_update(unsigned pi, bool is_visible) = 0;
		virtual void handle_plot_axis_update(unsigned pi, int ai, bool is_log) = 0;
		virtual void handle_subplot_visibility_update(unsigned pi, int si, bool is_visible) = 0;
	};

	class CGV_API view_overlay : public cgv::app::overlay
	{
	public:
		char toggle_key = 0;
		int current_pi = -1;
	protected:
		bool mouse_is_on_overlay = false;
		std::vector<std::pair<int,cgv::plot::plot_base*>> plots;
		view_update_handler* handler = 0;
		virtual void update_views() = 0;
		virtual void set_modelview_projection(cgv::render::context& ctx) = 0;
		virtual void toggle_default_view(bool set_default) = 0;
		virtual bool handle_mouse_event(const cgv::gui::mouse_event& me) = 0;
		virtual void on_visibility_change();
	public:
		view_overlay();
		void set_update_handler(view_update_handler* _handler);
		void add_plot(int pi, cgv::plot::plot_base* plot_ptr);
		std::string get_type_name() const { return "view_overlay"; }
		void init_frame(cgv::render::context& ctx);
		void draw(cgv::render::context& ctx);
		bool handle_event(cgv::gui::event& e);
	};
	typedef cgv::data::ref_ptr<view_overlay> view_overlay_ptr;

}

#include <cgv/config/lib_end.h>