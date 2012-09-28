#pragma once

#include "fltk_gui_group.h"

namespace fltk {
	class TabGroup;
}

#include "lib_begin.h"

/** the tab group adds for each child a new tab labeled with the name
    of the child */
class fltk_tab_group : public cgv::gui::gui_group, public fltk_gui_group
{
protected:
	fltk::TabGroup* tab_group;
	static void fltk_select_cb(fltk::Widget* w, void*ud);
	int last_selected_child;
public:
	/// construct from dimensions and name
	fltk_tab_group(int x, int y, int w, int h, const std::string& _name);
	/// destruct fltk group realization
	~fltk_tab_group();
	/// return a pointer to the fltk tab group
	fltk::TabGroup* get_fltk_tab_group() const;
	/// only uses the implementation of fltk_base
	std::string get_property_declarations();
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return the type name
	std::string get_type_name() const { return "fltk_tab_group"; }
	/// this virtual method allows to pass application specific data for internal purposes
	void* get_user_data() const;
	/// return the index of the currently selected child.
	int get_selected_child_index() const;
	/// select a given child
	void select_child(base_ptr c);
	/// put default sizes into dimension fields and set inner_group to be active
	void prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h);
	/// align last element and add element to group
	void finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& align, cgv::base::base_ptr element);
	/// remove all elements of the vector that point to child, return the number of removed children
	unsigned int remove_child(base_ptr child) { return static_cast<fltk_gui_group*>(this)->remove_child(cgv::gui::gui_group_ptr(this), child); }
	/// remove all children
	void remove_all_children() { return static_cast<fltk_gui_group*>(this)->remove_all_children(cgv::gui::gui_group_ptr(this)); }
	/// interface of adding an object
	void register_object(cgv::base::base_ptr object, const std::string& options);
	/// unregister an object
	void unregister_object(cgv::base::base_ptr object, const std::string& options);
};

/// ref counted pointer to fltk_tab_group
typedef cgv::data::ref_ptr<fltk_tab_group> fltk_tab_group_ptr;

#ifdef WIN32
CGV_TEMPLATE template class CGV_API cgv::data::ref_ptr<fltk_tab_group>;
#endif

#include <cgv/config/lib_end.h>