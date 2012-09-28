#include "fltk_dragger.h"
#include <fltk/Cursor.h>
#include <fltk/Group.h>
#include <fltk/events.h>
#include <iostream>
fltk_dragger::fltk_dragger(int x, int h) : x_ctrl(x), fltk::Widget(x-4,0,8,h)
{
}

void fltk_dragger::draw()
{
	return;
}

void fltk_dragger::drag(int dx)
{
	if (x_ctrl + dx < 4)
		dx = 4-x_ctrl;
	if (dx == 0)
		return;
	fltk::Group* g = parent();
	int max_w = 50;
	int n = g->children();
	for (int i=0; i<n; ++i) {
		fltk::Widget* w = g->child(i);
		w->position(w->x()+dx,w->y());
		if (w->x()+w->w() > max_w)
			max_w = w->x()+w->w();
	}
	x_ctrl += dx;
//	std::cout << "dragged by " << dx << " to " << x_ctrl << std::endl;
	if (max_w != g->w())
		g->resize(max_w,g->h());
	g->redraw();
	g->layout();
	g->init_sizes();
}

int fltk_dragger::handle(int event) 
{
	switch (event) {
	case fltk::ENTER :
	case fltk::MOVE :
		cursor(fltk::CURSOR_WE);
		return 1;
	case fltk::LEAVE :
	case fltk::RELEASE :
		cursor(fltk::CURSOR_DEFAULT);
		return 1;
	case fltk::PUSH :
		last_x = fltk::event_x();
		cursor(fltk::CURSOR_WE);
		return 1;
	case fltk::DRAG :
		drag(fltk::event_x()-last_x);
		cursor(fltk::CURSOR_WE);
		return 1;
	}
	return 0;
}
