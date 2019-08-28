#include "picker.h"
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/render/shader_program.h>

using namespace cgv::gui;
using namespace cgv::render;

/// construct from name which is necessary construction argument to node
picker::picker(const char* name) : node(name)
{
	drag_pnt_idx = -1;
	is_drag_action = false;
	srs.radius = 0.02f;
	srs.map_color_to_material = cgv::render::ColorMapping(cgv::render::CM_COLOR_FRONT + cgv::render::CM_COLOR_BACK);
	srs.surface_color = rgb(1, 0, 0);
	pnts.push_back(vec3(0.0f));
	pnts.push_back(vec3(0.5f));
	pnts.push_back(vec3(-0.5f));
}

/// show internal values
void picker::stream_stats(std::ostream& os)
{
	os << "nr pnts: " << pnts.size() << ", drag_pnt_idx: " << drag_pnt_idx << std::endl;
}

/// check if a world point is close enough to the drawing square
bool picker::is_inside(const vec3& p) const
{
	return fabs(p(2)) <= 1.001f && fabs(p(0)) <= 1.001f && fabs(p(1)) <= 1.001f;
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
		switch (me.get_action()) {
		case MA_PRESS :
			if (me.get_button() == MB_LEFT_BUTTON && me.get_modifiers() == EM_CTRL) {
				vec3 p = get_context()->get_model_point(me.get_x(), me.get_y(), MPW);
				if (is_inside(p)) {
					std::cout << p << std::endl;
					drag_pnt_idx = -1;
					int i = find_closest(p);
					if (i != -1) {
						if (length(pnts[i]-p) < 0.03f)
							drag_pnt_idx = i;
					}
					is_drag_action = drag_pnt_idx == -1;
					if (is_drag_action) {
						pnts.push_back(p);
						drag_pnt_idx = (int)pnts.size()-1;
					}
					post_redraw();
					return true;
				}
			}
			break;
		case MA_RELEASE :
			if (drag_pnt_idx != -1) {
				if (!is_drag_action)
					pnts.erase(pnts.begin()+drag_pnt_idx);
				drag_pnt_idx = -1;
				post_redraw();
				return true;
			}
			break;
		case MA_DRAG :
			if (drag_pnt_idx != -1) {
				vec3 p = get_context()->get_model_point(me.get_x(), me.get_y(), MPW);
				if (!is_inside(p)) {
					pnts.erase(pnts.begin()+drag_pnt_idx);
					drag_pnt_idx = -1;
				}
				else {
					is_drag_action = true;
					pnts[drag_pnt_idx] = p;
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
	ref_sphere_renderer(ctx, 1);
	return true;
}

/// init renderer
void picker::destruct(cgv::render::context& ctx)
{
	ref_sphere_renderer(ctx, -1);
}

/// optional method of drawable
void picker::draw(context& ctx)
{
	MPW = ctx.get_modelview_projection_window_matrix();

	auto& prog = ctx.ref_surface_shader_program();
	prog.set_uniform(ctx, "map_color_to_material", 3);
	prog.enable(ctx);
	ctx.set_color(rgb(0.7f,0.5f,0.4f));
	ctx.tesselate_unit_cube();
	prog.disable(ctx);

	if (pnts.empty())
		return;

	glDepthMask(GL_FALSE);
	auto& sr = ref_sphere_renderer(ctx);
	sr.set_render_style(srs);
	sr.set_position_array(ctx, pnts);
	sr.set_y_view_angle(float(view_ptr->get_y_view_angle()));
	if (sr.validate_and_enable(ctx)) {
		ctx.set_color(rgb(1.0f, 0, 0));
		glDrawArrays(GL_POINTS, 0, GLsizei(pnts.size()));
		sr.disable(ctx);
	}
	glDepthMask(GL_TRUE);
}

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration_1<picker,const char*> picker_fac("new/events/picker", 'P', "point picker", true);
//cgv::base::object_registration_1<picker,const char*> picker_reg("point picker", "");
