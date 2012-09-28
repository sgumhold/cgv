#include "fltk_align_group.h"

#include <cgv/utils/scan.h>
#include <cgv/gui/provider.h>
#include <stdlib.h>

using namespace cgv::base;
using namespace cgv::gui;

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Group.h>
#include <fltk/draw.h>
#include <fltk/Cursor.h>
#include <fltk/layout.h>
#include <fltk/events.h>
#include <fltk/Scrollbar.h>
#include <fltk/PackedGroup.h>
#include <fltk/Button.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif

namespace fltk {

class FL_API ScrollGroup : public Group {
protected:
  int xposition_, yposition_;
  int layoutdx, layoutdy;
  int scrolldx, scrolldy;
  bool enable_drag_scroll_;
  bool drag_scrolling_;
  bool delegate_alt_click_;
  int drag_x_, drag_y_, pos_x_, pos_y_;
  int max_x_scroll_, max_y_scroll_;
  static void hscrollbar_cb(Widget*, void*);
  static void scrollbar_cb(Widget*, void*);
  static void draw_clip(void*,const Rectangle&);

protected:

  void draw();

public:

  void bbox(Rectangle&);
  Scrollbar scrollbar;
  Scrollbar hscrollbar;

  void enable_drag_scroll( bool enable ) { enable_drag_scroll_ = true; }

  virtual int handle(int);
  virtual void layout();

  ScrollGroup(int x,int y,int w,int h, const char*l=0, bool begin=false);

  enum { // values for type()
    HORIZONTAL = 1,
    VERTICAL = 2,
    BOTH = 3,
    ALWAYS_ON = 4,
    HORIZONTAL_ALWAYS = 5,
    VERTICAL_ALWAYS = 6,
    BOTH_ALWAYS = 7
  };

  int xposition() const {return xposition_;}
  int yposition() const {return yposition_;}
  void scrollTo(int, int);
};

class MyScrollGroup : public ScrollGroup
{
	int last_x;
public:
	int offsetx, offsety;
	MyScrollGroup(int x,int y,int w,int h, const char*l=0, bool begin=false) : ScrollGroup(x,y,w,h,l,begin)
	{
		offsetx = offsety = 0;
	}
	void drag(int dx)
	{
		offsetx += dx;
		scrollTo(xposition()-dx, yposition());
	}

	bool inside()
	{
		if (children() == 0)
			return false;
		int dx = fltk::event_x() - child(0)->x() + 3;
		if (dx < 0) dx = -dx;
		return dx < 3;
	}
	int handle(int event) 
	{
		switch (event) {
		case fltk::ENTER :
		case fltk::MOVE :
		case fltk::RELEASE :
			cursor(inside() ? fltk::CURSOR_WE : fltk::CURSOR_DEFAULT);
			return fltk::ScrollGroup::handle(event);
		case fltk::LEAVE :
			fltk::ScrollGroup::handle(event);
			cursor(fltk::CURSOR_DEFAULT);
			return 1;
		case fltk::PUSH :
			if (inside()) {
				cursor(fltk::CURSOR_WE);
				last_x = fltk::event_x();
				return 1;
			}
			else
				return fltk::ScrollGroup::handle(event);
		case fltk::DRAG :
			drag(fltk::event_x()-last_x);
			cursor(fltk::CURSOR_WE);
			last_x = fltk::event_x();
			return 1;
		}
		return fltk::ScrollGroup::handle(event);
	}
	void layout() 
	{

	  int layout_damage = this->layout_damage();

	  // handle movement in xy without wasting time:
	  if (!(layout_damage&(LAYOUT_WH|LAYOUT_DAMAGE|LAYOUT_CHILD))) {
		Group::layout();
		return;
	  }

	  Rectangle rectangle; bbox(rectangle);

	  // This loop repeats if any scrollbars turn on or off:
	  for (int repeat=0; repeat<2; repeat++) {

		layout_damage &= ~LAYOUT_WH;
		if (!(type()&HORIZONTAL)) layout_damage |= LAYOUT_W;
		if (!(type()&VERTICAL)) layout_damage |= LAYOUT_H;
		Group::layout(rectangle, layout_damage);

		// move all the children and accumulate their bounding boxes:
		int dx = layoutdx;
		int dy = layoutdy;
		layoutdx = layoutdy = 0;
		scrolldx += dx;
		scrolldy += dy;
		int numchildren = children();
		int l = numchildren > 0 ? child(0)->x()-offsetx : w();
		int r = 0;
		int t = numchildren > 0 ? child(0)->y()-offsety : h();
		int b = 0;
		for (int i=0; i < numchildren; i++) {
		  Widget* o = child(i);
		  o->position(o->x()+dx, o->y()+dy);
		  o->layout();
		  if (o->x() < l) l = o->x();
		  if (o->y() < t) t = o->y();
		  if (o->x()+o->w() > r) r = o->x()+o->w();
		  if (o->y()+o->h() > b) b = o->y()+o->h();
		}

		// If there is empty space on the bottom (VERTICAL mode) or right
		// (HORIZONTAL mode), and widgets off to the top or left, move
		// widgets to use up the available space.
		int newDx = 0;
		int newDy = 0;
		if ( type() & VERTICAL ) {
		  if ( b < rectangle.b() ) {
		int space = rectangle.b()-b;
		int hidden = rectangle.y()-t;
		newDy = (space>hidden) ? hidden : space;
		newDy = (newDy>0) ? newDy : 0;
		  }
		}
		if ( type() & HORIZONTAL ) {
		  if ( r < rectangle.r() ) {
		int space = rectangle.r()-r;
		int hidden = rectangle.x()-l; // ell
		newDx = (space>hidden) ? hidden : space;
		newDx = (newDx>0) ? newDx : 0;
		  }
		}

		// Move the child widgets again if they are to be kept inside.
		if ( newDx || newDy ) {
		  for (int i=0; i < numchildren; i++) {
		Widget* o = child(i);
		o->position(o->x()+newDx, o->y()+newDy);
		o->layout();
		  }
		}

		// Turn on/off the scrollbars if it fits:
		if ((type() & VERTICAL) &&
		((type() & ALWAYS_ON) || t < rectangle.y() || b > rectangle.b())) {
		  // turn on the vertical scrollbar
		  if (!scrollbar.visible()) {
		scrollbar.set_visible();
		scrollbar.redraw();
		  }
		} else {
		  // turn off the vertical scrollbar
		  if (scrollbar.visible()) {
		scrollbar.clear_visible();
		  }
		}

		if ((type() & HORIZONTAL) &&
		((type() & ALWAYS_ON) || l < rectangle.x() || r > rectangle.r())) {
		  // turn on the horizontal scrollbar
		  if (!hscrollbar.visible()) {
		hscrollbar.set_visible();
		hscrollbar.redraw();
		  }
		} else {
		  // turn off the horizontal scrollbar
		  if (hscrollbar.visible()) {
		hscrollbar.clear_visible();
		  }
		}

		// fix the width of the scrollbars to current preferences:
		const int sw = scrollbar_width();
		scrollbar.w(sw);
		hscrollbar.h(sw);
		// Figure out the new contents rectangle and put scrollbars outside it:
		Rectangle R; bbox(R);

		scrollbar.resize(scrollbar_align()&ALIGN_LEFT ? R.x()-sw : R.r(), R.y(), sw, R.h());

		scrollbar.value(yposition_ = (R.y()-t), R.h(), 0, b-t);
		hscrollbar.resize(R.x(), scrollbar_align()&ALIGN_TOP ? R.y()-sw : R.b(), R.w(), sw);
		hscrollbar.value(xposition_ = (R.x()-l), R.w(), 0, r-l);
		max_x_scroll_ = r-l-R.w();
		max_y_scroll_ = b-t-R.h();

		// We are done if the new rectangle is the same as last time:
		if (R.x() == rectangle.x() &&
		R.y() == rectangle.y() &&
		R.w() == rectangle.w() &&
		R.h() == rectangle.h()) break;
		// otherwise layout again in this new rectangle:
		rectangle = R;
	  }

	}
};
}
fltk_align_group::fltk_align_group(int x, int y, int w, int h, const std::string& _name) : cgv::gui::gui_group(_name)
{
	init_aligment();
	scroll_group = new CG<fltk::MyScrollGroup>(x,y,w,h,get_name().c_str());
//	scroll_group->type(fltk::ScrollGroup::HORIZONTAL_ALWAYS);
	scroll_group->box(fltk::FLAT_BOX);
	scroll_group->user_data(static_cast<cgv::base::base*>(this));
//	scroll_group->type(fltk::ScrollGroup::VERTICAL);
//	scroll_group->begin();
//		inner_group = new PerLineResizeGroup(0,0,w,h,"");
//		inner_group = new fltk::Group(0,0,w,h,"");
//		inner_group->user_data(static_cast<cgv::base::base*>(this));
//	scroll_group->end();
//	scroll_group->resizable(inner_group);
}

/// delete fltk group realization
fltk_align_group::~fltk_align_group()
{
//	delete inner_group;
	delete scroll_group;
}


/// only uses the implementation of fltk_base
std::string fltk_align_group::get_property_declarations()
{
	return fltk_base::get_property_declarations();
}
/// abstract interface for the setter
bool fltk_align_group::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	return fltk_base::set_void(scroll_group, this, property, value_type, value_ptr);
}
/// abstract interface for the getter
bool fltk_align_group::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return fltk_base::get_void(scroll_group, this, property, value_type, value_ptr);
}

/// return a fltk::Widget pointer that can be cast into a fltk::Group
void* fltk_align_group::get_user_data() const
{
	return static_cast<fltk::Widget*>(scroll_group);
}


/// put default sizes into dimension fields and set inner_group to be active
void fltk_align_group::prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h)
{
	x = current_x;
	y = current_y;
	w = default_width;
	h = default_height;
	scroll_group->begin();
//	inner_group->begin();
}

void fltk_align_group::parse_variable_change(const std::string& align, unsigned int& i, int& var, int default_value)
{
	if (align.size() < i+2)
		return;
	++i;
	char op = align[i];
	switch (op) {
	case '+' :
	case '-' :
	case '*' :
	case '/' :
		if (align[i+1]!='=')
			return;
		++i;
		if (align.size() < i+1)
			return;
		break;
	}
	if (align[i+1] == '#') {
		++i;
		var = default_value;
		return;
	}
	unsigned int j;
	for (j=i; j<align.size()-1; ++j) {
		if (align[j+1] < '0' || align[j+1] > '9')
			break;
	}
	if (i == j)
		return;
	int new_val = atoi(align.substr(i+1, j-i).c_str());
	switch (op) {
	case '+' : var += new_val; break;
	case '-' : var -= new_val; break;
	case '*' : var *= new_val; break;
	case '/' : var /= new_val; break;
	default  : var  = new_val;
	}
	i = j;
}

/// initialize the members used for alignment
void fltk_align_group::init_aligment()
{
	x_offset = 100;
	y_offset = 10;
	x_spacing = 12;
	y_spacing = 8;

	current_x = x_offset;
	current_y = y_offset;
	current_line_height = 0;

	default_width = 200;
	default_height = 20;
	tab_size = 50;
}

/// overload to trigger initialization of alignment
void fltk_align_group::remove_all_children(cgv::gui::gui_group_ptr ggp)
{
	init_aligment();
	fltk_gui_group::remove_all_children(ggp);
}


void fltk_align_group::align(const std::string& _align)
{
	// compute next widget location
	if (_align.size() > 0) {
		for (unsigned int i=0; i<_align.size(); ++i) {
			switch(_align[i]) {
			case ' ' :
				current_x += x_spacing;
				break;
			case '\n' :
				current_x  = x_offset;
				current_y += current_line_height + y_spacing;
				current_line_height = 0;
				break;
			case '\a' :
				if (current_x == x_offset)
					current_x += x_spacing;
				x_offset += x_spacing;
				break;
			case '\b' :
				if (current_x == x_offset)
					current_x -= x_spacing;
				x_offset -= x_spacing;
				break;
			case '\t' :
				current_x += 2*x_spacing;
				current_x /= (2*x_spacing);
				current_x *= (2*x_spacing);
				break;
			case '%' :
				if (++i < _align.size()) {
					switch (_align[i]) {
					case 'X' : parse_variable_change(_align, i, x_spacing, 12); break;
					case 'Y' : parse_variable_change(_align, i, y_spacing, 8); break;
					case 'x' : parse_variable_change(_align, i, current_x, -1); break;
					case 'y' : parse_variable_change(_align, i, current_y, -1); break;
					case 't' : parse_variable_change(_align, i, tab_size, 50); break;
					case 'i' : parse_variable_change(_align, i, x_offset, 100); break;
					case 'w' : parse_variable_change(_align, i, default_width, 200); break;
					case 'h' : parse_variable_change(_align, i, default_height, 20); break;
					}
				}
			}
		}
	}
}

/// align last element and add element to group
void fltk_align_group::finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& _align, cgv::base::base_ptr element)
{
	scroll_group->end();
	if (scroll_group->children() == 1) {
		static_cast<fltk::MyScrollGroup*>(scroll_group)->offsetx = x_offset;
		static_cast<fltk::MyScrollGroup*>(scroll_group)->offsety = y_offset;
	}
	cgv::base::group::append_child(element);

	if (get_nr_children() != scroll_group->children())
		std::cerr << "fltk_align_group has different number of children " 
		<< get_nr_children() << " as fltk representation: " << scroll_group->children() 
		<< std::endl;

	fltk::Widget* element_widget = static_cast<fltk::Widget*>(element->get_user_data());
	fltk::Widget* last_widget = scroll_group->child(scroll_group->children()-1);
	if (element_widget != last_widget)
		std::cerr << "user data of element widget does not point to last element of group after creation." << std::endl;

	// update line height
	int dx = 0, last_height = last_widget->h();
	if ((last_widget->flags() & fltk::ALIGN_INSIDE) == 0) {
		if ((last_widget->flags() & fltk::ALIGN_BOTTOM) != 0)
			last_height += (int)last_widget->labelsize();
		if ((last_widget->flags() & fltk::ALIGN_LEFT) != 0 && 
			(last_widget->flags() & (fltk::ALIGN_BOTTOM+fltk::ALIGN_TOP)) == 0 && 
			current_x != x_offset) {
			fltk::setfont(last_widget->labelfont(), last_widget->labelsize());
			current_x += (int)fltk::getwidth(last_widget->label());
		}
		if ((last_widget->flags() & fltk::ALIGN_RIGHT) != 0 && 
			(last_widget->flags() & (fltk::ALIGN_BOTTOM+fltk::ALIGN_TOP)) == 0 && 
			current_x != x_offset) {
			fltk::setfont(last_widget->labelfont(), last_widget->labelsize());
			dx += (int)fltk::getwidth(last_widget->label());
		}
	}
	if (last_height > current_line_height)
		current_line_height = last_height;

	int last_widget_x = current_x;
	current_x += last_widget->w()+dx;
	// position child at current location
	last_widget->position(last_widget_x, current_y);
	align(_align);
	scroll_group->init_sizes();
	scroll_group->relayout();
}

void fltk_align_group::register_object(base_ptr object, const std::string& options)
{
	if (!cgv::utils::is_element(get_name(),options))
		return;
	provider* p = object->get_interface<cgv::gui::provider>();
	if (p && get_provider_parent(p).empty()) {
		set_provider_parent(p,gui_group_ptr(this));
		p->create_gui();
	}
}

///
void fltk_align_group::unregister_object(cgv::base::base_ptr object, const std::string& options)
{
	if (!options.empty() && !cgv::utils::is_element(get_name(),options))
		return;

	provider* p = object->get_interface<cgv::gui::provider>();
	if (p && get_provider_parent(p) == this)
		remove_all_children();
}

/// sets focus on the given child
void fltk_align_group::select_child(base_ptr ci, bool exclusive)
{
	fltk::Widget* widget = static_cast<fltk::Widget*>(ci->get_user_data());
	widget->take_focus();
}

/// defocus child 
bool fltk_align_group::unselect_child(base_ptr ci)
{
	return false;
}

/// returns index of focused child
int fltk_align_group::get_selected_child_index() const
{
	for (unsigned ci=0; ci<get_nr_children(); ++ci)
		if (static_cast<fltk::Widget*>(get_child(ci)->get_user_data())->focused())
			return (int)ci;
	return -1;
}

/// return whether the given child is selected
bool fltk_align_group::is_selected(base_ptr c) const
{
	return static_cast<fltk::Widget*>(c->get_user_data())->focused();
}
