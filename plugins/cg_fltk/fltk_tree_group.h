#pragma once

#include "fltk_gui_group.h"

//#include "lib_begin.h"

#include <fltk/Browser.h>
#include <cgv/type/variant.h>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::type;

#include "lib_begin.h"

// This class is entirely inline.  If that changes, add FL_API to its declaration
class CGVBrowser : public CW<fltk::Browser> {
public:
    CGVBrowser(int x,int y,int w,int h,const char *l=0);
	bool ensure_state_change();
	int handle(int event);
};

extern CGV_API bool goto_item(fltk::Browser* B, fltk::Widget* e);

class CGV_API fltk_tree_group: public cgv::gui::gui_group, public fltk_gui_group, public CGVBrowser
{
protected:
	std::vector<std::pair<std::string,unsigned> > column_infos;
	bool multi_selection;
	void ensure_column_infos();
public:
	fltk_tree_group(int x, int y, int w, int h, const std::string& _name);
	std::string get_type_name() const;
	/// only uses the implementation of fltk_base
	std::string get_property_declarations();
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return a fltk::Widget pointer that can be cast into a fltk::Group
	void* get_user_data() const;
	/// return whether several children of the group can be selected at the same time
	bool multiple_selection() const;
	/// same as version with child index
	void select_child(base_ptr ci, bool exclusive = false);
	/// same as version with child index
	bool unselect_child(base_ptr ci);
	/// return whether the given child is selected
	bool is_selected(base_ptr c) const;
	/// returns whether open and close of sub groups is allowed
	bool can_open_and_close() const;
	/// try to open given child group and return whether this was successful
	bool open_child_group(gui_group_ptr g);
	/// try to close given child group and return whether this was successful
	bool close_child_group(gui_group_ptr g);
	/// return whether the given child is open
	bool is_open_child_group(gui_group_ptr g) const;
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
	/// add a new group to the given parent group
	gui_group_ptr add_group(const std::string& label, const std::string& group_type, const std::string& options, const std::string& align);
	/// add a newly created decorator to the group
	base_ptr add_decorator(const std::string& label, const std::string& decorator_type, const std::string& options, const std::string& align);
	/// add a newly created button to the group
	button_ptr add_button(const std::string& label, const std::string& options, const std::string& align);
	/// add a newly created view to the group
	view_ptr add_view_void(const std::string& label, const void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align);
	/// add a newly created control to the group
	control_ptr add_control_void(const std::string& label, void* value_ptr, abst_control_provider* acp, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align, void* user_data);
	/// interface of adding an object
	void register_object(cgv::base::base_ptr object, const std::string& options);
	/// unregister an object
	void unregister_object(cgv::base::base_ptr object, const std::string& options);
};


/// ref counted pointer to fltk_tree_group
typedef cgv::data::ref_ptr<fltk_tree_group> fltk_tree_group_ptr;

#ifdef WIN32
CGV_TEMPLATE template class CGV_API cgv::data::ref_ptr<fltk_tree_group>;
#endif

#include <cgv/config/lib_end.h>