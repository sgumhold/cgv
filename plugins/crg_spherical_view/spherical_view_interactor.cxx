#include "spherical_view_interactor.h"
#include <cgv/media/text/ostream_printf.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cgv/render/gl/gl.h>

using namespace cgv::math;
using namespace cgv::gui;
using namespace cgv::media::text;
using namespace cgv::render;






spherical_view_interactor::spherical_view_interactor(const char* name) : node(name)
{
	set_default_values();
}

void spherical_view_interactor::set_default_values()
{
	
	target.zeros(3);
	elevation=45.0f;
	azimut=-45.0f;
	znear=0.1f;
	zfar=10000.0f;
	fovy=45.0f;
	distance=20.0f;
	aspect=1.33f;
	vp_x = 0;
	vp_y = 0;
}

const cgv::math::mat<float> spherical_view_interactor::get_projection() const
{
	return cgv::math::perspective_44(fovy,aspect, znear,zfar);
}

const cgv::math::mat<float> spherical_view_interactor::get_modelview() const
{
	//student begin
	return	cgv::math::translate_44(0.0f,0.0f,-distance)*
			cgv::math::rotatex_44(elevation)*
			cgv::math::rotatey_44(azimut)*
			cgv::math::translate_44(-target);
	//student end
}

const cgv::math::vec<float> spherical_view_interactor::get_up_dir() const
{
	//student begin
	cgv::math::vec<float> v(0.0,1.0,0.0,0.0);
	v=cgv::math::inv(get_modelview())*v;
	return cgv::math::vec<float>(3,v);
	//student end
}


const cgv::math::vec<float> spherical_view_interactor::get_right_dir()const 
{
	cgv::math::vec<float> v(1.0,0.0,0.0,0.0);
	v=cgv::math::inv(get_modelview())*v;
	return cgv::math::vec<float>(3,v);
}

void spherical_view_interactor::rotate_azimut(float delta)
{
	azimut += delta; 
}

void spherical_view_interactor::rotate_elevation(float delta)
{
	elevation += delta; 
}
		
void spherical_view_interactor::move(float dx, float dy)
{
	target += dy*get_up_dir()-dx*get_right_dir();
}


/// return the type name 
std::string spherical_view_interactor::get_type_name() const
{
	return "spherical_view_interactor";
}

/// overload to show the content of this object
void spherical_view_interactor::stream_stats(std::ostream& os)
{
	cgv::media::text::oprintf(os, "%s: target = (%.3f, %.3f, %.3f )\n", get_name().c_str(), target(0),target(1),target(2));
	cgv::media::text::oprintf(os, "%s: azimut = %.3f, elevation = %.3f, distance = %.3f\n", get_name().c_str(), azimut,elevation,distance);
}

bool spherical_view_interactor::init(context& ctx)
{
	return true;
}

/// overload and implement this method to handle events
bool spherical_view_interactor::handle(event& e)
{
	if (e.get_kind() == EID_KEY) {
		key_event ke = (key_event&) e;
		if (ke.get_action() == KA_PRESS) {
			switch (ke.get_key()) {
			case KEY_Space :
				set_default_values();
				post_redraw();
				return true;
			
			}

		}
	}
	
	if (e.get_kind() == EID_MOUSE) 
	{
		cgv::gui::mouse_event me = (cgv::gui::mouse_event&) e;
		switch (me.get_action()) {
		
			
			
		
		case MA_MOVE:
		
			if (me.get_button_state() == MB_LEFT_BUTTON && me.get_modifiers() == 0)
			{
				rotate_azimut((float)me.get_dx());
				rotate_elevation((float)me.get_dy());
				post_redraw();
				return true;
			}
			if (me.get_button_state() == MB_MIDDLE_BUTTON && me.get_modifiers() == 0)
			{
				move(distance/700.0f*me.get_dx(),distance/700.0f*me.get_dy());
				post_redraw();
				return true;
			}


			if (me.get_button_state() == MB_RIGHT_BUTTON && me.get_modifiers() == 0) 
			{
				distance=distance*(1.0f+0.01f*me.get_dy());	
				if(distance <= znear)
					distance=znear;
				post_redraw();
				return true;
			}
			break;
		case MA_WHEEL :
			if (e.get_modifiers() == 0) {
				if( me.get_dy() > 0)
			{
				distance=distance*1.1f;
				if(distance < znear)
					distance=znear;	
			}
			else
				distance=distance*0.9f;	
				post_redraw();
				return true;
			}
		}
	}
	return false;
}




/// overload to stream help information to the given output stream
void spherical_view_interactor::stream_help(std::ostream& os)
{
	os << "spherical_view_interactor\n\a"
	   << "move target parallel to image plane: middle mouse button\n"
	   << "rotate azimut /elevation:            left mouse button\n"
	   << "eye - target distance:               mouse wheel or right mouse button\n"
	   << "reset view:                          <Space> \n";
}

/// this method is called in one pass over all drawables before the draw method
void spherical_view_interactor::draw(context& ctx)
{
	vp_height = ctx.get_height();
	vp_width = ctx.get_width();
	if(vp_height != 0)
		aspect = (float)vp_width/(float)vp_height;
	
	glViewport(vp_x,vp_y,vp_width,vp_height);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(get_projection());
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(get_modelview());
}


