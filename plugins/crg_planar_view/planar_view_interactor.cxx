#include "planar_view_interactor.h"
#include <cgv/utils/ostream_printf.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cgv_gl/gl/gl.h>

using namespace cgv::math;
using namespace cgv::gui;
using namespace cgv::render;



planar_view_interactor::planar_view_interactor(const char* name) : node(name)
{
	set_default_values();
}

void planar_view_interactor::set_default_values()
{
	
	target.zeros(2);
	magnification=0.25;
	vp_x = 0;
	vp_y = 0;
	pos_down.resize(2);
	pressed=false;
}

const cgv::math::mat<float> planar_view_interactor::get_projection() const
{
	return cgv::math::ortho2d_44<float>(-aspect,aspect,-1,1);
}

const cgv::math::mat<float> planar_view_interactor::get_modelview() const
{
	
	return cgv::math::scale_44<float>(magnification,magnification,magnification)*
		cgv::math::translate_44<float>(-target(0),-target(1),0.0f);
	
}


		
void planar_view_interactor::move(int x, int y)
{

	vec<float> p = get_context()->get_point_W(x, y, 0.0, DPV).sub_vec(0,2); 

	target += (pos_down-p) ;
	post_redraw();

}

void planar_view_interactor::zoom(int x,int y, float ds)
{
	vec<float> p = get_context()->get_point_W(x, y, 0.0, DPV).sub_vec(0,2);
	
	float s1=magnification;
	if( ds > 0)
		magnification*=0.9f;
	else
		magnification*=1.1f;
	float s2 = magnification;

	target -= (s1-s2)*(p-target)/s2;


		
	post_redraw();
	
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
		
			
		case MA_PRESS:
			if (me.get_button_state() == MB_RIGHT_BUTTON && me.get_modifiers() == 0)
			{
				pressed=true;
				pos_down = get_context()->get_point_W(me.get_x(),me.get_y(), 0.0, DPV).sub_vec(0,2);
				return true;
			}
			
			break;

		
		case MA_DRAG:
		
			
			if (me.get_button_state() == MB_RIGHT_BUTTON && me.get_modifiers() == 0 && (me.get_dx() != 0 || me.get_dy() != 0))
			{
				if(pressed == true)
					move(me.get_x(),me.get_y());
				post_redraw();
			//	return true;
			}
			break;
		case MA_RELEASE:
			pressed=false;
			break;

		case MA_WHEEL :
			if (e.get_modifiers() == 0) 
			{
			

				zoom(me.get_x(),me.get_y(),me.get_dy());
				
			//	return true;
			}
			
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

/// this method is called in one pass over all drawables before the draw method
void planar_view_interactor::draw(context& ctx)
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
	DPV = ctx.get_DPV();
	
	
}

