#include "fltk_scroll_group.h"

#include <cgv/utils/scan.h>
#include <cgv/gui/provider.h>
#include <cgv/type/variant.h>
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
#include <fltk/ScrollGroup.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif

fltk_scroll_group::fltk_scroll_group(int x, int y, int w, int h, const std::string& _name) : cgv::gui::gui_group(_name)
{
	// extra space applied inside of group before and after any widgets
	x_padding = 10;
	y_padding = 10;
	init_aligment();
	scroll_group = new CG<fltk::ScrollGroup>(x,y,w,h,get_name().c_str());
	scroll_group->set_padding(x_padding, y_padding);
	scroll_group->box(fltk::FLAT_BOX);
	scroll_group->user_data(static_cast<cgv::base::base*>(this));
	
	scroll_group->hscrollbar.pagesize(20);
	scroll_group->scrollbar.pagesize(20);
	scroll_group->scrollbar.linesize(14);
}

/// delete fltk group realization
fltk_scroll_group::~fltk_scroll_group()
{
	delete scroll_group;
}


/// only uses the implementation of fltk_base
std::string fltk_scroll_group::get_property_declarations()
{
	return fltk_base::get_property_declarations()+";xscroll:int;yscroll:int";
}
/// abstract interface for the setter
bool fltk_scroll_group::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "xscroll") {
		if (scroll_group) {
			int xscroll = 0;
			cgv::type::get_variant(xscroll, value_type, value_ptr);
			scroll_group->scrollTo(xscroll, scroll_group->yposition());
			return true;
		}
	}
	if (property == "yscroll") {
		if (scroll_group) {
			int yscroll = 0;
			cgv::type::get_variant(yscroll, value_type, value_ptr);
			scroll_group->scrollTo(scroll_group->xposition(), yscroll);
			return true;
		}
	}
	if(property == "dolayout") {
		layout();
	}
	return fltk_base::set_void(scroll_group, this, property, value_type, value_ptr);

}
/// abstract interface for the getter
bool fltk_scroll_group::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "xscroll") {
		if (scroll_group) {
			int xscroll = scroll_group->xposition();
			cgv::type::set_variant(xscroll, value_type, value_ptr);
			return true;
		}
	}
	if (property == "yscroll") {
		if (scroll_group) {
			int yscroll = scroll_group->yposition();
			cgv::type::set_variant(yscroll, value_type, value_ptr);
			return true;
		}
	}
	if (property == "H") {
		cgv::type::set_variant(current_y, value_type, value_ptr);
		return true;
	}
	return fltk_base::get_void(scroll_group, this, property, value_type, value_ptr);
}

/// return a fltk::Widget pointer that can be cast into a fltk::Group
void* fltk_scroll_group::get_user_data() const
{
	return static_cast<fltk::Widget*>(scroll_group);
}

/// put default sizes into dimension fields and set inner_group to be active
void fltk_scroll_group::prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h)
{
	x = current_x;
	y = current_y;
	w = scroll_group->w() - 2 * x_padding;
	h = default_height;
	scroll_group->begin();
}

/// initialize the members used for alignment
void fltk_scroll_group::init_aligment()
{
	x_offset = 0;
	y_offset = 0;
	x_spacing = 12;
	y_spacing = 8;

	current_x = x_padding + x_offset;
	current_y = y_padding + y_offset;

	default_width = 200;
	default_height = 20;
}

/// overload to trigger initialization of alignment
void fltk_scroll_group::remove_all_children(cgv::gui::gui_group_ptr ggp)
{
	init_aligment();
	fltk_gui_group::remove_all_children(ggp);
}

/// align last element and add element to group
void fltk_scroll_group::finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& _align, cgv::base::base_ptr element)
{
	scroll_group->end();
	if (scroll_group->children() == 1) {
//		static_cast<fltk::MyScrollGroup*>(scroll_group)->offsetx = x_offset;
//		static_cast<fltk::MyScrollGroup*>(scroll_group)->offsety = y_offset;
	}
	cgv::base::group::append_child(element);

	if (get_nr_children() != scroll_group->children())
		std::cerr << "fltk_scroll_group has different number of children " 
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
	
	current_y += last_height + y_spacing;

	scroll_group->init_sizes();

	layout();
}

void fltk_scroll_group::register_object(base_ptr object, const std::string& options)
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
void fltk_scroll_group::unregister_object(cgv::base::base_ptr object, const std::string& options)
{
	if (!options.empty() && !cgv::utils::is_element(get_name(),options))
		return;

	provider* p = object->get_interface<cgv::gui::provider>();
	if (p && get_provider_parent(p) == this)
		remove_all_children();
}

void fltk_scroll_group::layout()
{
	//std::cout << "Do layout: " << std::endl;
	int iter = 0;
	bool do_layout = true;
	do {
		//std::cout << "Iteration " << iter << std::endl;
		int width_shrink = scroll_group->scrollbar.visible() ? scroll_group->scrollbar.w() : 0;

		cgv::type::int32_type w;
		get_void("w", "int32", &w);
		w -= 2 * x_padding + width_shrink;

		int x = x_padding;
		int y = y_padding;

		for(auto& child : children) {
			child->set_void("x", "int32", &x);
			child->set_void("y", "int32", &y);
			child->set_void("w", "int32", &w);

			int h = default_height;
			child->get_void("h", "int32", &h);

			y += y_spacing + h;
		}

		bool vscrollbar_visible = scroll_group->scrollbar.visible();
		bool hscrollbar_visible = scroll_group->hscrollbar.visible();

		scroll_group->layout();

		do_layout = false;

		//std::cout << "v: " << vscrollbar_visible << " vs " << scroll_group->scrollbar.visible() << std::endl;
		//std::cout << "h: " << hscrollbar_visible << " vs " << scroll_group->hscrollbar.visible() << std::endl;

		if(scroll_group->scrollbar.visible() && !vscrollbar_visible) {//} || scroll_group->hscrollbar.visible() != hscrollbar_visible) {
			do_layout = true;
		}
		++iter;
	} while(do_layout);
}

/// sets focus on the given child
void fltk_scroll_group::select_child(base_ptr ci, bool exclusive)
{
	fltk::Widget* widget = static_cast<fltk::Widget*>(ci->get_user_data());
	widget->take_focus();
	scroll_group->scrollTo(scroll_group->xposition(), widget->y());
}

/// defocus child 
bool fltk_scroll_group::unselect_child(base_ptr ci)
{
	return false;
}

/// returns index of focused child
int fltk_scroll_group::get_selected_child_index() const
{
	for (unsigned ci=0; ci<get_nr_children(); ++ci)
		if (static_cast<fltk::Widget*>(get_child(ci)->get_user_data())->focused())
			return (int)ci;
	return -1;
}

/// return whether the given child is selected
bool fltk_scroll_group::is_selected(base_ptr c) const
{
	return static_cast<fltk::Widget*>(c->get_user_data())->focused();
}
