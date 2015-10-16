#include "fltk_generic_window.h"
#include "fltk_driver.h"
#include <cgv/gui/gui_driver.h>
#include <fltk/Cursor.h>

void destroy_callback_1(fltk::Widget* w)
{
	fltk_generic_window* v = static_cast<fltk_generic_window*>(w);
	v->destroy();
	fltk_driver* d = cgv::gui::get_gui_driver()->get_interface<fltk_driver>();
	if (!d) {
		std::cerr << "could not notify driver!!" << std::endl;
		return;
	}
	window_ptr wp(v);
	d->remove_window(wp);
	(void*&)wp = 0;
}

fltk_generic_window::fltk_generic_window(int x, int y, int w, int h, const std::string& _name):
cgv::gui::window(_name), CI<fltk::Window>(w, h), title(_name)
{
	cursor_shown = true;

	fltk::Window::label(title.c_str());
	last_w = 0;
	last_h = 0;
	callback(destroy_callback_1);

	resizable(this);
}


fltk_generic_window::~fltk_generic_window() 
{

}

/// return the group that is managing the content of the window
gui_group_ptr fltk_generic_window::get_inner_group()
{
	if (inner_group)
		return inner_group;
	return gui_group_ptr(this);
}


void fltk_generic_window::prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h)
{
	x = 0;
	y = 0;
	w = 0;
	h = 0;

	if (inner_group)
		inner_group->get_interface<fltk_gui_group>()->prepare_new_element(ggp, x, y, w, h);
}


/// called by the driver after a new element has been constructed
void fltk_generic_window::finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& align, base_ptr child)
{
	if (inner_group)
		inner_group->get_interface<fltk_gui_group>()->finalize_new_element(ggp, align, child);
}


/// remove the given child, if it appears several times, remove al instances. Return the number of removed children
unsigned int fltk_generic_window::remove_child(cgv::gui::gui_group_ptr ggp, base_ptr child)
{
	return 0;
}


/// remove all children
void fltk_generic_window::remove_all_children(cgv::gui::gui_group_ptr ggp)
{

}


void fltk_generic_window::show(bool modal)
{
	if (modal)
		fltk::Window::exec();
	else
		fltk::Window::show();
}


void fltk_generic_window::hide()
{
	fltk::Window::hide();
}


/// only uses the implementation of fltk_base
std::string fltk_generic_window::get_property_declarations() 
{
	std::string decl("group:string");

	if (inner_group) {
		decl+=";";
		decl+=inner_group->get_property_declarations();
	}

	return decl;
}


/// abstract interface for the setter
bool fltk_generic_window::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "group") {
		get_variant(group_name, value_type, value_ptr);
		if (inner_group)
			inner_group.clear();
		remove_all();
		begin();
		inner_group = gui_group_ptr(gui_group::add_group("", group_name, "", ""));
		end();
		layout();

	} 
	else if (property == "hotspot") {
		hotspot(this);
		return true;
	} 
	else if (property == "W") {
		int _w;
		get_variant(_w, value_type, value_ptr);
		w(_w);
		layout();
		return true;
	} 
	else if (property == "H") {
		int _h;
		get_variant(_h, value_type, value_ptr);
		h(_h);
		layout();
		return true;
	} 
	else if (inner_group && inner_group->set_void(property, value_type, value_ptr))
		return true;
	else
		return false;

	return true;
}


/// abstract interface for the getter
bool fltk_generic_window::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{

	if (property == "group")
		set_variant(group_name, value_type, value_ptr);
	else
	if (inner_group && inner_group->get_void(property, value_type, value_ptr))
		return true;
	else
		return false;

	return true;
}


void fltk_generic_window::layout()
{
	if (inner_group && (last_w != w() || last_h != h())) {
		inner_group->set<int>("w", w());
		inner_group->set<int>("h", h());
		inner_group->set<int>("dolayout", 0);
	}

	last_w = w();
	last_h = h();

	fltk::Window::layout();
}

