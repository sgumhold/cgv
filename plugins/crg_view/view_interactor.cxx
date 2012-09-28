#include "view_interactor.h"
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

template <typename T>
fvec<T,3> rotate(const fvec<T,3>& v, const fvec<T,3>& n, T a) 
{
	fvec<T,3> vn = dot(n,v)*n;
	return vn + cos(a)*(v-vn) + sin(a)*cross(n,v);
}

ext_view::ext_view()
{
	set_default_values();
}

void ext_view::set_default_values()
{
	y_view_angle = 45;
	y_extent_at_focus = 2;
	set_view_dir(0,0,-1);
	set_focus(0,0,0);
	set_view_up_dir(0,1,0);
	z_near = 0.01;
	z_far  = 10000.0;
}

void ext_view::put_coordinate_system(vec_type& x, vec_type& y, vec_type& z) const
{
	z = -view_dir;
	z.normalize();
	x = cross(view_up_dir,z);
	x.normalize();
	y = cross(z,x);
}


///
view_interactor::view_interactor(const char* name) : node(name)
{
	set_default_values();
	check_for_click = -1;
}
/// return the type name 
std::string view_interactor::get_type_name() const
{
	return "view_interactor";
}
/// overload to show the content of this object
void view_interactor::stream_stats(std::ostream& os)
{
	pnt_type e = get_eye();
	oprintf(os,"View: y-view angle=%.1fº, y-extent=%.1f, z:[%.2f,%.2f], foc=%.2f,%.2f,%.2f, dir=%.2f,%.2f,%.2f, up=%.2f,%.2f,%.2f\n", 
		y_view_angle,y_extent_at_focus,z_near, z_far,
		view::focus(0),view::focus(1),view::focus(2),
		view_dir(0),view_dir(1),view_dir(2),
		view_up_dir(0),view_up_dir(1),view_up_dir(2));
}
bool view_interactor::init(context& ctx)
{
	return drawable::init(ctx);
}

/// overload and implement this method to handle events
bool view_interactor::handle(event& e)
{
	if (e.get_kind() == EID_KEY) {
		key_event ke = (key_event&) e;
		if (ke.get_action() == KA_PRESS) {
			switch (ke.get_key()) {
			case KEY_Space :
				set_default_values();
				post_redraw();
				return true;
			case 'F' :
				if (ke.get_modifiers() == EM_SHIFT)
					z_far /= 1.05;
				else
					z_far *= 1.05;
				post_redraw();
				return true;
			case 'N' :
				if (ke.get_modifiers() == EM_SHIFT)
					z_near /= 1.05;
				else
					z_near *= 1.05;
				post_redraw();
				return true;
			case KEY_Page_Up:
				y_extent_at_focus /= 1.2;
				post_redraw();
				return true;
			case KEY_Page_Down:
				y_extent_at_focus *= 1.2;
				post_redraw();
				return true;
			}

		}
	}
	else if (e.get_kind() == EID_MOUSE) {
		vec_type x,y,z;
		put_coordinate_system(x,y,z);
		int width = 0, height = 0;
		double aspect = 1;
		if (get_context()) {
			width = get_context()->get_width();
			height = get_context()->get_height();
			aspect = (double)width/height;
		}
		cgv::gui::mouse_event me = (cgv::gui::mouse_event&) e;
		switch (me.get_action()) {
		case MA_PRESS :
			if (me.get_button() == MB_LEFT_BUTTON && me.get_modifiers() == 0) {
				check_for_click = me.get_time();
				return true;
			}
			if (((me.get_button() == MB_LEFT_BUTTON) && 
				    ((me.get_modifiers() == 0) || (me.get_modifiers() == EM_SHIFT))) ||
				 ((me.get_button() == MB_RIGHT_BUTTON) && (me.get_modifiers() == 0))) 
				return true;
			break;
		case MA_RELEASE :
			if (check_for_click != -1) {
				double dt = me.get_time() - check_for_click;
				if (dt < 0.2) {
					if (get_context()) {
						double z = get_context()->get_z_D(me.get_x(), me.get_y());
						if (z > 0 && z < 1) {
							pnt_type p = get_context()->get_point_W(me.get_x(), me.get_y(), z, DPV);
							if (y_view_angle > 1) {
								pnt_type e = get_eye();
								double l_old = (e-view::focus).length();
								double l_new = dot(p-e,view_dir);
								y_extent_at_focus *= l_new/l_old;
							}
							view::focus = p;
							post_redraw();
							return true;
						}
					}
				}
				check_for_click = -1;
			}
			if ((me.get_button() == MB_LEFT_BUTTON && (me.get_modifiers() == 0 || me.get_modifiers() == EM_SHIFT)) ||
				me.get_button() == MB_RIGHT_BUTTON && me.get_modifiers() == 0) 
				return true;
			break;
		case MA_DRAG :
			check_for_click = -1;
			if (me.get_button_state() == MB_LEFT_BUTTON && me.get_modifiers() == 0) {
				rotate_image_plane(360.0*me.get_dx()/width,-360.0*me.get_dy()/height);
				post_redraw();
				return true;
			}
			if (me.get_button_state() == MB_LEFT_BUTTON && me.get_modifiers() == EM_SHIFT) {
				int rx = me.get_x() - width/2;
				int ry = me.get_y() - height/2;
				double ds = sqrt(((double)me.get_dx()*(double)me.get_dx()+(double)me.get_dy()*(double)me.get_dy())/
								 ((double)rx*(double)rx+(double)ry*(double)ry));
				if (rx*me.get_dy() > ry*me.get_dx())
					ds = -ds;
				roll(56.3*ds);
				post_redraw();
				return true;
			}
			if (me.get_button_state() == MB_RIGHT_BUTTON && me.get_modifiers() == 0) {
				view::focus = view::focus - 2*(y_extent_at_focus*me.get_dx()/width)*x
					          + 2*(y_extent_at_focus*me.get_dy()/height)*y;
				post_redraw();
				return true;
			}
			break;
		case MA_WHEEL :
			if (e.get_modifiers() == EM_SHIFT) {
				y_view_angle += me.get_dy()*5;
				if (y_view_angle < 0)
					y_view_angle = 0;
				if (y_view_angle > 180)
					y_view_angle = 180;
				post_redraw();
				return true;
			}
			else if (e.get_modifiers() == 0) {
				double scale = exp(0.2*me.get_dy());
				if (get_context()) {
					double z = get_context()->get_z_D(me.get_x(), me.get_y());
					if (z > 0 && z < 1) {
						pnt_type p = get_context()->get_point_W(me.get_x(), me.get_y(), z, DPV);
						view::focus = p + scale*(view::focus-p);
					}
				}
				y_extent_at_focus *= scale;
				post_redraw();
				return true;
			}
			break;
		default: break;
		}
	}
	return false;
}

///
void view_interactor::roll(double angle)
{
	view_up_dir = rotate(view_up_dir, view_dir, angle*.1745329252e-1);
}
///
void view_interactor::rotate_image_plane(double ax, double ay)
{
	vec_type x,y,z;
	put_coordinate_system(x,y,z);
	z = ay*x-ax*y;
	double a = z.length();
	z = (1/a) * z;
	a *= .1745329252e-1;
	view_dir = rotate(view_dir, z, a);
	view_up_dir = rotate(view_up_dir, z, a);
}

/// overload to stream help information to the given output stream
void view_interactor::stream_help(std::ostream& os)
{
	os << "view_interactor\n\a"
	   << "set focus:                    left mouse button click\n"
	   << "rotate in image plane:        left mouse button\n"
	   << "rotate around view direction: Shift+left mouse button\n"
	   << "dolly zoom / zoom to point:   [Shift+]mouse wheel / PgUp,PgDn\n"
	   << "move parallel to image plane: right mouse button\n"
	   << "decrease/increase z_near/far: [Shift+]N/F\b\n";
}

/// this method is called in one pass over all drawables before the draw method
void view_interactor::init_frame(context& ctx)
{
	int width = ctx.get_width();
	int height = ctx.get_height();
	double aspect = (double)width/height;

	// this command makes all successive matrix manipultations act on the projection transformation stack
	glMatrixMode(GL_PROJECTION);

	// initialize the projective transformation to the identity
	glLoadIdentity();

	if (y_view_angle <= 0.1)
		glOrtho(-aspect*y_extent_at_focus, aspect*y_extent_at_focus, 
				-y_extent_at_focus, y_extent_at_focus, z_near, z_far);
	else {
		double top    = z_near*tan(0.008726646262*y_view_angle);
		double bottom = -top;
		//double delta  = stereo_frame*top*aspect*eye_distance;
		double left   = bottom*aspect;//-delta;
		double right  = top*aspect;//-delta;
		glFrustum(left,right,bottom,top,z_near,z_far);
	}
	// switch back to the modelview transformation stack
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// set up the observer at location (0,-3,4) and look at the focus point (0,1,-1) with
	// the upward direction of the image along the z-axis
	//v3d x, y, z;
	//put_coordinate_system(x,y,z);
	//double s = scale*aspect*stereo_frame*eye_distance;
	pnt_type eye;
	if (y_view_angle <= 0.1)
		eye = view::focus-0.3*z_far*view_dir;
	else
		eye = get_eye(); // + s*x;
	pnt_type foc = view::focus; // + s*x;
	gluLookAt(eye(0),eye(1),eye(2),foc(0),foc(1),foc(2),view_up_dir(0),view_up_dir(1),view_up_dir(2));

	DPV = ctx.get_DPV();
}

#include <cgv/base/register.h>

/// register a newly created cube with the name "cube1" as constructor argument
extern cgv::base::object_registration_1<view_interactor,const char*> obj1("trackball", "");
