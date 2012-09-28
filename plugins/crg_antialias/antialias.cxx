#include "antialias.h"
#include <cgv/utils/ostream_printf.h>
#include <cgv/utils/convert.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/find_action.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/render/view.h>
#include <cgv/type/variant.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cgv_gl/gl/gl.h>
#include <stdio.h>

using namespace cgv::math;
using namespace cgv::gui;
using namespace cgv::signal;
using namespace cgv::utils;
using namespace cgv::render;
using namespace cgv::render::gl;
	
///
antialias::antialias() : node("antialias")
{
	nr_samples = 15;
}

void antialias::put_sample(int i, double& dx, double& dy)
{
	if (samples.size() != 2*nr_samples)
		generate_samples();
	dx = samples[2*i];
	dy = samples[2*i+1];
}

void antialias::generate_samples()
{
	static double jitter_points[15][2] =
	{
		{ 0.285561,  0.188437}, { 0.360176, -0.065688}, {-0.111751,  0.275019},
		{-0.055918, -0.215197},	{-0.080231, -0.470965},	{ 0.138721,  0.409168},
		{ 0.384120,  0.458500},	{-0.454968,  0.134088},	{ 0.179271, -0.331196},
		{-0.307049, -0.364927},	{ 0.105354, -0.010099},	{-0.154180,  0.021794},
		{-0.370135, -0.116425},	{ 0.451636, -0.300013},	{-0.370610,  0.387504}
	};

	samples.resize(2*nr_samples);
	for (int i = 0; i < nr_samples; ++i) {
		samples[2*i] = jitter_points[i][0];
		samples[2*i+1] = jitter_points[i][1];
	}
}

void antialias::update_view(int i)
{
	cgv::math::fmat<double,4,4> m;
	m.identity();
	double dx,dy;
	put_sample(i,dx,dy);
	m(0,2) = -dx*pixel_scale_x;
	m(1,2) = -dy*pixel_scale_y;
	glMatrixMode(GL_PROJECTION);
	glMultMatrixd(m);
	glMatrixMode(GL_MODELVIEW);
}

void antialias::restore_view(int i)
{
	cgv::math::fmat<double,4,4> m;
	m.identity();
	double dx,dy;
	put_sample(i,dx,dy);
	m(0,2) = dx*pixel_scale_x;
	m(1,2) = dy*pixel_scale_y;
	glMatrixMode(GL_PROJECTION);
	glMultMatrixd(m);
	glMatrixMode(GL_MODELVIEW);
}


/// return the type name 
std::string antialias::get_type_name() const
{
	return "antialias";
}
/// overload to show the content of this object
void antialias::stream_stats(std::ostream& os)
{
	oprintf(os,"Anti: nr samples=%.1d\n", nr_samples);
}

/// overload and implement this method to handle events
bool antialias::handle(event& e)
{
/*
    if (e.get_kind() == EID_KEY) {
		key_event ke = (key_event&) e;
		if (ke.get_action() == KA_PRESS) {
			switch (ke.get_key()) {
			}
		}
	}
	*/
	return false;
}

/// overload to stream help information to the given output stream
void antialias::stream_help(std::ostream& os)
{
}

bool antialias::init(context& ctx)
{
	ctx.attach_accum_buffer();
	return true;
}

/// this method is called in one pass over all drawables before the draw method
void antialias::init_frame(context& ctx)
{
	// determine pixel scale
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	std::vector<cgv::render::view*> views;
	cgv::base::find_interface(base_ptr(this),views);
	if (views.size() > 0) {
		pixel_scale_y = 2*tan(.8726646262e-2*views[0]->get_y_view_angle())/vp[3];
		pixel_scale_x = pixel_scale_y*vp[2]/vp[3];
	}
	// avoid infinite recursion
	if (ctx.get_render_pass() == RP_USER_DEFINED && ctx.get_render_pass_user_data() == this) {
		update_view(iter);
		return;
	}
	glClearAccum(0,0,0,0);
	glClear(GL_ACCUM_BUFFER_BIT);
	// iterate all but first view sample
	for (int i = 1; i<nr_samples; ++i) {
		cgv::render::RenderPassFlags rpf = ctx.get_default_render_pass_flags();
		iter = i;
		ctx.render_pass(RP_USER_DEFINED,RenderPassFlags(rpf & ~(RPF_HANDLE_SCREEN_SHOT|RPF_CLEAR_ACCUM)), this);
		restore_view(i);
	}
	// finally start rendering the view sample in main render pass
	iter = 0;
	update_view(0);
}

/// this method is called in one pass over all drawables after drawing
void antialias::finish_frame(cgv::render::context& ctx)
{
}

/// this method is called in one pass over all drawables after finish frame
void antialias::after_finish(cgv::render::context& ctx)
{
	glReadBuffer(GL_BACK);
	glAccum(GL_ACCUM, 1.0f/nr_samples);
	if (ctx.get_render_pass() == RP_USER_DEFINED && ctx.get_render_pass_user_data() == this)
		return;
	glAccum(GL_RETURN, 1.0f);
	restore_view(0);
}
/*
/// return a shortcut to activate the gui without menu navigation
cgv::gui::shortcut antialias::get_shortcut() const
{
	return cgv::gui::shortcut('A', EM_CTRL);
}


/// return a path in the main menu to select the gui
std::string antialias::get_menu_path() const
{
	return "view/antialias";
}
*/
/// you must overload this for gui creation
void antialias::create_gui()
{
	add_decorator("Antialias", "heading");
	connect_copy(add_control("enable", drawable::active, "check")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("nr_samples", nr_samples, "value_slider", "min=1;max=15;log=true;ticks=true")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
}

void antialias::on_set(void* m)
{
	update_member(m);
	post_redraw();
}

/// you must overload this for gui creation
bool antialias::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return 
		srh.reflect_member("nr_samples", nr_samples) &&
		srh.reflect_member("enabled", drawable::active);
}

#include <cgv/base/register.h>

extern cgv::base::factory_registration<antialias> anti_reg("antialias", "menu_text='new/antialias';shortcut='Ctrl-Alt-A'", true, 
														                "menu_text='&view/antialias';shortcut='Ctrl-A'");
