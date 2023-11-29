#include "antialias.h"
#include <cgv/utils/ostream_printf.h>
#include <cgv/utils/convert.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/find_action.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/render/view.h>
#include <cgv/type/variant.h>
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
antialias::antialias() : node("Antialias")
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

void antialias::update_view(cgv::render::context& ctx, int i)
{
	dmat4 m;
	m.identity();
	double dx,dy;
	put_sample(i,dx,dy);
	m(0,2) = -dx*pixel_scale_x;
	m(1,2) = -dy*pixel_scale_y;
	ctx.mul_projection_matrix(m);
}

void antialias::restore_view(cgv::render::context& ctx, int i)
{
	dmat4 m;
	m.identity();
	double dx,dy;
	put_sample(i,dx,dy);
	m(0,2) = dx*pixel_scale_x;
	m(1,2) = dy*pixel_scale_y;
	ctx.mul_projection_matrix(m);
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

bool antialias::init(context& ctx)
{
	ctx.attach_accumulation_buffer();
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
	// check of need of recursion
	if (initiate_render_pass_recursion(ctx)) {
		glClearAccum(0, 0, 0, 0);
		glClear(GL_ACCUM_BUFFER_BIT);
		// iterate all but first view sample
		for (int i = 1; i < nr_samples; ++i) {
			perform_render_pass(ctx, i, RP_USER_DEFINED, RPF_HANDLE_SCREEN_SHOT + RPF_CLEAR_ACCUM);
			restore_view(ctx, i);
		}
		// finally start rendering the view sample in main render pass
		initiate_terminal_render_pass(0);
	}
	update_view(ctx, current_render_pass);
}

/// this method is called in one pass over all drawables after finish frame
void antialias::after_finish(cgv::render::context& ctx)
{
	if (multi_pass_ignore_finish(ctx))
		return;

	glReadBuffer(GL_BACK);
	glAccum(GL_ACCUM, 1.0f/nr_samples);
	if (multi_pass_terminate(ctx)) {
		glAccum(GL_RETURN, 1.0f);
		restore_view(ctx, 0);
	}
}

/// you must overload this for gui creation
void antialias::create_gui()
{
	add_decorator("Antialias", "heading");
	add_member_control(this, "enable", drawable::active, "check");
	add_member_control(this, "nr_samples", nr_samples, "value_slider", "min=1;max=15;log=true;ticks=true");
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

cgv::base::factory_registration<antialias> anti_reg("antialias", "menu_text='New/Antialias';shortcut='Ctrl-Alt-A'", true, 
														                "menu_text='&View/Antialias';shortcut='Ctrl-A'");
