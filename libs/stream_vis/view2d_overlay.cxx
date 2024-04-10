#include "view2d_overlay.h"
#include <cgv/gui/theme_info.h>
#include <cgv/math/ftransform.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/key_event.h>

namespace stream_vis {
	view2d_overlay::view2d_overlay()
	{
	}
	void view2d_overlay::update_views()
	{
		cgv::ivec2 os = get_rectangle().size;
		for (const auto& p : plots) {
			cgv::vec2 pixel_scales;
			for (int ai = 0; ai < 2; ++ai)
				pixel_scales[ai] = p.second->get_domain_config_ptr()->axis_configs[ai].extent * zoom_factor * os[ai];
			if (handler)
				handler->handle_view2d_update(p.first, pixel_scales);
		}
	}
	void view2d_overlay::on_set(void* member_ptr)
	{
		if (member_ptr == &zoom_factor)
			update_views();
		update_member(member_ptr);
		post_redraw();
	}
	void view2d_overlay::toggle_default_view(bool set_default)
	{
		if (set_default) {
			initial_pan_pos = pan_pos;
			initial_zoom_factor = zoom_factor;
			on_set(&initial_pan_pos(0));
			on_set(&initial_pan_pos(1));
			on_set(&initial_zoom_factor);
		}
		else {
			pan_pos = initial_pan_pos;
			zoom_factor = initial_zoom_factor;
			on_set(&pan_pos(0));
			on_set(&pan_pos(1));
			on_set(&zoom_factor);
		}
	}
	void view2d_overlay::set_modelview_projection(cgv::render::context& ctx)
	{
		cgv::ivec2 os = get_rectangle().size;
		float aspect = (float)os[0]/ os[1];
		float extent_x = 0.5f * view_width / zoom_factor;
		float extent_y = extent_x / aspect;
		ctx.set_projection_matrix(cgv::math::ortho4<float>(pan_pos(0) - extent_x, pan_pos(0) + extent_x,
			pan_pos(1) - extent_y, pan_pos(1) + extent_y, -10.0f, 10.0f));
		ctx.set_modelview_matrix(cgv::math::identity4<float>());
		MPW = ctx.get_modelview_projection_window_matrix();
	}
	bool view2d_overlay::handle_mouse_event(const cgv::gui::mouse_event& me)
	{
		switch (me.get_action()) {
		case cgv::gui::MA_PRESS:
			if (me.get_button() == cgv::gui::MB_RIGHT_BUTTON) {
				pan_start_x = me.get_x();
				pan_start_y = me.get_y();
				pan_start_pos = pan_pos;
				return true;
			}
			break;
		case cgv::gui::MA_MOVE:
			{
				int x_gl = me.get_x();
				int y_gl = get_context()->get_height() - 1 - me.get_y();
				cgv::vec3 p = get_context()->get_model_point(cgv::dvec3(me.get_x(), y_gl, 0.0f), MPW);
				//std::cout << "x=" << x_gl << ", y=" << y_gl << " -> " << p << std::endl;
				int pi = -1;
				for (int pj = 0; pj < plots.size(); ++pj) {
					const auto& pp = plots[pj];
					cgv::vec2 e = cgv::vec2::from_vec(pp.second->get_extent());
					cgv::vec2 c = (cgv::vec2&)pp.second->get_center();
					cgv::box2 b(c - 0.5f * e, c + 0.5f * e);
					//std::cout << pi << " | " << pp.second->get_domain().get_min_pnt() << " -> " << pp.second->get_domain().get_max_pnt() << std::endl;
					if (b.inside((const cgv::vec2&)(p)))
						pi = pj;
				}
				if (current_pi != pi) {
//					std::cout << "focus plot changed from " << current_pi << " to " << pi << std::endl;
					current_pi = pi;
				}
			}
			break;
		case cgv::gui::MA_DRAG:
			if (me.get_button_state() == cgv::gui::MB_RIGHT_BUTTON) {
				int dx = me.get_x() - pan_start_x;
				int dy = me.get_y() - pan_start_y;
				cgv::ivec2 os = get_rectangle().size;
				float aspect = (float)os[0]/os[1];
				pan_pos(0) = pan_start_pos(0) - (float)dx/os[0] * view_width / zoom_factor;
				pan_pos(1) = pan_start_pos(1) + (float)dy/os[1] * view_width / zoom_factor / aspect;
				on_set(&pan_pos(0));
				on_set(&pan_pos(1));
				return true;
			}
			break;
		case cgv::gui::MA_WHEEL:
			zoom_factor *= exp(-zoom_sensitivity * me.get_dy());
			on_set(&zoom_factor);
			return true;
		}
		return false;
	}
	void view2d_overlay::create_gui_impl()
	{
		add_decorator(std::string("View2D ")+get_name(), "heading", "level=1");
		add_member_control(this, "view_width", view_width, "value_slider", "min=0.2;max=5;log=true;ticks=true");
		add_member_control(this, "zoom_sensitivity", zoom_sensitivity, "value_slider", "min=0.01;max=1;log=true;ticks=true");
		add_member_control(this, "zoom_factor", zoom_factor, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		add_member_control(this, "pan_x", pan_pos(0), "value_slider", "min=-5;max=5;ticks=true");
		add_member_control(this, "pan_y", pan_pos(1), "value_slider", "min=-5;max=5;ticks=true");
	}
}