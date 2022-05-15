#pragma once

#include <cgv_glutil/overlay.h>
#include <libs/plot/plot_base.h>
#include <cgv/gui/mouse_event.h>
#include "lib_begin.h"

namespace stream_vis {

	class CGV_API view_overlay : public cgv::glutil::overlay
	{
	public:
		char toggle_key = 0;
	protected:
		bool mouse_is_on_overlay = false;
		std::vector<std::pair<int,cgv::plot::plot_base*>> plots;
		virtual void update_views() = 0;
		virtual void set_modelview_projection(cgv::render::context& ctx) = 0;
		virtual void toggle_default_view(bool set_default) = 0;
		virtual bool handle_mouse_event(const cgv::gui::mouse_event& me) = 0;
	public:
		view_overlay();
		void add_plot(int pi, cgv::plot::plot_base* plot_ptr);
		std::string get_type_name() const { return "view_overlay"; }
		void init_frame(cgv::render::context& ctx);
		void draw(cgv::render::context& ctx);
		bool handle_event(cgv::gui::event& e);
	};
}

#include <cgv/config/lib_end.h>