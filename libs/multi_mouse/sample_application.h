#include "multi_mouse.h"
#include <map>
#include <vector>
#include <cgv_gl/gl/gl_cursor.h>

/// simple point structure
struct point
{
	int x,y;
	point(int _x=0, int _y=0);
};

/// simple line segment structure
struct line
{
	point p0;
	point p1;
	line(int x0 = 0, int y0 = 0, int x1 = 0, int y1 = 0);
};

/// a drawing is composed with points and lines
struct drawing
{
	std::vector<point> points;
	std::vector<line> lines;
	void add_point(int x, int y);
	void add_line(int x, int y, int dx, int dy);
	void render() const;
};

/// sample aplpication with functionality independent of used gui library
struct sample_application
{
	/// used to draw multiple mouse pointers in opengl
	cgv::render::gl::gl_cursor c;
	/// store mouse device information
	std::vector<cgv::gui::mouse_info> mice;
	/// store per mouse device a drawing
	std::map<void*,drawing> mice_drawings;
	/// standard construction does nothing
	sample_application();
	/// initialize the mouse positions on circle around window center
	void init_mice(int w, int h);
	/// process mouse device change events
	void process_mouse_change_event(bool attach, void* id, int w, int h);
	/// process multi mouse events
	bool handle_mouse_event(const cgv::gui::multi_mouse_event& mme);
	/// draw the drawings and mouse pointers in different colors
	void draw_scene(double time = 0);
};