#pragma once

#include "fltk_gui_group.h"

#include "lib_begin.h"

namespace fltk {
	class ScrollGroup;
	class Group;
}

/** TODO: Update comment! the align group provides a simple alignment mechanism for arranging
    the children. It is built upon a fltk::ScrollGroup containing a
	 fltk::Group */
class fltk_scroll_group : public cgv::gui::gui_group, public fltk_gui_group
{
protected:
	int x_padding, y_padding;
	int x_offset, y_offset;
	int x_spacing, y_spacing;
	//int tab_size;
	int current_x;
	int current_y;
	//int current_line_height;
	//int width_shrink;

	int default_width;
	int default_height;
	fltk::ScrollGroup* scroll_group;
//	fltk::Group* inner_group;
//	fltk_dragger* dragger;

	//void parse_variable_change(const std::string& align, unsigned int& i, int& var, int default_value);

public:
	/// construct from width, height and name
	fltk_scroll_group(int x, int y, int w, int h, const std::string& _name);
	/// delete fltk group realization
	~fltk_scroll_group();
	/// only uses the implementation of fltk_base
	std::string get_property_declarations();
	/// return the type name
	std::string get_type_name() const { return "fltk_scroll_group"; }
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return a fltk::Widget pointer that can be cast into a fltk::Group
	void* get_user_data() const;
	/// initialize the members used for alignment
	void init_aligment();
	/// process alignment information
	//void align(const std::string& align);
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
	///
	void layout();

	/**@name selection of children */
	//@{
	/// return whether several children of the group can be selected at the same time
	bool multiple_selection() const { return false; }
	/// sets focus on the given child
	void select_child(base_ptr ci, bool exclusive = false);
	/// defocus child 
	bool unselect_child(base_ptr ci);
	/// returns index of focused child
	int get_selected_child_index() const;
	/// return whether the given child is selected
	bool is_selected(base_ptr c) const;
	//@}
};

/// ref counted pointer to fltk_scroll_group
typedef cgv::data::ref_ptr<fltk_scroll_group> fltk_scroll_group_ptr;

#ifdef WIN32
CGV_TEMPLATE template class CGV_API cgv::data::ref_ptr<fltk_scroll_group>;
#endif

#include <cgv/config/lib_end.h>
