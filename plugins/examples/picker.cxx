#include "picker.h"
#include <cgv/gui/key_event.h>
#include <cgv/math/ftransform.h>
#include <cgv/gui/mouse_event.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv/render/shader_program.h>

using namespace cgv::gui;
using namespace cgv::render;

/// construct from name which is necessary construction argument to node
picker::picker(const char* name) : node(name)
{
	drag_pnt_idx = -1;
	is_drag_action = false;
	srs.radius = 0.02f;
	srs.map_color_to_material = cgv::render::CM_COLOR;
	srs.surface_color = rgb(1, 0, 0);
	mouse_over_panel = false;
	n = 3;
	m = 2;
}

/// show internal values
void picker::stream_stats(std::ostream& os)
{
	os << "nr pnts: " << pnts.size() << ", drag_pnt_idx: " << drag_pnt_idx << std::endl;
}

/// check if a world point is close enough to the drawing square
bool picker::is_inside(const vec3& p) const
{
	float bound = 1.0f + 1.2f*srs.radius*srs.radius_scale;
	return fabs(p(2)) <= bound && fabs(p(0)) <= bound && fabs(p(1)) <= bound;
}

picker::vec3 picker::project(const vec3& p3d) const
{
	vec3 p = p3d;
	for (unsigned i = 0; i < 3; ++i) {
		if (p(i) < -1)
			p(i) = -1;
		if (p(i) > 1)
			p(i) = 1;
	}
	return p;
}

/// find closest point and return index or -1 if we do not have any points yet
int picker::find_closest(const vec3& p3d) const
{
	if (pnts.empty())
		return -1;
	double min_dist = length(pnts[0]-p3d);
	int res = 0;
	for (int i = 1; i < (int)pnts.size(); ++i) {
		if (length(pnts[i]-p3d) < min_dist) {
			min_dist = length(pnts[i]-p3d);
			res = i;
		}
	}
	return res;
}

/// necessary method of event_handler
bool picker::handle(event& e)
{
	if (e.get_kind() == EID_MOUSE) {
		mouse_event& me = static_cast<mouse_event&>(e);
		const dmat4* MPW_ptr = 0;

		if (me.get_action() == MA_LEAVE) {
			if (mouse_over_panel) {
				mouse_over_panel = false;
				post_redraw();
			}
		}
		else {
			view_ptr->get_modelview_projection_window_matrices(me.get_x(), me.get_y(),
				get_context()->get_width(), get_context()->get_height(),
				&MPW_ptr, 0, 0, 0, &col_idx, &row_idx);
			mouse_over_panel = true;
			post_redraw();
		}
		switch (me.get_action()) {
		case MA_PRESS :
			if (me.get_button() == MB_LEFT_BUTTON && me.get_modifiers() == EM_CTRL) {
				vec3 p = get_context()->get_model_point(me.get_x(), me.get_y(), *MPW_ptr);
				if (is_inside(p)) {
					p = project(p);
					drag_pnt_idx = -1;
					int i = find_closest(p);
					if (i != -1) {
						if (length(pnts[i]-p) < 1.2f*srs.radius*srs.radius_scale)
							drag_pnt_idx = i;
					}
					is_drag_action = drag_pnt_idx == -1;
					if (is_drag_action) {
						pnts.push_back(p);
						clrs.push_back(rgb((const rgb&)(0.5f*(p + 1.0f))));
						drag_pnt_idx = (int)pnts.size()-1;
					}
					post_redraw();
					return true;
				}
			}
			break;
		case MA_RELEASE :
			if (drag_pnt_idx != -1) {
				if (!is_drag_action) {
					pnts.erase(pnts.begin() + drag_pnt_idx);
					clrs.erase(clrs.begin() + drag_pnt_idx);
				}
				drag_pnt_idx = -1;
				post_redraw();
				return true;
			}
			break;
		case MA_DRAG :
			if (drag_pnt_idx != -1) {
				vec3 p = get_context()->get_model_point(me.get_x(), me.get_y(), *MPW_ptr);
				if (!is_inside(p)) {
					pnts.erase(pnts.begin()+drag_pnt_idx);
					clrs.erase(clrs.begin() + drag_pnt_idx);
					drag_pnt_idx = -1;
				}
				else {
					is_drag_action = true;
					pnts[drag_pnt_idx] = project(p);
				}
				post_redraw();
				return true;
			}
			break;
		}
	}
	return false;
}

/// necessary method of event_handler
void picker::stream_help(std::ostream& os)
{
	os << "Picker: add, move and remove points on square with Ctrl+Left Mouse Button\n";
}

#include <cgv_gl/gl/gl.h>

/// init renderer
bool picker::init(cgv::render::context& ctx)
{
	view_ptr = find_view_as_node();
	view_ptr->enable_viewport_splitting(n, m);
	for (int i = 0; i < (int)n; ++i)
		for (int j = 0; j < (int)m; ++j) {
			view_ptr->enable_viewport_individual_view(i, j);
			auto& view = view_ptr->ref_viewport_view(i, j);
			view.set_focus(0, 0, 0);
			vec3 view_dir((float)i - 1, 0, 2.0f * (j - 0.5f));
			view.set_view_dir(view_dir);
			view.set_view_up_dir(0, 1, 0);
		}
	ref_sphere_renderer(ctx, 1);
	return true;
}

/// init renderer
void picker::destruct(cgv::render::context& ctx)
{
	view_ptr->disable_viewport_splitting();
	ref_sphere_renderer(ctx, -1);
}

/// optional method of drawable
void picker::draw(context& ctx)
{
	for (unsigned i = 0; i < n; ++i) {
		for (unsigned j = 0; j < m; ++j) {
			view_ptr->activate_split_viewport(ctx, i, j);

			if (mouse_over_panel && col_idx == i && row_idx == j) {
				glDepthMask(GL_FALSE);
				auto& prog = ctx.ref_default_shader_program();
				prog.enable(ctx);
				ctx.set_color(rgb(1, 0.6f, 0.6f));
				cgv::render::gl::cover_screen(ctx, &prog);
				prog.disable(ctx);
				glDepthMask(GL_TRUE);
			}

			auto& prog = ctx.ref_surface_shader_program();
			prog.set_uniform(ctx, "map_color_to_material", 3);
			prog.enable(ctx);
			ctx.set_color(rgb(0.7f, 0.5f, 0.4f));
			ctx.tesselate_unit_cube();
			prog.disable(ctx);

			if (!pnts.empty()) {

				auto& sr = ref_sphere_renderer(ctx);
				sr.set_render_style(srs);
				sr.set_position_array(ctx, pnts);
				sr.set_color_array(ctx, clrs);
				sr.set_y_view_angle(float(view_ptr->get_y_view_angle()));
				sr.render(ctx, 0, pnts.size());
			}
			view_ptr->deactivate_split_viewport(ctx);
		}
	}
}

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration_1<picker,const char*> picker_fac("new/events/picker", 'P', "point picker", true);
//cgv::base::object_registration_1<picker,const char*> picker_reg("point picker", "");
