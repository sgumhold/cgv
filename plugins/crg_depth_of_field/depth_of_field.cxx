#include "depth_of_field.h"
#include <cgv/utils/ostream_printf.h>
#include <cgv/utils/convert.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/find_action.h>
#include <cgv/render/stereo_view.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/type/variant.h>
#include <cmath>
#include <cgv_gl/gl/gl.h>
#include <stdio.h>

using namespace cgv::math;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::utils;
using namespace cgv::render;

///
depth_of_field::depth_of_field() : node("Depth of Field")
{
	z_focus = 10;
	nr_samples = 10;
	aperture = 0.1;
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
	cgv::dmat4 m;
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
	cgv::dmat4 m;
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

bool depth_of_field::init(context& ctx)
{
	ctx.attach_accumulation_buffer();
	return true;
}

/// this method is called in one pass over all drawables before the draw method
void depth_of_field::init_frame(context& ctx)
{
	if (copy_z_focus) {
		cgv::render::view* view_ptr = find_view_as_node();
		if (view_ptr) {
			cgv::render::stereo_view* stereo_view_ptr = dynamic_cast<cgv::render::stereo_view*>(view_ptr);
			if (stereo_view_ptr)
				z_focus = stereo_view_ptr->get_parallax_zero_depth();
			else
				z_focus = view_ptr->get_depth_of_focus();
			update_member(&z_focus);
		}
	}
	// check if this has not issued render passes
	if (initiate_render_pass_recursion(ctx)) {
		// init accumulation
		glClearAccum(0, 0, 0, 0);
		glClear(GL_ACCUM_BUFFER_BIT);

		// perform recursive render passes but first one
		for (int i = 1; i < nr_samples; ++i) {
			perform_render_pass(ctx, i, RP_USER_DEFINED, RPF_HANDLE_SCREEN_SHOT + RPF_CLEAR_ACCUM);
			restore_view(ctx, i);
		}
		// finally start rendering the view sample in main render pass
		initiate_terminal_render_pass(0);
	}
	update_view(ctx, current_render_pass);
}

/// this method is called in one pass over all drawables after drawing
void depth_of_field::finish_frame(cgv::render::context& ctx)
{
}

/// this method is called in one pass over all drawables after finish frame
void depth_of_field::after_finish(cgv::render::context& ctx)
{
	if (multi_pass_ignore_finish(ctx))
		return;
	
	// do accumulation
	glReadBuffer(GL_BACK);
	glAccum(GL_ACCUM, 1.0f / nr_samples);

	// check for last accumulation pass
	if (multi_pass_terminate(ctx)) {
		// write back accumulated buffer and terminate accumulation
		glAccum(GL_RETURN, 1.0f);
		restore_view(ctx, 0);
	}
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
	add_member_control(this, "enable", drawable::active, "check");
	add_member_control(this, "nr_samples", nr_samples, "value_slider", "min=1;max=255;log=true;ticks=true");
	add_member_control(this, "aperture", aperture, "value_slider", "min=0.001;max=10;ticks=true;step=0.0001;log=true");
	add_member_control(this, "z-focus", z_focus, "value_slider", "min=0.01;max=1000;ticks=true;step=0.001;log=true");
	add_member_control(this, "copy_z_focus", copy_z_focus, "check");
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
		rh.reflect_member("enabled", drawable::active) &&
		rh.reflect_member("copy_z_focus", copy_z_focus) &&
		rh.reflect_member("z_focus", z_focus);
}

#include <cgv/base/register.h>

cgv::base::factory_registration<depth_of_field> dof_fac("depth_of_field", "menu_text='New/Depth of Field';shortcut='Ctrl-Alt-D'", true, 
														                "menu_text='&View/Depth of Field';shortcut='Ctrl-D'");
