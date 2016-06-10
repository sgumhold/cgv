#include "stereo_view_interactor.h"
#include <cgv/utils/scan.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/type/variant.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <stdio.h>

using namespace cgv::math;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::utils;
using namespace cgv::render;
using namespace cgv::render::gl;

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
	set_default_view();
	set_y_view_angle(45);
	set_z_near(0.01);
	set_z_far(10000.0);
	set_eye_distance(0.03);
	set_parallax_zero_scale(0.5);
	enable_stereo(false);
	set_stereo_mode(GLSU_ANAGLYPH);
	set_anaglyph_config(GLSU_RED_CYAN);
	two_d_enabled=false;
}

void ext_view::put_coordinate_system(vec_type& x, vec_type& y, vec_type& z) const
{
	z = -view_dir;
	z.normalize();
	x = cross(view_up_dir,z);
	x.normalize();
	y = cross(z,x);
}


double ext_view::get_parallax_zero_z() const
{
	pnt_type foc = view::focus;
	pnt_type eye = (y_view_angle <= 0.1) ? foc-0.3*z_far*view_dir : get_eye();
	pnt_type dv  = (1/parallax_zero_scale-2)*(eye - foc);
	foc += dv;
	eye += dv;
	return dot(foc-eye,view_dir);
}

///
stereo_view_interactor::stereo_view_interactor(const char* name) : node(name)
{
	write_depth = false;
	write_color = true;
	write_stereo = true;
	write_width = 1400;
	write_height = 1050;

	fix_view_up_dir = false;
	write_images = false;
	stereo_translate_in_model_view = false;
	depth_offset = 0.99f;
	depth_scale = 100;
	auto_view_images = true;
	image_file_name_prefix = "frame_buffer";
	set_default_values();
	z_near_derived = z_near;
	z_far_derived = z_far;
	clip_relative_to_extent = true;
	show_focus = false;
	check_for_click = -1;
	mono_mode = GLSU_CENTER;
	zoom_sensitivity = rotate_sensitivity = 1;
}
/// return the type name 
std::string stereo_view_interactor::get_type_name() const
{
	return "stereo_view_interactor";
}
/// overload to show the content of this object
void stereo_view_interactor::stream_stats(std::ostream& os)
{
	static const char* stereo_strings[] = {
		"vsplit", "hsplit", "anaglyph", "quad_buffer"
	};
	static const char* mono_strings[] = {
		"left", "center", "right"
	};
	static const char* enabled_strings[] = {
		"off", "on"
	};
	pnt_type e = get_eye();
	oprintf(os,"View: y-view angle=%.1fº, y-extent=%.1f, z:[%.2f|%.2f,%.2f|%.2f], foc=%.2f,%.2f,%.2f, dir=%.2f,%.2f,%.2f, up=%.2f,%.2f,%.2f\n", 
		y_view_angle,y_extent_at_focus,z_near_derived, z_near, z_far_derived, z_far,
		view::focus(0),view::focus(1),view::focus(2),
		view_dir(0),view_dir(1),view_dir(2),
		view_up_dir(0),view_up_dir(1),view_up_dir(2));
	oprintf(os,"      mono:%s, stereo:%s, mode=%s, eye-dist=%.3f\n", 
		stereo_strings[mono_mode+1],
		enabled_strings[is_stereo_enabled()?1:0],
		stereo_strings[stereo_mode], eye_distance);
}
bool stereo_view_interactor::init(context& ctx)
{
	return drawable::init(ctx);
}

double stereo_view_interactor::get_z_and_unproject(cgv::render::context* ctx, int x, int y, pnt_type& p)
{
	double z; 
	if (stereo_enabled) {
		ctx->make_current();
		double aspect = (double)ctx->get_width() / ctx->get_height();
		if (stereo_mode == GLSU_SPLIT_VERTICALLY)
			aspect *= 0.5;
		if (stereo_mode == GLSU_SPLIT_HORIZONTALLY)
			aspect *= 2;
		double y_extent = y_extent_at_focus;
		GlsuEye e = GLSU_CENTER;
		
		// compute eye and focus point
		pnt_type foc = view::focus;
		pnt_type eye = get_eye();
		pnt_type dv = (1 / parallax_zero_scale - 2)*(eye - foc);
		foc += dv;
		eye += dv;
		double z_eye = dot(eye, view_dir);
		double z_focus = dot(foc, view_dir) - z_eye;

		switch (stereo_mode) {
		case GLSU_SPLIT_HORIZONTALLY:
			e = y >= (int)ctx->get_height() / 2 ? GLSU_RIGHT : GLSU_LEFT;
			break;
		case GLSU_SPLIT_VERTICALLY:
			e = x >= (int)ctx->get_width() / 2 ? GLSU_RIGHT : GLSU_LEFT;
			break;
		}
		if (e == GLSU_RIGHT)
			glsuConfigureStereo(GLSU_LEFT, stereo_mode, ac);
		glsuConfigureStereo(e, stereo_mode, ac);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		if (stereo_translate_in_model_view)
			glsuStereoFrustumScreen(e, eye_distance, 2 * y_extent*aspect, 2 * y_extent, z_focus, z_near_derived, z_far_derived);
		else
			glsuStereoPerspectiveScreen(e, eye_distance, 2 * y_extent*aspect, 2 * y_extent, z_focus, z_near_derived, z_far_derived);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		if (stereo_translate_in_model_view)
			glsuStereoTranslateScreen(e, eye_distance, y_extent*aspect);
		gluLookAt(eye(0), eye(1), eye(2), foc(0), foc(1), foc(2), view_up_dir(0), view_up_dir(1), view_up_dir(2));


		z = ctx->get_z_D(x, y);
		DPV = ctx->get_DPV();
		p = (const double*)get_context()->get_point_W(x, y, z, DPV);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		if (e == GLSU_LEFT)
			glsuConfigureStereo(GLSU_RIGHT, stereo_mode, ac);
		glsuConfigureStereo(GLSU_CENTER, stereo_mode, ac);
	}
	else {
		z = ctx->get_z_D(x, y);
		p = (const double*)get_context()->get_point_W(x, y, z, DPV);
	}
	return z;
}

/// overload and implement this method to handle events
bool stereo_view_interactor::handle(event& e)
{
	if (e.get_kind() == EID_KEY) {
		key_event ke = (key_event&) e;
		if (ke.get_action() == KA_PRESS) {
			switch (ke.get_key()) {
			case KEY_Space :
				set_default_view();
				post_redraw();
				return true;
			case 'F' :
				if (ke.get_modifiers() == EM_SHIFT)
					z_far /= 1.05;
				else
					z_far *= 1.05;
				update_member(&z_far);
				post_redraw();
				return true;
			case 'N' :
				if (ke.get_modifiers() == EM_SHIFT)
					z_near /= 1.05;
				else
					z_near *= 1.05;
				update_member(&z_near);
				post_redraw();
				return true;
			case KEY_F4:
				if (ke.get_modifiers() == 0) {
					stereo_enabled = !stereo_enabled;
					update_member(&stereo_enabled);
					on_stereo_change();
					return true;
				}
				else if (ke.get_modifiers() == EM_SHIFT) {
					stereo_mode = GlsuStereoMode(stereo_mode+1);
					if (stereo_mode == GLSU_STEREO_MODE_END)
						stereo_mode = GLSU_STEREO_MODE_BEGIN;
					update_member(&stereo_mode);
					on_stereo_change();
					return true;
				}
				break;
			case KEY_Page_Up:
				y_extent_at_focus /= pow(1.2,1/zoom_sensitivity);
				update_member(&y_extent_at_focus);
				post_redraw();
				return true;
			case KEY_Page_Down:
				y_extent_at_focus *= pow(1.2,1/zoom_sensitivity);
				update_member(&y_extent_at_focus);
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
				 ((me.get_button() == MB_RIGHT_BUTTON) && (me.get_modifiers() == 0)) ||
				 ((me.get_button() == MB_MIDDLE_BUTTON) && (me.get_modifiers() == 0))) 
					return true;
			break;
		case MA_RELEASE :
			if (check_for_click != -1) {
				double dt = me.get_time() - check_for_click;
				if (dt < 0.2) {
					cgv::render::context* ctx = get_context();
					if (ctx) {
						pnt_type p;
						double z = get_z_and_unproject(ctx, me.get_x(), me.get_y(), p);
						if (z > 0 && z < 1) {
							if (y_view_angle > 1) {
								pnt_type e = get_eye();
								double l_old = (e-view::focus).length();
								double l_new = dot(p-e,view_dir);
								y_extent_at_focus *= l_new/l_old;
							}
							view::focus = p;
							update_vec_member(view::focus);
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
			if (me.get_dx() == 0 && me.get_dy() == 0)
				break;
			if (me.get_button_state() == MB_LEFT_BUTTON && me.get_modifiers() == 0) {
				if(!two_d_enabled)
				{
					rotate_image_plane(360.0*me.get_dx()/width/rotate_sensitivity,-360.0*me.get_dy()/height/rotate_sensitivity);
					update_vec_member(view_up_dir);
					update_vec_member(view_dir);
					post_redraw();
					return true;
				}
			}
			if (me.get_button_state() == MB_LEFT_BUTTON && me.get_modifiers() == EM_SHIFT) {
				int rx = me.get_x() - width/2;
				int ry = me.get_y() - height/2;
				double ds = sqrt(((double)me.get_dx()*(double)me.get_dx()+(double)me.get_dy()*(double)me.get_dy())/
								 ((double)rx*(double)rx+(double)ry*(double)ry));
				if (rx*me.get_dy() > ry*me.get_dx())
					ds = -ds;
				roll(56.3*ds/rotate_sensitivity);
				update_vec_member(view_up_dir);
				post_redraw();
				return true;
			}
			if (me.get_button_state() == MB_RIGHT_BUTTON && me.get_modifiers() == 0) {
				view::focus = view::focus - 2*(y_extent_at_focus*me.get_dx()/width)*x
					          + 2*(y_extent_at_focus*me.get_dy()/height)*y;
				update_vec_member(view::focus);
				post_redraw();
				return true;
			}
			if (me.get_button_state() == MB_MIDDLE_BUTTON && me.get_modifiers() == 0) {
				view::focus = view::focus - 
								  10*y_extent_at_focus*me.get_dy()/height*z/zoom_sensitivity;
				update_vec_member(view::focus);
				post_redraw();
				return true;
			}
			break;
		case MA_WHEEL :
			if (e.get_modifiers() == EM_ALT) {
				eye_distance -= 0.001*me.get_dy();
				if (eye_distance < 0)
					eye_distance = 0;
				update_member(&eye_distance);
				post_redraw();
				return true;
			}
			else if (e.get_modifiers() == EM_CTRL) {
				parallax_zero_scale *= exp(-0.03*me.get_dy());
				if (parallax_zero_scale > 1)
					parallax_zero_scale = 1;
				else if (parallax_zero_scale < 0.01)
					parallax_zero_scale = 0.01;
				update_member(&parallax_zero_scale);
				post_redraw();
				return true;
			}
			else if (e.get_modifiers() == EM_SHIFT) {
				y_view_angle += me.get_dy()*5;
				if (y_view_angle < 0)
					y_view_angle = 0;
				if (y_view_angle > 180)
					y_view_angle = 180;
				update_member(&y_view_angle);
				post_redraw();
				return true;
			}
			else if (e.get_modifiers() == 0) {
				double scale = exp(0.2*me.get_dy()/zoom_sensitivity);
				cgv::render::context* ctx = get_context();
				if (ctx) {
					pnt_type p;
					double z = get_z_and_unproject(ctx, me.get_x(), me.get_y(), p);
					if (z > 0 && z < 1) {
						view::focus = p + scale*(view::focus-p);
						update_vec_member(view::focus);
					}
				}
				y_extent_at_focus *= scale;
				update_member(&y_extent_at_focus);
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
void stereo_view_interactor::roll(double angle)
{
	view_up_dir = rotate(view_up_dir, view_dir, angle*.1745329252e-1);
	on_rotation_change();
}
///
void stereo_view_interactor::rotate_image_plane(double ax, double ay)
{
	
	vec_type x,y,z;
	put_coordinate_system(x,y,z);
	z = ay*x-ax*y;
	double a = z.length();
	if (a < 1e-6)
		return;
	z = (1/a) * z;
	a *= .1745329252e-1;
	view_dir = rotate(view_dir, z, a);
	view_up_dir = rotate(view_up_dir, z, a);
	on_rotation_change();
}

void stereo_view_interactor::on_rotation_change()
{
	for (unsigned i=0; i<3; ++i) {
		if (fix_view_up_dir)
			view_up_dir(i) = i==1 ? 1 : 0;
		update_member(&view_up_dir(i));
		update_member(&view_dir(i));
	}
	post_redraw();
}

/// overload to stream help information to the given output stream
void stereo_view_interactor::stream_help(std::ostream& os)
{
	os << "stereo_view_interactor\n\a"
		<< "stereo:    on/off with <F4>, toggle mode with <Shift-F4>\n"
		<< "set focus:                    left mouse button click\n"
		<< "rotate in image plane:        left mouse button\n";
	if (fix_view_up_dir)
		os << "rotate around view direction: disabled [enable by disable of |stereo interactor->Current View->fix_view_up_dir|]\n";
	else
		os << "rotate around view direction: Shift+left mouse button\n";

	os << "dolly zoom / zoom to point:   [Shift+]mouse wheel / PgUp,PgDn\n"
	   << "change eye separation:        Alt + mouse wheel\n"
	   << "move parallax zero plane:     Ctrl + mouse wheel\n"
		<< "move parallel to view dir:   middle mouse button\n"
	   << "move parallel to image plane: right mouse button\n"
	   << "decrease/increase z_near/far: [Shift+]N/F\b\n";
}

/// this method is called in one pass over all drawables after drawing
void stereo_view_interactor::finish_frame(cgv::render::context& ctx)
{
	cgv::render::RenderPassFlags rpf = ctx.get_render_pass_flags();
	if ((rpf & RPF_SET_MODELVIEW_PROJECTION) == 0)
		return;
	if (show_focus) {
		ctx.push_P();
		ctx.push_V();
		ctx.set_P(P);
		ctx.set_V(V);

		glDisable(GL_DEPTH_TEST);
		glLineStipple(1,15);
		glEnable(GL_LINE_STIPPLE);
		draw_focus();
		glDisable(GL_LINE_STIPPLE);
		glEnable(GL_DEPTH_TEST);

		ctx.pop_V();
		ctx.pop_P();
	}
}

/// this method is called in one pass over all drawables after finish frame
void stereo_view_interactor::after_finish(cgv::render::context& ctx)
{
	if (ctx.get_render_pass() == RP_MAIN) {
		if (is_stereo_enabled())
			glsuConfigureStereo(GLSU_CENTER,stereo_mode,ac);
		check_write_image(ctx, (is_stereo_enabled()&&stereo_mode==GLSU_QUAD_BUFFER)?"_r":"");
	}
}

static unsigned int cms[8][2][3] = { 
	{ {1,0,0}, {0,0,1} },
	{ {1,0,0}, {0,1,1} },
	{ {1,1,0}, {0,0,1} },
	{ {1,0,1}, {0,1,0} },

	{ {0,0,1}, {1,0,0} },
	{ {0,1,1}, {1,0,0} },
	{ {0,0,1}, {1,1,0} },
	{ {0,1,0}, {1,0,1} }
};

void stereo_view_interactor::on_stereo_change()
{
	base* bp = dynamic_cast<base*>(get_context());
	if (bp) {
		bool need_quad_buffer = is_stereo_enabled() && (stereo_mode == GLSU_QUAD_BUFFER);
		if (need_quad_buffer != bp->get<bool>("quad_buffer")) {
			bp->set("quad_buffer", need_quad_buffer);
			if (need_quad_buffer && !bp->get<bool>("quad_buffer"))
				enable_stereo(false);
		}
	}
	post_redraw();
}

/// overload to set local lights before modelview matrix is set
void stereo_view_interactor::on_set_local_lights()
{
}

/// this method is called in one pass over all drawables before the draw method
void stereo_view_interactor::init_frame(context& ctx)
{
	cgv::render::RenderPassFlags rpf = ctx.get_render_pass_flags();
	if ((rpf & RPF_SET_MODELVIEW_PROJECTION) == 0)
		return;

	GlsuEye e = mono_mode;
	if (is_stereo_enabled()) {
		if (ctx.get_render_pass() == RP_STEREO)
			e = GLSU_LEFT;
		else {
			glsuConfigureStereo(GLSU_LEFT,stereo_mode,ac);
			ctx.render_pass(RP_STEREO,RenderPassFlags(rpf&~RPF_HANDLE_SCREEN_SHOT));
			glsuConfigureStereo(GLSU_RIGHT,stereo_mode,ac);
			e = GLSU_RIGHT;
		}
	}
	// determine aspect ratio from opengl settings
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	double aspect = (double)vp[2]/vp[3];

	// compute eye and focus point
	pnt_type foc = view::focus;
	pnt_type eye = (y_view_angle <= 0.1) ? foc-0.3*z_far*view_dir : get_eye();
	pnt_type dv  = (1/parallax_zero_scale-2)*(eye - foc);
	foc += dv;
	eye += dv;
	double y_extent = y_extent_at_focus;
	double z_eye = dot(eye,view_dir);
	double z_focus = dot(foc,view_dir)-z_eye;

	// compute the clipping planes based on the eye and scene extent
	z_near_derived = z_near;
	z_far_derived = z_far;
	if (scene_extent.is_valid()) {
		box_type B = scene_extent;
		B.scale(1.1);
		double z_min = dot(B.get_corner(0),view_dir);
		double z_max = z_min;
		for (unsigned int i=1; i<8; ++i) {
			double new_z= dot(B.get_corner(i),view_dir);
			if (new_z< z_min)
				z_min = new_z;
			if (new_z > z_max)
				z_max = new_z;
		}
		z_min -= z_eye;
		z_max -= z_eye;
		if (z_min > z_near)
			z_near_derived = z_min;
		if (z_max > z_near && z_max < z_far)
			z_far_derived = z_max;
	}
	else if (clip_relative_to_extent) {
		z_near_derived = y_extent_at_focus*z_near;
		z_far_derived = y_extent_at_focus*z_far;
	}

	if (rpf & RPF_SET_PROJECTION) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if (y_view_angle <= 0.1)
			glOrtho(-aspect*y_extent, aspect*y_extent, 
					-y_extent, y_extent, z_near_derived, z_far_derived);
		else {
			if (stereo_translate_in_model_view)
				glsuStereoFrustumScreen(e, eye_distance, 2*y_extent*aspect, 2*y_extent, z_focus, z_near_derived, z_far_derived);
			else
				glsuStereoPerspectiveScreen(e, eye_distance, 2*y_extent*aspect, 2*y_extent, z_focus, z_near_derived, z_far_derived);
		}
	}
	glMatrixMode(GL_MODELVIEW);
	if (rpf & RPF_SET_MODELVIEW) {
		// switch back to the modelview transformation stack
		glLoadIdentity();
	}
	if (rpf & RPF_SET_LIGHTS) {
		float lps[] = { 0,1,1,0, 1,0,1,0, 0,0,1,0, 0,1,0,0 };

		glLightfv(GL_LIGHT0, GL_POSITION, lps);
		glLightfv(GL_LIGHT1, GL_POSITION, lps+4);
		glLightfv(GL_LIGHT2, GL_POSITION, lps+8);
		glLightfv(GL_LIGHT3, GL_POSITION, lps+12);
	}
	on_set_local_lights();
	if (rpf & RPF_SET_MODELVIEW) {
		if (stereo_translate_in_model_view)
			glsuStereoTranslateScreen(e, eye_distance, y_extent*aspect);
		gluLookAt(eye(0), eye(1), eye(2), foc(0), foc(1), foc(2), view_up_dir(0),view_up_dir(1),view_up_dir(2));

		DPV = ctx.get_DPV();
	}
}

/// 
void stereo_view_interactor::draw(cgv::render::context& ctx)
{
	if (show_focus) {
		V = ctx.get_V();
		P = ctx.get_P();
		draw_focus();
	}
}


void stereo_view_interactor::draw_focus()
{
	glLineWidth(1.0f);
	glColor3f(0.5f,0.5f,0.5f);
	glBegin(GL_LINES);
	glVertex3dv(view::focus);
	glVertex3dv(view::focus+vec_type(get_y_extent_at_focus(),0,0));
	glVertex3dv(view::focus);
	glVertex3dv(view::focus+vec_type(0,get_y_extent_at_focus(),0));
	glVertex3dv(view::focus);
	glVertex3dv(view::focus+vec_type(0,0,get_y_extent_at_focus()));
	glVertex3dv(view::focus);
	glVertex3dv(view::focus+vec_type(-get_y_extent_at_focus(),0,0));
	glVertex3dv(view::focus);
	glVertex3dv(view::focus+vec_type(0,-get_y_extent_at_focus(),0));
	glVertex3dv(view::focus);
	glVertex3dv(view::focus+vec_type(0,0,-get_y_extent_at_focus()));
	glEnd();
}


/// return a shortcut to activate the gui without menu navigation
cgv::gui::shortcut stereo_view_interactor::get_shortcut() const
{
	return cgv::gui::shortcut('V', EM_CTRL);
}


/// return a path in the main menu to select the gui
std::string stereo_view_interactor::get_menu_path() const
{
	return "view/stereo interactor";
}

void stereo_view_interactor::check_write_image(context& ctx, const char* post_fix, bool done)
{
	if (!write_images)
		return;
	
	std::string ext("bmp");
	std::string exts = cgv::media::image::image_writer::get_supported_extensions();
	if (cgv::utils::is_element("png",exts))
		ext = "png";
	else if (cgv::utils::is_element("tif",exts))
		ext = "tif";

	std::string file_name = image_file_name_prefix;
	if (write_stereo) {
		if (mono_mode == GLSU_LEFT)
			file_name += "_left";
		else if (mono_mode == GLSU_RIGHT)
			file_name += "_right";
	}
	if (write_color) {
		ctx.write_frame_buffer_to_image(file_name+"_rgb." + ext);
		if (auto_view_images)
			system((std::string("\"")+file_name+"_rgb." + ext + "\"").c_str());
	}

	if (write_depth) { 
		ctx.write_frame_buffer_to_image(file_name+"_depth." + ext, cgv::data::CF_D,FB_BACK,0,0,-1,-1,depth_offset, depth_scale);
		if (auto_view_images) 
			system((std::string("\"")+file_name+"_depth." + ext + "\"").c_str());
	}
}

void stereo_view_interactor::write_images_to_file()
{
	context* ctx = get_context();
	if (ctx == 0)
		return;

	if ( (write_width != -1 && ctx->get_width() != write_width) ||
		 (write_height != -1 && ctx->get_height() != write_height) ) {
			ctx->resize(write_width, write_height);
	}

	if (!stereo_enabled && write_stereo) {
		GlsuEye tmp = mono_mode;
		mono_mode = GLSU_LEFT;
		ctx->force_redraw();
		mono_mode = GLSU_RIGHT;
		ctx->force_redraw();
		mono_mode = tmp;
	}
	else
		ctx->force_redraw();

	write_images = false;
	update_member(&write_images);
}

void stereo_view_interactor::add_dir_control(const std::string& name, vec_type& dir)
{
	add_control(name+" x", dir(0), "value_input", "w=50;step=0.01;min=-1;max=1;ticks=true"," ");
	add_control("y", dir(1), "value_input", "w=50;step=0.01;min=-1;max=1;ticks=true"," ");
	add_control("z", dir(2), "value_input", "w=50;step=0.01;min=-1;max=1;ticks=true");
	connect_copy(find_control(dir(0))->value_change,
		rebind(this, &stereo_view_interactor::dir_gui_cb, cgv::signal::_r(dir), 0));
	connect_copy(find_control(dir(1))->value_change,
		rebind(this, &stereo_view_interactor::dir_gui_cb, cgv::signal::_r(dir), 1));
	connect_copy(find_control(dir(2))->value_change,
		rebind(this, &stereo_view_interactor::dir_gui_cb, cgv::signal::_r(dir), 2));
}

void stereo_view_interactor::dir_gui_cb(vec_type& dir, int i)
{
	int j = (i+1)%3, k=(i+2)%3;
	double r2_old = dir(j)*dir(j)+dir(k)*dir(k);
	if (r2_old < 1e-10) {
		dir(j)=dir(k)=1;
		r2_old = 2;
	}
	double r2_new = 1-dir(i)*dir(i);
	double scale = sqrt(r2_new/r2_old);
	dir(j) *= scale;
	dir(k) *= scale;
	update_vec_member(dir);
	post_redraw();
}


/// you must overload this for gui creation
void stereo_view_interactor::create_gui()
{
	if (begin_tree_node("View Configuration", zoom_sensitivity, false)) {
		align("\a");
			add_member_control(this, "zoom_sensitivity", zoom_sensitivity, "value_slider", "min=0.1;max=10;ticks=true;step=0.01;log=true");
			add_member_control(this, "rotate_sensitivity", rotate_sensitivity, "value_slider", "min=0.1;max=10;ticks=true;step=0.01;log=true");
			add_member_control(this, "show focus", show_focus, "check");
		align("\b");
		end_tree_node(zoom_sensitivity);
	}
	if (begin_tree_node("Framebuffer to Image", write_images, false)) {
		align("\a");
			add_member_control(this, "write buffers to file", write_images, "toggle");
			add_member_control(this, "width", write_width);
			add_member_control(this, "height", write_height);
			add_member_control(this, "write left/right", write_stereo);
			add_member_control(this, "write color", write_color);
			add_member_control(this, "write depth", write_depth);
			add_member_control(this, "depth offset", depth_offset, "value_input", "min=0;max=1;ticks=true;log=true");
			add_member_control(this, "depth scale", depth_scale, "value_input", "min=0.01;max=100;ticks=true;log=true");
			add_member_control(this, "image file name", image_file_name_prefix);
			add_member_control(this, "autoview", auto_view_images, "check");
		align("\b");
		end_tree_node(write_images);
	}
	if (begin_tree_node("Stereo Parameters", stereo_enabled, true)) {
		align("\a");
			connect_copy(add_control("stereo", stereo_enabled, "check")->value_change, rebind(this, &stereo_view_interactor::on_stereo_change));
			add_member_control(this, "mono mode", mono_mode, "dropdown", "enums='left=-1,center,right'");
			add_member_control(this, "stereo mode", stereo_mode, "dropdown", "enums='vsplit,hsplit,anaglyph,quad buffer'");
			add_member_control(this, "config", ac, "dropdown", "enums='<red|blue>,<red|cyan>,<yellow|blue>,<magenta|green>,<blue|red>,<cyan|red>,<blue|yellow>,<green|magenta>'");
			add_member_control(this, "eye-dist", eye_distance, "value_slider", "min=0;max=0.1;ticks=true;step=0.001");
			add_member_control(this, "parallax-zero-scale", parallax_zero_scale, "value_slider", "min=0.03;max=1;ticks=true;step=0.001;log=true");		
			add_member_control(this, "stereo_translate_in_model_view", stereo_translate_in_model_view, "check");
		align("\b");
		end_tree_node(stereo_enabled);
	}
	if (begin_tree_node("Current View", view::focus(0), true)) {
		align("\a");
			add_member_control(this, "focus x", view::focus(0), "value_input", "w=50;min=-10;max=10;ticks=true"," ");
			add_member_control(this, "y", view::focus(1), "value_input", "w=50;min=-10;max=10;ticks=true"," ");
			add_member_control(this, "z", view::focus(2), "value_input", "w=50;min=-10;max=10;ticks=true");
			///
			add_dir_control("view dir",view_dir);
			add_dir_control("up dir",view_up_dir);
			connect_copy(add_control("fix_view_up_dir", fix_view_up_dir, "check")->value_change, rebind(this, &stereo_view_interactor::on_rotation_change));

			add_member_control(this, "y view angle", y_view_angle, "value_slider", "min=0;max=90;ticks=true;log=true");
			add_member_control(this, "extent", y_extent_at_focus, "value_slider", "min=0;max=100;ticks=true;log=true;step=0.0001");
			add_member_control(this, "z_near", z_near, "value_slider", "min=0;max=100;log=true;step=0.00001");
			add_member_control(this, "z_far", z_far, "value_slider", "min=0;max=10000;log=true;step=0.00001");
			add_member_control(this, "clip_relative_to_extent", clip_relative_to_extent, "check");
		align("\b");
		end_tree_node(view::focus(0));
	}
}

/*
	srh.reflect_member("mode", stereo_mode);, "vsplit,hsplit,anaglyph,quad buffer")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("config", ac, 
		"<red|blue>,<red|cyan>,<yellow|blue>,<magenta|green>,<blue|red>,<cyan|red>,<blue|yellow>,<green|magenta>")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
*/

std::string stereo_view_interactor::get_property_declarations()
{
	return cgv::base::base::get_property_declarations()+";stereo_mode:string(vsplit,hsplit,anaglyph,quad buffer);stereo_config(<red|blue>,<red|cyan>,<yellow|blue>,<magenta|green>,<blue|red>,<cyan|red>,<blue|yellow>,<green|magenta>):string";
}

bool stereo_view_interactor::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (cgv::base::base::set_void(property, value_type, value_ptr))
		return true;
	
	if (property == "stereo_mode") {
		std::string v;
		cgv::type::get_variant(v, value_type, value_ptr);
		if (v == "vsplit")
			stereo_mode = GLSU_SPLIT_VERTICALLY;
		else if (v == "hsplit")
			stereo_mode = GLSU_SPLIT_HORIZONTALLY;
		else if (v == "anaglyph")
			stereo_mode = GLSU_ANAGLYPH;
		else if (v == "quad buffer")
			stereo_mode = GLSU_QUAD_BUFFER;
		else
			std::cerr << "string value of stereo_mode must be one out of 'vsplit,hsplit,anaglyph,quad buffer'" << std::endl;
		on_set(&stereo_mode);
		return true;
	}
	if (property == "stereo_config") {
		std::string v;
		cgv::type::get_variant(v, value_type, value_ptr);
		if (v == "<red|blue>")
			ac = GLSU_RED_BLUE;
		else if (v == "<red|cyan>")
			ac = GLSU_RED_CYAN;
		else if (v == "<yellow|blue>")
			ac = GLSU_YELLOW_BLUE;
		else if (v == "<magenta|green>")
			ac = GLSU_MAGENTA_GREEN;
		else if (v == "<blue|red>")
			ac = GLSU_BLUE_RED;
		else if (v == "<cyan|red>")
			ac = GLSU_CYAN_RED;
		else if (v == "<blue|yellow>")
			ac = GLSU_BLUE_YELLOW;
		else if (v == "<green|magenta>")
			ac = GLSU_GREEN_MAGENTA;
		else
			std::cerr << "string value of stereo_mode must be one out of '<red|blue>,<red|cyan>,<yellow|blue>,<magenta|green>,<blue|red>,<cyan|red>,<blue|yellow>,<green|magenta>'" << std::endl;
		on_set(&ac);
		return true;
	}
	return false;
}

bool stereo_view_interactor::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (cgv::base::base::get_void(property, value_type, value_ptr))
		return true;
	
	if (property == "stereo_mode") {
		switch (stereo_mode) {
		case GLSU_SPLIT_VERTICALLY : cgv::type::set_variant(std::string("vsplit"), value_type, value_ptr); break;
		case GLSU_SPLIT_HORIZONTALLY : cgv::type::set_variant(std::string("hsplit"), value_type, value_ptr); break;
		case GLSU_ANAGLYPH : cgv::type::set_variant(std::string("anaglyph"), value_type, value_ptr); break;
		case GLSU_QUAD_BUFFER : cgv::type::set_variant(std::string("quad buffer"), value_type, value_ptr); break;
		}
		return true;
	}
	return false;
}


void stereo_view_interactor::on_set(void* m)
{
	if (m == &stereo_enabled || m == &stereo_mode)
		on_stereo_change();
	if (find_control_void(m,0))
		find_control_void(m,0)->update();
	if (m == &write_images) {
		write_images_to_file();
	}
	else {
		post_redraw();
	}
}

/// you must overload this for gui creation
bool stereo_view_interactor::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return 
	srh.reflect_member("rotate_sensitivity", rotate_sensitivity) &&
	srh.reflect_member("zoom_sensitivity", zoom_sensitivity) &&
	srh.reflect_member("focus_x", view::focus(0)) &&
	srh.reflect_member("focus_y", view::focus(1)) &&
	srh.reflect_member("focus_z", view::focus(2)) &&
	srh.reflect_member("stereo", stereo_enabled) &&
	srh.reflect_member("eye_dist", eye_distance) &&
	srh.reflect_member("parallax_zero_scale", parallax_zero_scale) &&
	srh.reflect_member("stereo_translate_in_model_view", stereo_translate_in_model_view) &&
	srh.reflect_member("view_dir_x",view_dir(0)) &&
	srh.reflect_member("view_dir_y",view_dir(1)) &&
	srh.reflect_member("view_dir_z",view_dir(2)) &&
	srh.reflect_member("fix_view_up_dir", fix_view_up_dir) &&
	srh.reflect_member("up_dir_x",view_up_dir(0)) &&
	srh.reflect_member("up_dir_y",view_up_dir(1)) &&
	srh.reflect_member("up_dir_z",view_up_dir(2)) &&
	srh.reflect_member("y_view_angle", y_view_angle) &&
	srh.reflect_member("extent", y_extent_at_focus) &&
	srh.reflect_member("z_near", z_near) &&
	srh.reflect_member("z_far", z_far) &&
	srh.reflect_member("two_d_enabled", two_d_enabled) &&
	srh.reflect_member("show_focus", show_focus) &&
	srh.reflect_member("clip_relative_to_extent", clip_relative_to_extent);
}


#ifndef NO_STEREO_VIEW_INTERACTOR

#include <cgv/base/register.h>

/// register a newly created cube with the name "cube1" as constructor argument
extern cgv::base::object_registration_1<stereo_view_interactor,const char*> 
 obj1("stereo interactor", "registration of stereo interactor");

#endif