#pragma once

#include "fltk_gui_group.h"

//#include "lib_begin.h"

#include <fltk/Group.h>
#include <fltk/Widget.h>

#include <cgv/gui/layout.h>
#include <cgv/gui/layout_table.h>
#include <cgv/gui/layout_inline.h>

#include <cgv/type/variant.h>

using namespace cgv::type;

#include "lib_begin.h"

class CGV_API fltk_layout_group: public cgv::gui::gui_group, public fltk_gui_group, public CG<fltk::Group>
{
public:
	fltk_layout_group(int x, int y, int w, int h, const std::string& _name);

	/// return the type name
	std::string get_type_name() const { return "fltk_layout_group"; }
	/// only uses the implementation of fltk_base
	std::string get_property_declarations();
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return a fltk::Widget pointer that can be cast into a fltk::Group
	void* get_user_data() const;
	/// put default sizes into dimension fields and set inner_group to be active
	void prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h);
	/// align last element and add element to group
	void finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& align, cgv::base::base_ptr element);
	/// remove all elements of the vector that point to child, return the number of removed children
	unsigned int remove_child(base_ptr child) { return static_cast<fltk_gui_group*>(this)->remove_child(cgv::gui::gui_group_ptr(this), child); }
	/// remove all children
	void remove_all_children() { return static_cast<fltk_gui_group*>(this)->remove_all_children(cgv::gui::gui_group_ptr(this)); }
	/// overload to trigger initialization of alignment
	void remove_all_children(cgv::gui::gui_group_ptr ggp);
	/// interface of adding an object
	void register_object(cgv::base::base_ptr object, const std::string& options);
	/// unregister an object
	void unregister_object(cgv::base::base_ptr object, const std::string& options);

	void layout();

private:
	cgv::gui::layout_ptr formatter;
	std::string border_style;
	std::string formatter_name;

	void update_box_type();
};


/// ref counted pointer to fltk_layout_group
typedef cgv::data::ref_ptr<fltk_layout_group> fltk_layout_group_ptr;

#ifdef WIN32
CGV_TEMPLATE template class CGV_API cgv::data::ref_ptr<fltk_layout_group>;
#endif

#include <cgv/config/lib_end.h>