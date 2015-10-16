#pragma once

#include "fltk_gui_group.h"

#include <fltk/Window.h>
#include <cgv/gui/window.h>
#include <cgv/gui/gui_group.h>
#include <cgv/type/variant.h>

using namespace cgv::type;
using namespace cgv::gui;

#include "lib_begin.h"

class CGV_API fltk_generic_window: 
	public cgv::gui::window, 
	public CI<fltk::Window>, 
	public fltk_gui_group 
{
public:
	fltk_generic_window(int x, int y, int w, int h, const std::string& _name);
	~fltk_generic_window();

	// prepare adding a new element
	void prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h);
	/// called by the driver after a new element has been constructed
	void finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& align, base_ptr child);
	/// remove the given child, if it appears several times, remove al instances. Return the number of removed children
	unsigned int remove_child(cgv::gui::gui_group_ptr ggp, base_ptr child);
	/// remove all children
	void remove_all_children(cgv::gui::gui_group_ptr ggp);

	// window functions
	void show(bool modal);
	void hide();

	/// return the group that is managing the content of the window
	gui_group_ptr get_inner_group();
	/// only uses the implementation of fltk_base
	std::string get_property_declarations();
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);


	// fltk functions
	void layout();

private:
	gui_group_ptr inner_group;
	std::string title;
	std::string group_name;
	int last_w, last_h;
	bool cursor_shown;

};

#include <cgv/config/lib_end.h>