#include "grid.h"
#include <cgv/gui/key_event.h>
#include <cgv/base/find_action.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/trigger.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/render/view.h>
#include <cgv/math/det.h>
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

	show_grid=true;
	factor=0;
	adaptive_grid=false;
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
			if (ke.get_key() == 'G' && ke.get_modifiers() == 0) {
				show_grid=!show_grid;
				post_redraw();
				return true;
			}
			if (show_grid && ke.get_key() == 'A' && ke.get_modifiers() == 0) {
				adaptive_grid=!adaptive_grid;
				post_redraw();
				return true;
			}
			
		}
	}
	return false;
}

void grid::stream_help(std::ostream& os)
{
	os << "grid: toggle [g]rid, toggle [a]daptive\n";
}

void grid::stream_stats(std::ostream& os)
{
	//cgv::utils::oprintf(os, "%s: angle = %.1f<left arrow/right arrow>\n", get_name(), a);
}

/// overload to set the view
bool grid::init(cgv::render::context&)
{
	return true;
}
/// 
void grid::init_frame(cgv::render::context& ctx)
{
	// find the view by traversing the base hierarchy, we assume there is only one
	std::vector<cgv::render::view*> views;
	cgv::base::find_interface(base_ptr(this), views);
	if (views.size() == 0)
		abort();
	view_ptr = views[0];
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
	using namespace cgv::math;

	zaxis = normalize(ctx.get_V().row(2).sub_vec(0, 3));

	mat<double> p(4, 4);

	p(0, 0) = 1.0; p(0, 1) = 0.0; p(0, 2) = 0.0; p(0, 3) = 0.0;
	p(1, 0) = 0.0; p(1, 1) = 1.0; p(1, 2) = 0.0; p(1, 3) = 0.0;
	p(2, 0) = 0.0; p(2, 1) = 0.0; p(2, 2) = 1.0; p(2, 3) = 0.0;
	p(3, 0) = 1.0; p(3, 1) = 1.0; p(3, 2) = 1.0; p(3, 3) = 1.0;

	p = ctx.get_DPV()*p;
	cgv::math::unhomog(p);

	double v = cgv::math::maximum(length(p.col(0) - p.col(3)), length(p.col(1) - p.col(3)), length(p.col(2) - p.col(3)));
	//std::cout << v <<":"<< pow(2.0,ceil(log(40.0)-log(v))/log(2.0)) << std::endl;
	factor = (float)pow(2.0, ceil(log(20.0) - log(v)) / log(2.0));

	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColor3f(0.5f, 0.5f, 0.5f);
	float r = arrow_length * arrow_aspect;
	ctx.push_V();
	glScalef(r, r, r);
	ctx.tesselate_unit_sphere(40);
	ctx.pop_V();

	if (show_axes) {
		ctx.push_V();
		//z-axis
		if (fabs(fabs(zaxis(2)) - 1.0)> 0.001)
		{
			glColor3f(0, 0, 1);
			ctx.tesselate_arrow((double)arrow_length, (double)arrow_aspect, (double)arrow_rel_tip_radius, (double)arrow_tip_aspect);
		}
		//x-axis
		if (fabs(fabs(zaxis(0)) - 1.0)> 0.001)
		{
			glRotatef(90, 0, 1, 0);
			glColor3f(1, 0, 0);
			ctx.tesselate_arrow((double)arrow_length, (double)arrow_aspect, (double)arrow_rel_tip_radius, (double)arrow_tip_aspect);
		}

		//y-axis
		if (fabs(fabs(zaxis(1)) - 1.0)> 0.001)
		{

			glRotatef(-90, 1, 0, 0);
			glColor4f(0, 1, 0, 1);
			ctx.tesselate_arrow((double)arrow_length, (double)arrow_aspect, (double)arrow_rel_tip_radius, (double)arrow_tip_aspect);
		}
		ctx.pop_V();
	}
	glPopAttrib();
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
				view_ptr->deactivate_split_viewport();
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
				view_ptr->deactivate_split_viewport();
			}
		}
	}
	else
		draw_lines(ctx);
}

void grid::draw_lines(context& ctx)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT | GL_LINE_BIT);
	using namespace cgv::math;
	glEnable (GL_BLEND); 
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
		
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glPushMatrix();
	if(adaptive_grid)
		glScaled(factor,factor,factor);
	{
		glPushMatrix();
		::glRotated(90,0,0,1);
		render_grid_lines(fabs(zaxis(0)));
		glPopMatrix();
	}


	render_grid_lines(fabs(zaxis(1)));

	{
		glPushMatrix();
		::glRotated(90,0,0,1);
		::glRotated(90,1,0,0);
		render_grid_lines(fabs(zaxis(2)));
		glPopMatrix();
	}
	glPopMatrix();
	
	glPopAttrib();
}

/// reflect adjustable members
bool grid::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return
	srh.reflect_member("show_axes", show_axes) &&
	srh.reflect_member("adaptive_grid", adaptive_grid) &&
	srh.reflect_member("show_grid", show_grid) &&
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
	connect_copy(add_control("grid", show_grid, "check")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("grid_lines", show_lines, "check")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("adaptive", adaptive_grid, "check")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("axes", show_axes, "check")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("arrow_length", arrow_length, "value_slider", "min=0;max=100;log=true;ticks=true")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("arrow_aspect", arrow_aspect, "value_slider", "min=0.001;max=1;step=0.0005;log=true;ticks=true")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("arrow_rel_tip_radius", arrow_rel_tip_radius, "value_slider", "min=1;max=10;log=true;ticks=true")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("arrow_tip_aspect", arrow_tip_aspect, "value_slider", "min=0.1;max=10;log=true;ticks=true")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
}
