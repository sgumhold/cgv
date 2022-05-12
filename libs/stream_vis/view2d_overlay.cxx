#include "view2d_overlay.h"
#include <cgv/math/ftransform.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/key_event.h>

namespace stream_vis {

	view2d_overlay::view2d_overlay()
	{
		set_overlay_alignment(AO_START, AO_START);
		set_overlay_stretch(SO_BOTH);
		set_overlay_margin(ivec2(-3));
		set_overlay_size(ivec2(600u, 600u));
	}
	void view2d_overlay::add_plot(int pi, cgv::plot::plot_base* plot_ptr)
	{
		plots.push_back({ pi, plot_ptr });
	}
	void view2d_overlay::set_update_handler(view2d_update_handler* _handler)
	{
		handler = _handler;
	}
	void view2d_overlay::update_views()
	{
		ivec2 os = get_overlay_size();
		for (const auto& p : plots) {
			vec2 pixel_scales;
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
	void view2d_overlay::init_frame(cgv::render::context& ctx)
	{
		if (ensure_overlay_layout(ctx))
			update_views();
	}
	void view2d_overlay::draw(cgv::render::context& ctx)
	{
		ivec2 container_size = get_overlay_size();
		float aspect = (float)container_size[0]/ container_size[1];
		float extent_x = 0.5f * view_width / zoom_factor;
		float extent_y = extent_x / aspect;
		ctx.push_window_transformation_array();
		ctx.push_projection_matrix();
		ctx.push_modelview_matrix();
		ctx.set_projection_matrix(cgv::math::ortho4<float>(pan_pos(0) - extent_x, pan_pos(0) + extent_x,
			pan_pos(1) - extent_y, pan_pos(1) + extent_y, -10.0f, 10.0f));
		ctx.set_modelview_matrix(cgv::math::identity4<float>());
		ivec4 vp;
		(ivec2&)vp = get_overlay_position();
		((ivec2*)&vp)[1] = get_overlay_size();
		ctx.set_viewport(vp);

		for (auto p : plots)
			p.second->draw(ctx);

		ctx.pop_modelview_matrix();
		ctx.pop_projection_matrix();
		ctx.pop_window_transformation_array();
	}
	bool view2d_overlay::handle_event(cgv::gui::event& e)
	{
		if (e.get_kind() == cgv::gui::EID_KEY) {
			auto& ke = reinterpret_cast<cgv::gui::key_event&>(e);
			if (ke.get_action() != cgv::gui::KA_RELEASE) {
				switch (ke.get_key()) {
				case cgv::gui::KEY_Space:
					if (ke.get_modifiers() == cgv::gui::EM_CTRL) {
						initial_pan_pos = pan_pos;
						initial_zoom_factor = zoom_factor;
						on_set(&initial_pan_pos(0));
						on_set(&initial_pan_pos(1));
						on_set(&initial_zoom_factor);
						return true;
					}
					else if (ke.get_modifiers() == cgv::gui::EM_SHIFT) {
						pan_pos = initial_pan_pos;
						zoom_factor = initial_zoom_factor;
						on_set(&pan_pos(0));
						on_set(&pan_pos(1));
						on_set(&zoom_factor);
						return true;
					}
					break;
				}
			}
		}
		else if (e.get_kind() == cgv::gui::EID_MOUSE) {
			auto& me = reinterpret_cast<cgv::gui::mouse_event&>(e);
			switch (me.get_action()) {
			case cgv::gui::MA_PRESS:
				if (me.get_button() == cgv::gui::MB_RIGHT_BUTTON) {
					pan_start_x = me.get_x();
					pan_start_y = me.get_y();
					pan_start_pos = pan_pos;
					return true;
				}
				break;
			case cgv::gui::MA_DRAG:
				if (me.get_button_state() == cgv::gui::MB_RIGHT_BUTTON) {
					int dx = me.get_x() - pan_start_x;
					int dy = me.get_y() - pan_start_y;
					float aspect = (float)get_context()->get_width() / get_context()->get_height();
					pan_pos(0) = pan_start_pos(0) - (float)dx / get_context()->get_width() * view_width / zoom_factor;
					pan_pos(1) = pan_start_pos(1) + (float)dy / get_context()->get_height() * view_width / zoom_factor / aspect;
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
		}
		return false;
	}
	void view2d_overlay::create_gui()
	{
		add_decorator(std::string("View2D ")+get_name(), "heading", "level=1");
		add_member_control(this, "view_width", view_width, "value_slider", "min=0.2;max=5;log=true;ticks=true");
		add_member_control(this, "zoom_sensitivity", zoom_sensitivity, "value_slider", "min=0.01;max=1;log=true;ticks=true");
		add_member_control(this, "zoom_factor", zoom_factor, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		add_member_control(this, "pan_x", pan_pos(0), "value_slider", "min=-5;max=5;ticks=true");
		add_member_control(this, "pan_y", pan_pos(1), "value_slider", "min=-5;max=5;ticks=true");
	}
}