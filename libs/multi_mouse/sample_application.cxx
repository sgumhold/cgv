#include "sample_application.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <cmath>

using namespace std;
using namespace cgv::gui;

point::point(int _x, int _y) : x(_x), y(_y) 
{
}

line::line(int x0, int y0, int x1, int y1) : p0(x0,y0), p1(x1,y1) 
{
}

void drawing::add_point(int x, int y)
{
	points.push_back(point(x,y)); 
}

void drawing::add_line(int x, int y, int dx, int dy) 
{ 
	if (dx != 0 || dy != 0) 
		lines.push_back(line(x-dx,y-dy,x,y)); 
}

void drawing::render() const
{
	glLineWidth(1);
	glBegin(GL_LINES);
	for (unsigned l=0; l<lines.size(); ++l) {
		glVertex2i(lines[l].p0.x, lines[l].p0.y);
		glVertex2i(lines[l].p1.x, lines[l].p1.y);
	}
	glEnd();
	if (points.size() > 0) {
		glPointSize(5);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_POINT_SMOOTH);
		glBegin(GL_POINTS);
		for (unsigned p=0; p<points.size(); ++p)
			glVertex2i(points[p].x, points[p].y);
		glEnd();
	}
}

void sample_application::init_mice(int w, int h)
{
	if (mice.empty())
		scan_mouse_devices(mice);
	int r = 20;
	for (unsigned i=0; i<mice.size(); ++i) {
		double a = 2*3.14*i/mice.size();
		set_mouse_rectangle(mice[i].id,0,w,0,h);
		set_mouse_position(mice[i].id,(int)(w/2+r*cos(a)),(int)(h/2+r*sin(a)));
	}
}

void sample_application::process_mouse_change_event(bool attach, void* id, int w, int h)
{
	if (attach)
		cout << "attached " << id << endl;
	else
		cout << "detached " << id << endl;
	mice.clear();
	mice_drawings.clear();
	init_mice(w, h);
}

bool sample_application::handle_mouse_event(const multi_mouse_event& mme)
{
	switch (mme.get_action()) {
	case MA_PRESS:
		mice_drawings[mme.get_id()].add_point(mme.get_x(),mme.get_y());
		break;
	case MA_DRAG:
		mice_drawings[mme.get_id()].add_line(mme.get_x(),mme.get_y(),mme.get_dx(),mme.get_dy());
		break;
	default:
		return false;
	}
	return true;
}

sample_application::sample_application()
{
}

void sample_application::draw_scene(double time)
{
	// used rgb colors for different mice
	static GLfloat colors[] = {	1,1,1, 1,1,0, 0,1,1, 1,0,1, 1,0,0, 0,1,0, 0,0,1 };

	// create default mouse pointer
	if (!c.is_created())
		c.create();

	// draw drawings in different colors
	unsigned i;
	for (i=0; i<mice.size(); ++i) {
		glColor3fv(colors+3*(i%7));
		mice_drawings[mice[i].id].render();
	}
	// draw mouse pointers in different colors
	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	for (i=0; i<mice.size(); ++i) {
		int x,y;
		get_mouse_position(mice[i].id, x, y);
		glColor3fv(colors+3*(i%7));
		c.draw(x,y,true,c.get_step_frame(c.find_step_index(time)));
	}
	glPopAttrib();
}
