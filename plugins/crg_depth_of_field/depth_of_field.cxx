#include "depth_of_field.h"
#include "../crg_stereo_view/stereo_view_interactor.h"
#include <cgv/utils/ostream_printf.h>
#include <cgv/utils/convert.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/find_action.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/type/variant.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cgv_gl/gl/gl.h>
#include <stdio.h>

using namespace cgv::math;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::utils;
using namespace cgv::render;
using namespace cgv::render::gl;

///
depth_of_field::depth_of_field() : node("depth of field")
{
	z_focus = 10;
	nr_samples = 10;
	aperture = 0.1;
	enabled = true;
	copy_z_focus = true;
}

void depth_of_field::put_sample(int i, double& dx, double& dy)
{
	if (samples.size() != 2*nr_samples)
		generate_samples();
	dx = aperture*samples[2*i];
	dy = aperture*samples[2*i+1];
}

void depth_of_field::generate_samples()
{
	samples.resize(2*nr_samples);
	for (int i = 0; i < nr_samples; ++i) {
		double x, y;
		do {
			x = (2.0*rand())/RAND_MAX-1.0;
			y = (2.0*rand())/RAND_MAX-1.0;
		} while (x*x+y*y > 1);
		samples[2*i] = x;
		samples[2*i+1] = y;
	}
}

void depth_of_field::update_view(cgv::render::context& ctx, int i)
{
	dmat4 m;
	m.identity();
	double dx,dy;
	put_sample(i,dx,dy);
	m(0,2) = -dx/z_focus;
	m(1,2) = -dy/z_focus;
	m(0,3) = -dx;
	m(1,3) = -dy;
	ctx.mul_projection_matrix(m);
}

void depth_of_field::restore_view(cgv::render::context& ctx, int i)
{
	dmat4 m;
	m.identity();
	double dx,dy;
	put_sample(i,dx,dy);
	m(0,2) = dx/z_focus;
	m(1,2) = dy/z_focus;
	m(0,3) = dx;
	m(1,3) = dy;
	ctx.mul_projection_matrix(m);
}


/// return the type name 
std::string depth_of_field::get_type_name() const
{
	return "depth_of_field";
}
/// overload to show the content of this object
void depth_of_field::stream_stats(std::ostream& os)
{
	oprintf(os,"DoF: z-focus=%.2fº, nr samples=%.1f\n", z_focus, nr_samples);
}

/// overload and implement this method to handle events
bool depth_of_field::handle(event& e)
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
void depth_of_field::stream_help(std::ostream& os)
{
}

bool depth_of_field::init(context& ctx)
{
	ctx.attach_accum_buffer();
//	ctx.set_bg_accum_color(0,0,0,0);
//	ctx.set_default_render_pass_flags(RenderPassFlags(ctx.get_default_render_pass_flags()&~RPF_CLEAR_ACCUM));
	return true;
}

/// this method is called in one pass over all drawables before the draw method
void depth_of_field::init_frame(context& ctx)
{
	if (!enabled)
		return;

	if (copy_z_focus) {
		std::vector<ext_view*> views;
		cgv::base::find_interface(base_ptr(this),views);
		if (views.size() > 0) {
			z_focus = views[0]->get_parallax_zero_z();
			update_member(&z_focus);
		}
	}
	// avoid infinite recursion
	if (ctx.get_render_pass() == RP_USER_DEFINED && ctx.get_render_pass_user_data() == this) {
		update_view(ctx, iter);
		return;
	}
	glClearAccum(0,0,0,0);
	glClear(GL_ACCUM_BUFFER_BIT);
	// iterate all but first view sample
	for (int i = 1; i<nr_samples; ++i) {
		cgv::render::RenderPassFlags rpf = ctx.get_render_pass_flags();
		iter = i;
		ctx.render_pass(RP_USER_DEFINED,RenderPassFlags(rpf & ~(RPF_HANDLE_SCREEN_SHOT|RPF_CLEAR_ACCUM)), this);
		restore_view(ctx, i);
	}
	// finally start rendering the view sample in main render pass
	iter = 0;
	update_view(ctx, 0);
}

/// this method is called in one pass over all drawables after drawing
void depth_of_field::finish_frame(cgv::render::context& ctx)
{
}

/// this method is called in one pass over all drawables after finish frame
void depth_of_field::after_finish(cgv::render::context& ctx)
{
	if (!enabled)
		return;
	glReadBuffer(GL_BACK);
	glAccum(GL_ACCUM, 1.0f/nr_samples);
	if (ctx.get_render_pass() == RP_USER_DEFINED && ctx.get_render_pass_user_data() == this)
		return;
	glAccum(GL_RETURN, 1.0f);
	restore_view(ctx, 0);
}

/// return a shortcut to activate the gui without menu navigation
cgv::gui::shortcut depth_of_field::get_shortcut() const
{
	return cgv::gui::shortcut('D', EM_CTRL);
}


/// return a path in the main menu to select the gui
std::string depth_of_field::get_menu_path() const
{
	return "view/depth of field";
}

/// you must overload this for gui creation
void depth_of_field::create_gui()
{
	add_decorator("Depth of Field", "heading");
	connect_copy(add_control("enable", enabled, "check")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("copy_z_focus", copy_z_focus, "check")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("z-focus", z_focus, "value_slider", "min=0.01;max=1000;ticks=true;step=0.001;log=true")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("aperture", aperture, "value_slider", "min=0.001;max=10;ticks=true;step=0.0001;log=true")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("nr_samples", nr_samples, "value_slider", "min=1;max=255;log=true;ticks=true")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
}

void depth_of_field::on_set(void* m)
{
	update_member(m);
	post_redraw();
}

/// you must overload this for gui creation
bool depth_of_field::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return
		rh.reflect_member("aperture", aperture) &&
		rh.reflect_member("nr_samples", nr_samples) &&
		rh.reflect_member("enabled", enabled) &&
		rh.reflect_member("copy_z_focus", copy_z_focus) &&
		rh.reflect_member("z_focus", z_focus);
}

#include <cgv/base/register.h>

extern cgv::base::factory_registration<depth_of_field> dof_fac("depth_of_field", "menu_text='new/depth of field';shortcut='Ctrl-Alt-D'", true, 
														                "menu_text='&view/depth of field';shortcut='Ctrl-D'");
