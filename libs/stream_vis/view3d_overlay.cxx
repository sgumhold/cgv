#include "view3d_overlay.h"
#include <cgv/gui/theme_info.h>
#include <cgv/math/ftransform.h>
#include <cgv/gui/animate.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/key_event.h>

namespace stream_vis {

	view3d_overlay::view3d_overlay()
	{
		set_overlay_alignment(AO_START, AO_START);
		set_overlay_stretch(SO_BOTH);
		set_overlay_margin(ivec2(-3));
		set_overlay_size(ivec2(600u, 600u));
		current_view.set_y_view_angle(50);
		current_view.set_y_extent_at_focus(4);
		last_view = current_view;
	}
	void view3d_overlay::set_current_view(const cgv::render::clipped_view& _current_view)
	{
		current_view = _current_view;
	}
	void view3d_overlay::set_default_view(const cgv::render::view& _default_view)
	{
		default_view = _default_view;
	}

	void view3d_overlay::add_plot(int pi, cgv::plot::plot_base* plot_ptr)
	{
		plots.push_back({ pi, plot_ptr });
	}
	void view3d_overlay::set_update_handler(view3d_update_handler* _handler)
	{
		handler = _handler;
	}
	void view3d_overlay::update_views()
	{
		ivec4 vp;
		mat4 PM, M;
		compute_matrices_and_viewport(PM, M, vp);
		PM *= M;
		for (const auto& p : plots) {
			vec3 axes_scales(1.0f);
			for (unsigned ai = 0; ai < p.second->get_dim(); ++ai)
				axes_scales[ai] = p.second->get_domain_config_ptr()->axis_configs[ai].extent;
			mat4 S = cgv::math::scale4<float>(axes_scales);
			if (handler)
				handler->handle_view3d_update(p.first, PM*S, vp);
		}
	}
	void view3d_overlay::on_set(void* member_ptr)
	{
		cgv::render::view* vp = &current_view;
		if (member_ptr >= vp && member_ptr < vp + 1)
			update_views();
		update_member(member_ptr);
		post_redraw();
	}
	void view3d_overlay::init_frame(cgv::render::context& ctx)
	{
		if (length(current_view.get_focus() - last_view.get_focus()) > 0.00001f ||
			length(current_view.get_view_dir() - last_view.get_view_dir()) > 0.00001f ||
			length(current_view.get_view_up_dir() - last_view.get_view_up_dir()) > 0.00001f ||
			std::abs(current_view.get_y_extent_at_focus() - last_view.get_y_extent_at_focus()) > 0.00001f ||
			std::abs(current_view.get_y_view_angle() - last_view.get_y_view_angle()) > 0.00001f ||
			ensure_overlay_layout(ctx)) {

			update_views();
			last_view = current_view;
		}
	}
	void view3d_overlay::compute_matrices_and_viewport(mat4& projection_matrix, mat4& modelview_matrix, ivec4& viewport)
	{
		(ivec2&)viewport = get_overlay_position();
		((ivec2*)&viewport)[1] = get_overlay_size();
		float aspect = (float)viewport[2] / viewport[3];
		projection_matrix = cgv::math::perspective4<float>((float)current_view.get_y_view_angle(), aspect, (float)current_view.get_z_near(), (float)current_view.get_z_far());
		modelview_matrix = cgv::math::look_at4<float>(current_view.get_eye(), current_view.get_focus(), current_view.get_view_up_dir());
	}
	void view3d_overlay::draw(cgv::render::context & ctx)
	{
		ivec4 vp;
		mat4 P, M;
		compute_matrices_and_viewport(P, M, vp);
		ctx.push_window_transformation_array();
		ctx.push_projection_matrix();
		ctx.push_modelview_matrix();
			ctx.set_viewport(vp);

			ctx.set_projection_matrix(cgv::math::identity4<float>());
			ctx.set_modelview_matrix(cgv::math::identity4<float>());
			glDepthMask(GL_FALSE);
			ctx.ref_default_shader_program().enable(ctx);
			auto& ti = cgv::gui::theme_info::instance();
			ctx.set_color(mouse_is_on_overlay ? ti.control() : ti.group());
			ctx.tesselate_unit_square();
			ctx.ref_default_shader_program().disable(ctx);
			glDepthMask(GL_TRUE);

			ctx.set_projection_matrix(P);
			ctx.set_modelview_matrix(M);
			MPW = ctx.get_modelview_projection_window_matrix();
			
			for (auto p : plots)
				p.second->draw(ctx);
		ctx.pop_modelview_matrix();
		ctx.pop_projection_matrix();
		ctx.pop_window_transformation_array();
	}
	bool view3d_overlay::handle_event(cgv::gui::event& e)
	{
		ivec2 os = get_overlay_size();
		ivec2 cr = get_overlay_position() + os / 2;
		if (e.get_kind() == cgv::gui::EID_KEY) {
			auto& ke = reinterpret_cast<cgv::gui::key_event&>(e);
			if (ke.get_action() != cgv::gui::KA_RELEASE) {
				switch (ke.get_key()) {
				case cgv::gui::KEY_Space:
					if (ke.get_modifiers() == cgv::gui::EM_CTRL) {
						default_view = current_view;
						on_set(&default_view);
						return true;
					}
					else if (ke.get_modifiers() == cgv::gui::EM_SHIFT) {
						static_cast<cgv::render::view&>(current_view) = default_view;
						on_set(&current_view);
						return true;
					}
					break;
				}
			}
		}
		else if (e.get_kind() == cgv::gui::EID_MOUSE) {
			auto& me = reinterpret_cast<cgv::gui::mouse_event&>(e);
			int x_gl = me.get_x();
			int y_gl = get_context()->get_height() - 1 - me.get_y();
			dvec3 x, y, z;
			current_view.put_coordinate_system(x, y, z);
			if (me.get_action() == cgv::gui::MA_LEAVE)
				last_x = -1;
			else {
				last_x = x_gl;
				last_y = y_gl;
			}

			switch (me.get_action()) {
			case cgv::gui::MA_ENTER:
				mouse_is_on_overlay = true;
				post_redraw();
				return true;
			case cgv::gui::MA_LEAVE:
				mouse_is_on_overlay = false;
				post_redraw();
				return true;
			case cgv::gui::MA_PRESS:
				if (me.get_button() == cgv::gui::MB_LEFT_BUTTON && me.get_modifiers() == 0) {
					check_for_click = me.get_time();
					return true;
				}
				if (((me.get_button() == cgv::gui::MB_LEFT_BUTTON) &&
					((me.get_modifiers() == 0) || (me.get_modifiers() == cgv::gui::EM_SHIFT))) ||
					((me.get_button() == cgv::gui::MB_RIGHT_BUTTON) && (me.get_modifiers() == 0)) ||
					((me.get_button() == cgv::gui::MB_MIDDLE_BUTTON) && (me.get_modifiers() == 0)))
					return true;
				break;
			case cgv::gui::MA_RELEASE:
				if (check_for_click != -1) {
					double dt = me.get_time() - check_for_click;
					if (dt < 0.2) {
						if (get_context()) {
							cgv::render::context& ctx = *get_context();
							dvec3 p;
							ctx.make_current();
							double z = ctx.get_window_z(x_gl, y_gl);
							p = ctx.get_model_point(x_gl, y_gl, z, MPW);
							if (z > 0 && z < 1) {
								if (current_view.get_y_view_angle() > 0.1) {
									dvec3 e = current_view.get_eye();
									double l_old = (e - current_view.get_focus()).length();
									double l_new = dot(p - e, current_view.get_view_dir());
									//std::cout << "e=(" << e << "), p=(" << p << "), vd=(" << view_ptr->get_view_dir() << ") l_old=" << l_old << ", l_new=" << l_new << std::endl;
									cgv::gui::animate_with_geometric_blend(current_view.ref_y_extent_at_focus(), current_view.get_y_extent_at_focus() * l_new / l_old, 0.5)->set_base_ptr(this);
								}
								cgv::gui::animate_with_linear_blend(current_view.ref_focus(), p, 0.5)->configure(cgv::gui::APM_SIN_SQUARED, this);
								post_redraw();
								return true;
							}
						}
					}
					check_for_click = -1;
				}
				if ((me.get_button() == cgv::gui::MB_LEFT_BUTTON && (me.get_modifiers() == 0 || me.get_modifiers() == cgv::gui::EM_SHIFT)) ||
					me.get_button() == cgv::gui::MB_RIGHT_BUTTON && me.get_modifiers() == 0)
					return true;
				break;
			case cgv::gui::MA_DRAG:
				check_for_click = -1;
				if (me.get_dx() == 0 && me.get_dy() == 0)
					break;
				if (me.get_button_state() == cgv::gui::MB_LEFT_BUTTON && me.get_modifiers() == 0) {
					current_view.rotate(
						-6.0 * me.get_dy() / os[1] / rotate_sensitivity, 
						-6.0 * me.get_dx() / os[0] / rotate_sensitivity, current_view.get_depth_of_focus());
					for (int ci = 0; ci < 3; ++ci) {
						update_member(&current_view.ref_view_up_dir()[ci]);
						update_member(&current_view.ref_view_dir()[ci]);
					}
					post_redraw();
					return true;
				}
				if (me.get_button_state() == cgv::gui::MB_LEFT_BUTTON && me.get_modifiers() == cgv::gui::EM_SHIFT) {
					int rx = x_gl - cr[0];
					int ry = y_gl - cr[1];
					double ds = sqrt(((double)me.get_dx() * (double)me.get_dx() + (double)me.get_dy() * (double)me.get_dy()) /
						((double)rx * (double)rx + (double)ry * (double)ry));
					if (rx * me.get_dy() > ry * me.get_dx())
						ds = -ds;
					current_view.roll(ds / rotate_sensitivity);
					for (int ci = 0; ci < 3; ++ci)
						update_member(&current_view.ref_view_up_dir()[ci]);
					post_redraw();
					return true;
				}
				if (me.get_button_state() == cgv::gui::MB_RIGHT_BUTTON && me.get_modifiers() == 0) {
					current_view.set_focus(current_view.get_focus() - (current_view.get_y_extent_at_focus() * me.get_dx() / os[0]) * x
						+ (current_view.get_y_extent_at_focus() * me.get_dy() / os[1]) * y);
					for (int ci = 0; ci < 3; ++ci)
						update_member(&current_view.ref_focus()[ci]);
					post_redraw();
					return true;
				}
				if (me.get_button_state() == cgv::gui::MB_MIDDLE_BUTTON && me.get_modifiers() == 0) {
					current_view.set_focus(current_view.get_focus() -
						5 * current_view.get_y_extent_at_focus() * me.get_dy() / os[1] * z / zoom_sensitivity);
					for (int ci = 0; ci < 3; ++ci)
						update_member(&current_view.ref_focus()[ci]);
					post_redraw();
					return true;
				}
				break;
			case cgv::gui::MA_WHEEL:
				if (e.get_modifiers() == cgv::gui::EM_SHIFT) {
					current_view.set_y_view_angle(current_view.get_y_view_angle() + me.get_dy() * 5);
					if (current_view.get_y_view_angle() < 0)
						current_view.set_y_view_angle(0);
					if (current_view.get_y_view_angle() > 180)
						current_view.set_y_view_angle(180);
					update_member(&current_view.ref_y_view_angle());
					post_redraw();
					return true;
				}
				else if (e.get_modifiers() == 0) {
					double scale = exp(0.2 * me.get_dy() / zoom_sensitivity);
					if (get_context()) {
						cgv::render::context& ctx = *get_context();
						dvec3 p;
						ctx.make_current();
						double z = ctx.get_window_z(x_gl, y_gl);
						p = ctx.get_model_point(x_gl, y_gl, z, MPW);
						if (z > 0 && z < 1) {
							current_view.set_focus(p + scale * (current_view.get_focus() - p));
							for (int ci=0;ci<3;++ci)
								update_member(&current_view.ref_focus()[ci]);
						}
					}
					current_view.set_y_extent_at_focus(current_view.get_y_extent_at_focus() * scale);
					update_member(&current_view.ref_y_extent_at_focus());
					post_redraw();
					return true;
				}
				break;
			default: break;
			}
		}
		return false;
	}
	void view3d_overlay::create_gui()
	{
		add_decorator(std::string("View3D ")+get_name(), "heading", "level=1");
		add_member_control(this, "Zoom Sensitivity", zoom_sensitivity, "value_slider", "min=0.1;max=10;ticks=true;step=0.01;log=true");
		add_member_control(this, "Rotate Sensitivity", rotate_sensitivity, "value_slider", "min=0.1;max=10;ticks=true;step=0.01;log=true");
		add_gui("Focus", current_view.ref_focus(), "", "options='min=-10;max=10;ticks=true'");
		add_gui("View Dir", current_view.ref_view_dir(), "direction", "options='min=-1;max=1;ticks=true'");
		add_gui("View Up Dir", current_view.ref_view_up_dir(), "direction", "options='min=-1;max=1;ticks=true'");

		add_member_control(this, "y View Angle", current_view.ref_y_view_angle(), "value_slider", "min=0;max=90;ticks=true;log=true");
		add_member_control(this, "y Extent at Focus", current_view.ref_y_extent_at_focus(), "value_slider", "min=0;max=100;ticks=true;log=true;step=0.0001");
		add_member_control(this, "z Near", current_view.ref_z_near(), "value_slider", "min=0;max=100;log=true;step=0.00001");
		add_member_control(this, "z Far", current_view.ref_z_far(), "value_slider", "min=0;max=10000;log=true;step=0.00001");
	}
}