#pragma once

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Widget.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif

#include "lib_begin.h"

struct CGV_API fltk_dragger : public fltk::Widget
{
	int x_ctrl;
	int last_x;
	/// construct from initial x-position and height
	fltk_dragger(int x, int h);
	/// overload to not draw anything
	void draw();
	/// the drag method repositions all child widgets of the the parent group
	void drag(int dx);
	/// handle mouse events to realize dragging
	int handle(int event);
};

#include <cgv/config/lib_end.h>
