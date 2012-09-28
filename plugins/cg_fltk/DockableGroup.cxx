#include "DockableGroup.h"
#include <algorithm>

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/draw.h>
#include <fltk/layout.h>
#include <fltk/Box.h>
#include <fltk/events.h>
#include <fltk/Cursor.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif
#define abs(x) ((x)>=0?(x):-(x))
#define pos(x) ((x)<0?0:(x))

#define flag_loc 0x02000000
#define res_loc  0x08000000

using namespace fltk;


bool is_resizable(const Widget* widget)
{
	return (widget->flags() & res_loc) != 0;
}

void set_resizable(Widget* widget, bool v = true)
{
	if (v)
		widget->flags(widget->flags() | res_loc);
	else
		widget->flags(widget->flags() & ~res_loc);
}

int get_side(const Widget* widget)
{
	return (widget->flags()&(flag_loc*3))/flag_loc;
}

void set_side(Widget* widget, int s)
{
	return widget->flags( (widget->flags() & ~(3*flag_loc)) | s*flag_loc );
}

bool is_horizontal(int s)
{
	return (s&1) == 0;
}

DockableGroup::DockableGroup(int x, int y, int w, int h, const char *l, bool begin) : fltk::Group(x, y, w, h, l, begin) 
{
  resizable(0);
  spacing_ = 5;
  drag_widget = 0;
  margin(0);
}

DockableGroup::~DockableGroup()
{
	delete drag_widget;
}

void DockableGroup::draw()
{
	fltk::Group::draw();
}


void DockableGroup::dock(fltk::Widget* w, int side, bool resizable)
{
	set_side(w,side);
	set_resizable(w,resizable);

	w->set_visible();
	std::vector<fltk::Widget*>::iterator iter;
	while ((iter = std::find(dock_order.begin(),dock_order.end(),w)) != dock_order.end())
		dock_order.erase(iter);
	dock_order.push_back(w);
	init_sizes();
	redraw();
}

void DockableGroup::undock(fltk::Widget* w)
{
	std::vector<fltk::Widget*>::iterator iter;
	while ((iter = std::find(dock_order.begin(),dock_order.end(),w)) != dock_order.end())
		dock_order.erase(iter);
	w->clear_visible();
	init_sizes();
	redraw();
}


void DockableGroup::layout()
{
	// check whether all children are visible
	for (int ci=0; ci<children(); ++ci) {
		if (child(ci) == resizable())
			continue;
		if (std::find(dock_order.begin(),dock_order.end(),child(ci)) == dock_order.end())
			child(ci)->clear_visible();
	}

	if (!layout_damage()) 
		return;

	// we only need to do something special if the group is resized:
	if (!(layout_damage() & (LAYOUT_WH|LAYOUT_DAMAGE)) || !children()) {
	  Group::layout();
	  if (!(layout_damage() & LAYOUT_CHILD)) 
		  return;
	}
	int extradamage = layout_damage()&LAYOUT_DAMAGE;

	// clear the layout flags, so any resizes of children will set them again:
	Widget::layout();

	// This is the rectangle to lay out the remaining widgets in:
	Rectangle r(w(),h());
	box()->inset(r);

	// Apply margins
	r.set_x(r.x()+margin_left_);
	r.set_r(r.r()-margin_right_);
	r.set_y(r.y()+margin_top_);
	r.set_b(r.b()-margin_bottom_);

	for (unsigned int i=0; i<dock_order.size(); ++i) {
		// clean up children that where removed
		if (find(dock_order[i]) == children()) {
			dock_order.erase(dock_order.begin()+i);
			--i;
			continue;
		}
		// ignore resizable and invisible widgets
		Widget* widget = dock_order[i];
		if (widget->contains(resizable())) 
				break;
		if (!widget->visible()) 
			continue;
		// layout docked widgets
		switch (get_side(widget)) {
		case 0:
			widget->resize(r.r()-widget->w(), r.y(), widget->w(), r.h());
			widget->layout_damage(widget->layout_damage()|extradamage);
			widget->layout();
			r.move_r(-widget->w()-spacing_);
			break;
		case 1:
			widget->resize(r.x(), r.y(), r.w(), widget->h());
			widget->layout_damage(widget->layout_damage()|extradamage);
			widget->layout();
			r.move_y(widget->h()+spacing_);
			break;
		case 2:
			widget->resize(r.x(), r.y(), widget->w(), r.h());
			widget->layout_damage(widget->layout_damage()|extradamage);
			widget->layout();
			r.move_x(widget->w()+spacing_);
			break;
		case 3:
			widget->resize(r.x(), r.b()-widget->h(), r.w(), widget->h());
			widget->layout_damage(widget->layout_damage()|extradamage);
			widget->layout();
			r.move_b(-widget->h()-spacing_);
			break;
		}
	}
	// layout resizable widget
	Widget* widget = resizable();
	if (widget) {
		widget->resize(r.x(), r.y(), r.w(), r.h());
		widget->layout_damage(widget->layout_damage()|extradamage);
		widget->layout();
	}
	// A non-resizable widget will become the size of its items:
	int W = w();
	int H = h();
	if (r.w() < 0)
	  W -= r.w();    
	if (r.h() < 0)
	  H -= r.h();
	Widget::resize(W,H);
}

int DockableGroup::handle(int event) 
{
	int mx = event_x();
	int my = event_y();

	switch (event) {
	case MOVE:
	case ENTER:
	case PUSH: {
			drag_widget = 0;
			for (unsigned int i=0; i<dock_order.size(); ++i) {
				if (!is_resizable(dock_order[i]))
					continue;
				// ignore resizable and invisible widgets
				Widget* widget = dock_order[i];
				// layout docked widgets
				switch (get_side(widget)) {
				case 0:
					if (my >= widget->y() && my <= widget->b() && abs(mx - widget->x()) < 3)
						drag_widget = widget;
					break;
				case 1:
					if (mx >= widget->x() && mx <= widget->r() && abs(my - widget->b()) < 3)
						drag_widget = widget;
					break;
				case 2:
					if (my >= widget->y() && my <= widget->b() && abs(mx - widget->r()) < 3)
						drag_widget = widget;
					break;
				case 3:
					if (mx >= widget->x() && mx <= widget->r() && abs(my - widget->y()) < 3)
						drag_widget = widget;
					break;
				}
				if (drag_widget) {
					cursor(is_horizontal(get_side(widget)) ? CURSOR_WE : CURSOR_NS);
					last_x = mx;
					last_y = my;
					return 1;
				}
			}
			cursor(0);
			return Group::handle(event);
		}
	case DRAG:
	case RELEASE: {
			if (!drag_widget)
				return 0;
			int dx = mx-last_x;
			int dy = my-last_y;

			switch (get_side(drag_widget)) {
			case 0 : 
				if (!resizable() || resizable()->w()+dx >= 20)
					drag_widget->w(pos(drag_widget->w()-dx)); 
				break;
			case 1 :
				if (!resizable() || resizable()->h()-dy >= 20)
					drag_widget->h(pos(drag_widget->h()+dy)); 
				break;
			case 2 : 
				if (!resizable() || resizable()->w()-dx >= 20)
					drag_widget->w(pos(drag_widget->w()+dx)); 
				break;
			case 3 : 
				if (!resizable() || resizable()->h()+dy >= 20)
					drag_widget->h(pos(drag_widget->h()-dy)); 
				break;
			}
			last_x = mx;
			last_y = my;
			init_sizes();
			return 1;
		}
	}

	return Group::handle(event);
}

