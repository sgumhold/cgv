#pragma once

#include "picker.h"
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>

using namespace cgv::gui;
using namespace cgv::render;

/// construct from name which is necessary construction argument to node
picker::picker(const char* name) : node(name)
{
	drag_pnt_idx = -1;
	is_drag_action = false;
}

/// show internal values
void picker::stream_stats(std::ostream& os)
{
	os << "nr pnts: " << pnts.size() << ", drag_pnt_idx: " << drag_pnt_idx << std::endl;
}

/// check if a world point is close enough to the drawing square
bool picker::is_inside(const Pnt& p) const
{
	return fabs(p(2)) < 0.1 && fabs(p(0)) <= 1 && fabs(p(1)) <= 1;
}

/// transform from 3d world to 2d parametric space
picker::Pnt picker::transform_2_local(const Pnt& p3d) const
{
	return Pnt(p3d(0),p3d(1));
}
/// find closest point and return index or -1 if we do not have any points yet
int picker::find_closest(const Pnt& p2d) const
{
	if (pnts.empty())
		return -1;
	double min_dist = length(pnts[0]-p2d);
	int res = 0;
	for (int i = 1; i < (int)pnts.size(); ++i) {
		if (length(pnts[i]-p2d) < min_dist) {
			min_dist = length(pnts[i]-p2d);
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
				Pnt p = get_context()->get_point_W(me.get_x(), me.get_y(), DPV);
				if (is_inside(p)) {
					Pnt q = transform_2_local(p);
					drag_pnt_idx = -1;
					int i = find_closest(q);
					if (i != -1) {
						if (length(pnts[i]-q) < 0.03)
							drag_pnt_idx = i;
					}
					is_drag_action = drag_pnt_idx == -1;
					if (is_drag_action) {
						pnts.push_back(q);
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
				Pnt p = get_context()->get_point_W(me.get_x(), me.get_y(), DPV);
				if (!is_inside(p)) {
					pnts.erase(pnts.begin()+drag_pnt_idx);
					drag_pnt_idx = -1;
				}
				else {
					is_drag_action = true;
					pnts[drag_pnt_idx] = transform_2_local(p);
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

/// optional method of drawable
void picker::draw(context& ctx)
{
	glDisable(GL_LIGHTING);
	glColor3d(1,1,0.7);
	DPV = ctx.get_DPV();
	glPolygonOffset(1,1);
	glDisable(GL_CULL_FACE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glBegin(GL_QUADS);
	glVertex2d(-1,-1);
	glVertex2d(1,-1);
	glVertex2d(1,1);
	glVertex2d(-1,1);
	glEnd();
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glColor3d(1,0,0);
	glPointSize(5);
	glPolygonOffset(-1,-1);
	glEnable(GL_POLYGON_OFFSET_POINT);
	glEnable(GL_POINT_SMOOTH);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i<pnts.size(); ++i)
		glVertex2d(pnts[i](0),pnts[i](1));
	glEnd();
	glDisable(GL_POLYGON_OFFSET_POINT);
	glEnable(GL_LIGHTING);
}

#include <cgv/base/register.h>

/// register a factory to create new cubes
extern cgv::base::factory_registration_1<picker,const char*> picker_fac("new/picker", 'P', "point picker", true);
//extern cgv::base::object_registration_1<picker,const char*> picker_reg("point picker", "");
