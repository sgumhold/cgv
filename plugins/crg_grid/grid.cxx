#include "grid.h"
#include <cgv/gui/key_event.h>
#include <cgv/base/find_action.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/trigger.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/render/view.h>
#include <cgv/render/shader_program.h>
#include <cgv/math/det.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/point_operations.h>
#include <cgv/math/functions.h>
#include <cgv/type/variant.h>
#include <cgv_gl/gl/gl.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::type;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;

grid::grid() : minX(-50),minZ(-50),maxX(50),maxZ(50) 
{
	view_ptr = 0;
	set_name("grid");
	threshold = 5;
	alpha = 0.5;
	show_grid = true;
	factor=0;
	adaptive_grid=true;
	show_axes = true;
	show_lines = true;

	arrow_length = 1.5;
	arrow_aspect = 0.03333f;
	arrow_rel_tip_radius = 2;
	arrow_tip_aspect = 0.3f;
}

/// overload to return the type name of this object
std::string grid::get_type_name() const
{
	return "grid";
}


bool grid::handle(event& e)
{
	if (e.get_kind() == EID_KEY) {
		key_event& ke = static_cast<key_event&>(e);
		if (ke.get_action() == KA_PRESS) {
			if ((ke.get_key() == 'G') && (ke.get_modifiers() == 0)) {
				show_grid=!show_grid;
				on_set(&show_grid);
				return true;
			}
			if (show_grid && (ke.get_key() == 'A') && (ke.get_modifiers() == 0)) {
				adaptive_grid = !adaptive_grid;
				on_set(&adaptive_grid);
				return true;
			}
			if (show_grid && (ke.get_key() == 'L') && (ke.get_modifiers() == 0)) {
				show_lines = !show_lines;
				on_set(&show_lines);
				return true;
			}
			if (show_grid && (ke.get_key() == 'X') && (ke.get_modifiers() == 0)) {
				show_axes = !show_axes;
				on_set(&show_axes);
				return true;
			}
		}
	}
	return false;
}

void grid::stream_help(std::ostream& os)
{
	os << "grid: show [G]rid, [L]ines, a[X]es, [A]daptive grid\n";
}

void grid::stream_stats(std::ostream& os)
{
	os << "grid: show" << (show_grid ? "*" : "") << ", lines" << (show_lines ? "*" : "") << ", axes" << (show_axes ? "*" : "") << ", adaptive" << (adaptive_grid ? "*" : "") << "\n";
}

/// overload to set the view
bool grid::init(cgv::render::context&)
{
	view_ptr = find_view_as_node();
	return true;
}

void grid::render_grid_lines(float alpha)
{
	glColor3f(1,1,1);
	alpha = (float)exp((alpha-1.0)/0.25);
	
	
	cgv::math::vec<float> v(4);
	glGetFloatv(GL_COLOR_CLEAR_VALUE,v);
	
	if(v.sub_vec(0,3).length() > 0.7)
		glColor4f(0.3f,0.3f,0.3f,alpha);
	else
		glColor4f(0.7f,0.7f,0.7f,alpha);
	
	int i;
	glBegin(GL_LINES);
	// Horizontal lines. 
	for (i=minZ; i<= maxZ; i++) 
	{
		if(i == 0)
			continue;
	  glVertex3i(minX,0, i);
	  glVertex3i(maxX,0, i);
	}
	// Vertical lines. 
	for (i=minX; i<=maxX; i++) 
	{
		if(i == 0)
			continue;
	  glVertex3i(i,0,minZ);
	  glVertex3i(i,0,maxZ);
	}
	glEnd();

	if(v.sub_vec(0,3).length() > 0.7)
		glColor4f(0.2f,0.2f,0.2f,alpha);
	else
		glColor4f(0.8f,0.8f,0.8f,alpha);

	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glVertex3i(minX,0, 0);
	glVertex3i(maxX,0, 0);
	glVertex3i(0,0,minZ);
	glVertex3i(0,0,maxZ);
	glEnd();
	glLineWidth(1.0f);
	
	

}

void grid::draw_grid(context &ctx)
{
	// transform coordinate frame to clip space
	dmat4 P;
	P(0, 0) = 1.0; P(0, 1) = 0.0; P(0, 2) = 0.0; P(0, 3) = 0.0;
	P(1, 0) = 0.0; P(1, 1) = 1.0; P(1, 2) = 0.0; P(1, 3) = 0.0;
	P(2, 0) = 0.0; P(2, 1) = 0.0; P(2, 2) = 1.0; P(2, 3) = 0.0;
	P(3, 0) = 1.0; P(3, 1) = 1.0; P(3, 2) = 1.0; P(3, 3) = 1.0;
	P = ctx.get_modelview_projection_device_matrix()*P;

	// compute clip space origin
	dvec3 l, O_clip = 1 / P(3, 3)*reinterpret_cast<const dvec3&>(P.col(3));
	
	// compute length of each coordinate axes in screen space and set z_axis
	for (unsigned i = 0; i < 3; ++i) {
		l(i) = (1 / P(3, i)*reinterpret_cast<const dvec3&>(P.col(i)) - O_clip).length();
		z_axis(i) = ((l(i) < threshold) ? 1.0f : 0.0f);
	}

	double v = cgv::math::maximum(l(0),l(1),l(2));
	factor = (float)pow(2.0, ceil(log(20.0) - log(v)) / log(2.0));

	cgv::render::shader_program& prog = ctx.ref_surface_shader_program();
	prog.enable(ctx);
	prog.set_uniform(ctx, "map_color_to_material", 3);
	ctx.set_color(rgb(0.5f, 0.5f, 0.5f));
	float r = arrow_length * arrow_aspect;
	ctx.push_modelview_matrix();
	ctx.mul_modelview_matrix(cgv::math::scale4<double>(r, r, r));
	ctx.tesselate_unit_sphere(40);
	ctx.pop_modelview_matrix();

	if (show_axes) {
		ctx.push_modelview_matrix();
		//z-axis
		if (fabs(fabs(z_axis(2)) - 1.0)> 0.01) {
			ctx.set_color(rgb(0, 0, 1));
			ctx.tesselate_arrow((double)arrow_length, (double)arrow_aspect, (double)arrow_rel_tip_radius, (double)arrow_tip_aspect);
		}
		//x-axis
		if (fabs(fabs(z_axis(0)) - 1.0)> 0.01) {
			ctx.set_color(rgb(1, 0, 0));
			ctx.mul_modelview_matrix(cgv::math::rotate4<double>(90, vec3(0, 1, 0)));
			ctx.tesselate_arrow((double)arrow_length, (double)arrow_aspect, (double)arrow_rel_tip_radius, (double)arrow_tip_aspect);
		}

		//y-axis
		if (fabs(fabs(z_axis(1)) - 1.0)> 0.01) {
			ctx.set_color(rgb(0, 1, 0));
			ctx.mul_modelview_matrix(cgv::math::rotate4<double>(-90, vec3(1, 0, 0)));
			ctx.tesselate_arrow((double)arrow_length, (double)arrow_aspect, (double)arrow_rel_tip_radius, (double)arrow_tip_aspect);
		}
		ctx.pop_modelview_matrix();
	}
	prog.disable(ctx);
}

void grid::draw(context &ctx)
{
	if (!show_grid)
		return;

	unsigned nr_cols, nr_rows;
	if (view_ptr && view_ptr->is_viewport_splitting_enabled(&nr_cols, &nr_rows)) {
		for (unsigned c = 0; c < nr_cols; ++c) {
			for (unsigned r = 0; r < nr_rows; ++r) {
				view_ptr->activate_split_viewport(ctx, c, r);
				draw_grid(ctx);
				view_ptr->deactivate_split_viewport(ctx);
			}
		}
	}
	else
		draw_grid(ctx);
}

void grid::finish_frame(context& ctx)
{
	if (!show_grid || !show_lines)
		return;

	unsigned nr_cols, nr_rows;
	if (view_ptr && view_ptr->is_viewport_splitting_enabled(&nr_cols, &nr_rows)) {
		for (unsigned c = 0; c < nr_cols; ++c) {
			for (unsigned r = 0; r < nr_rows; ++r) {
				view_ptr->activate_split_viewport(ctx, c, r);
				draw_lines(ctx);
				view_ptr->deactivate_split_viewport(ctx);
			}
		}
	}
	else
		draw_lines(ctx);
}

void grid::draw_lines(context& ctx)
{
	glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_LINE_BIT);
	GLboolean is_blend = glIsEnabled(GL_BLEND);
	GLint blend_src, blend_dst;
	glGetIntegerv(GL_BLEND_SRC_RGB, &blend_src);
	glGetIntegerv(GL_BLEND_DST_RGB, &blend_dst);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	cgv::render::shader_program& prog = ctx.ref_default_shader_program();
	prog.enable(ctx);

	ctx.set_color(rgba(0.5f, 0.5f, 0.5f, alpha));
	ctx.push_modelview_matrix();
	
		if(adaptive_grid)
			ctx.mul_modelview_matrix(cgv::math::scale4<double>(factor, factor, factor));

		ctx.push_modelview_matrix();
			render_grid_lines(fabs(z_axis(1)));
		
			ctx.mul_modelview_matrix(cgv::math::rotate4<double>(90, vec3(0, 0, 1)));
			render_grid_lines(fabs(z_axis(0)));

			ctx.mul_modelview_matrix(cgv::math::rotate4<double>(90, vec3(1, 0, 0)));
			render_grid_lines(fabs(z_axis(2)));
		ctx.pop_modelview_matrix();
	ctx.pop_modelview_matrix();

	prog.disable(ctx);

	glPopAttrib();
	if (!is_blend)
		glDisable(GL_BLEND);
	glBlendFunc(blend_src, blend_dst);
}

/// reflect adjustable members
bool grid::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return
		srh.reflect_member("show_axes", show_axes) &&
		srh.reflect_member("adaptive_grid", adaptive_grid) &&
		srh.reflect_member("show_grid", show_grid) &&
		srh.reflect_member("opacity", alpha) &&
		srh.reflect_member("show_lines", show_lines) &&
		srh.reflect_member("arrow_length", arrow_length) &&
		srh.reflect_member("arrow_aspect", arrow_aspect) &&
		srh.reflect_member("arrow_rel_tip_radius", arrow_rel_tip_radius) &&
		srh.reflect_member("arrow_tip_aspect", arrow_tip_aspect);
}

/// update gui and post redraw in on_set method
void grid::on_set(void* member_ptr)
{
	update_member(member_ptr);
	if (show_grid || member_ptr == &show_grid)
		post_redraw();
}

/// return a path in the main menu to select the gui
std::string grid::get_menu_path() const
{
	return "view/grid";
}
/// you must overload this for gui creation
void grid::create_gui()
{
	add_decorator("Grid", "heading");
	add_member_control(this, "grid", show_grid, "check");
	add_member_control(this, "grid_lines", show_lines, "check");
	add_member_control(this, "adaptive", adaptive_grid, "check");
	add_member_control(this, "threshold", threshold, "value_slider", "min=0;max=20;ticks=true;log=false");
	add_member_control(this, "opacity", alpha, "value_slider", "min=0;max=1;ticks=true");
	add_member_control(this, "axes", show_axes, "check");
	add_member_control(this, "arrow_length", arrow_length, "value_slider", "min=0;max=100;log=true;ticks=true");
	add_member_control(this, "arrow_aspect", arrow_aspect, "value_slider", "min=0.001;max=1;step=0.0005;log=true;ticks=true");
	add_member_control(this, "arrow_rel_tip_radius", arrow_rel_tip_radius, "value_slider", "min=1;max=10;log=true;ticks=true");
	add_member_control(this, "arrow_tip_aspect", arrow_tip_aspect, "value_slider", "min=0.1;max=10;log=true;ticks=true");
}
