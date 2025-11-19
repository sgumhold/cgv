#include "planar_view_interactor.h"
#include <cgv/utils/ostream_printf.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <cgv/math/ftransform.h>
#include <cgv_reflect_types/math/fvec.h>
#include <cgv_gl/gl/gl.h>

using namespace cgv::math;
using namespace cgv::gui;
using namespace cgv::render;

planar_view_interactor::planar_view_interactor(const std::string& name) : node(name)
{
}

void planar_view_interactor::resize(unsigned w, unsigned h)
{
	aspect = static_cast<float>(w) / h;
}

void planar_view_interactor::reset_view()
{
	target = { 0.0f };
	magnification = 1.0f;
	angle = 0.0f;

	on_set(&magnification);
	on_set(&target(0));
	on_set(&target(1));
	on_set(&angle);
}

/// describe members
bool planar_view_interactor::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return
		rh.reflect_member("lock_rotation", lock_rotation) &&
		rh.reflect_member("angle", angle) &&
		rh.reflect_member("magnification", magnification) &&
		rh.reflect_member("center", target);
}

/// default callback
void planar_view_interactor::on_set(void* member_ptr)
{
	if (member_ptr == &lock_rotation) {
		if (find_control(angle))
			find_control(angle)->set("active", !lock_rotation);
	}
	update_member(member_ptr);
	post_redraw();
}

/// create a gui
void planar_view_interactor::create_gui()
{
	add_decorator("Planar View Configuration", "heading", "level=2");
	add_member_control(this, "lock_rotation", lock_rotation, "toggle");
	add_member_control(this, "angle", angle, "value_slider", "min=-180;max=180;ticks=true");
	add_member_control(this, "zoom", magnification, "value_slider", "min=0.01;max=100.0;step=0.0001;log=true;ticks=true");
	add_member_control(this, "center_x", target(0), "value");
	add_member_control(this, "center_y", target(1), "value");
	find_control(angle)->set("active", !lock_rotation);
}

const cgv::dmat4 planar_view_interactor::get_projection() const
{
	return cgv::math::ortho4<double>(-aspect,aspect, -1.0f, 1.0f, 0.1f, 10.0f);
}

const cgv::dmat4 planar_view_interactor::get_modelview() const
{
	return 
		cgv::math::scale4<double>(magnification)*
		cgv::math::rotate4<double>(angle, cgv::dvec3(0.0, 0.0, 1.0))*
		cgv::math::translate4<double>(-target(0), -target(1), 0.0f);
}

	
void planar_view_interactor::move(int x, int y)
{
	auto p = get_context()->get_model_point(x, y, 0.0, MPW);
	target += pos_down - cgv::vec2(p);

	on_set(&target(0));
	on_set(&target(1));
}

void planar_view_interactor::rotate(int x, int y)
{
	auto p = get_context()->get_model_point(x, y, 0.0, MPW);
	cgv::vec2 dp = cgv::vec2(p) - target;
	float ap = static_cast<float>(180.0f * atan2(dp(1), dp(0)) / M_PI);
	cgv::vec2 dd = pos_down - target;
	float ad = static_cast<float>(180.0f * atan2(dd(1), dd(0)) / M_PI);
	angle += ap - ad;

	on_set(&angle);
}

void planar_view_interactor::zoom(int x, int y, float ds)
{
	auto p = get_context()->get_model_point(x, y, 0.0, MPW);
	
	float s1 = magnification;

	if(ds > 0.0f)
		magnification *= 0.9f;
	else
		magnification *= 1.1f;

	float s2 = magnification;

	target -= (s1-s2) * (cgv::vec2(p) - target) / s2;

	on_set(&magnification);
	on_set(&target(0));
	on_set(&target(1));
}

void planar_view_interactor::set_center(const cgv::vec2& center)
{
	target = center;
	on_set(&target(0));
	on_set(&target(1));
}

void planar_view_interactor::set_magnification(float magnification)
{
	this->magnification = magnification;
	on_set(&magnification);
}

void planar_view_interactor::set_angle(float angle)
{
	this->angle = angle;
	on_set(&angle);
}

void planar_view_interactor::keep_rotation_locked(bool lock)
{
	lock_rotation = lock;
	on_set(&lock_rotation);
}

/// return the type name 
std::string planar_view_interactor::get_type_name() const
{
	return "planar_view_interactor";
}

/// overload to show the content of this object
void planar_view_interactor::stream_stats(std::ostream& os)
{
	//cgv::media::text::oprintf(os, "%s: azimut = %.3f, elevation = %.3f\n", get_name(), azimut,elevation);
}

bool planar_view_interactor::init(context& ctx)
{
	return true;
}

/// overload and implement this method to handle events
bool planar_view_interactor::handle(event& e)
{
	if (e.get_kind() == EID_KEY) {
		key_event ke = (key_event&) e;
		if (ke.get_action() == KA_PRESS) {
			switch (ke.get_key()) {
			case KEY_Space :
				reset_view();
				post_redraw();
				return true;
			}
		}
	}
	
	if (e.get_kind() == EID_MOUSE) 
	{
		cgv::gui::mouse_event me = (cgv::gui::mouse_event&) e;
		int y_gl = get_context()->get_height() - 1 - me.get_y();
		switch (me.get_action()) {
		case MA_PRESS:
			if ( (me.get_button_state() == MB_RIGHT_BUTTON && me.get_modifiers() == 0) ||
				 (me.get_button_state() == MB_LEFT_BUTTON && me.get_modifiers() == 0) ||
				 (me.get_button_state() == MB_MIDDLE_BUTTON && me.get_modifiers() == 0) )
					{
				pressed = true;
				cgv::vec3 p = get_context()->get_model_point(me.get_x(), y_gl, 0.0, MPW);
				pos_down = cgv::vec2(p);
				return true;
			}
			break;
		case MA_DRAG:
			if (me.get_button_state() == MB_RIGHT_BUTTON && me.get_modifiers() == 0 && (me.get_dx() != 0 || me.get_dy() != 0))
			{
				if(pressed)
					move(me.get_x(), y_gl);
				post_redraw();
				return true;
			}
			else if (me.get_button_state() == MB_LEFT_BUTTON && me.get_modifiers() == 0 && (me.get_dx() != 0 || me.get_dy() != 0) && !lock_rotation)
			{
				if (pressed)
					rotate(me.get_x(), y_gl);
				post_redraw();
				return true;
			}
			else if (me.get_button_state() == MB_MIDDLE_BUTTON && me.get_modifiers() == 0 && me.get_dy() != 0)
			{
				if (pressed)
					zoom(me.get_x(), y_gl, me.get_dy());
				post_redraw();
				return true;
			}
			break;
		case MA_RELEASE:
			pressed = false;
			break;
		case MA_WHEEL :
			if (e.get_modifiers() == 0) 
			{
				zoom(me.get_x(), y_gl,me.get_dy());
				//return true;
			}
			break;
		default:
			break;
		}
	}
	return false;
}

/// overload to stream help information to the given output stream
void planar_view_interactor::stream_help(std::ostream& os)
{
	os << "planar_view_interactor\n\a"
	   << "move target parallel to image plane: right mouse button\n"
	   << "scale: mouse wheel\n"
	   << "reset view: <Space> \n";
}

///
void planar_view_interactor::init_frame(cgv::render::context& ctx)
{
	ctx.push_modelview_matrix();
	ctx.set_modelview_matrix(get_modelview());
	ctx.push_projection_matrix();
	ctx.set_projection_matrix(get_projection());
	MPW = ctx.get_modelview_projection_window_matrix();
}

/// 
void planar_view_interactor::after_finish(cgv::render::context& ctx)
{
	ctx.pop_modelview_matrix();
	ctx.pop_projection_matrix();
}
