#pragma once

#include <cgv/type/info/type_name.h>
#include <cgv/signal/rebind.h>
#include <cgv/utils/convert.h>
#include "gui_group.h"
#include "shortcut.h"
#include "gui_creator.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// derive from this class to provide a gui to the current viewer
class CGV_API provider : public cgv::signal::tacker
{
protected:
	/**@name interface used by the parent gui*/
	//@{
	/// driver specific handle for the group gui element managing the gui built in the provider
	gui_group_ptr parent_group;
	/// 
	provider* parent_provider;
	/// the gui window sets the parent group through this method
	void set_parent(gui_group_ptr);
	/// update the parent group
	void update_parent();
	/// make the gui group a friend class
	friend class gui_group;
	//@}
public:
	/// use the parent group to append to be managed elements that should be destroyed in  a post_recreate_gui event
	gui_group_ptr get_parent_group() const { return parent_group; }
	/// add a newly created view to the group
	view_ptr add_view_void(const std::string& label, const void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align);
	/// add a newly created control to the group
	control_ptr add_control_void(const std::string& label, void* value_ptr, abst_control_provider* acp, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align, void* user_data);
	/**@name creation of gui*/
	//@{
	/// send pure alignment information
	void align(const std::string& _align);
	/// add a new group to the given parent group, not supported yet
	// gui_group_ptr add_group(const std::string& label, const std::string& group_type, const std::string& options, const std::string& align);
	//! Add group with the gui of another object inside.
	/*! Add a new group, where the group elements are defined by another object that
		must be derived from provider. You can use the same group types as in the add_group
		method. */
	gui_group_ptr add_object_gui(base_ptr object, const std::string& label, const std::string& group_type, const std::string& options, const std::string& align);
	/// inline the gui of another object that must be derived from provider.
	void inline_object_gui(base_ptr object);
	/// add a newly created subgroup to the group
	gui_group_ptr add_group(const std::string& label, const std::string& group_type, const std::string& options = "", const std::string& align = "\n");
	/// add a newly created decorator to the group
	base_ptr add_decorator(const std::string& label, const std::string& decorator_type, const std::string& options = "", const std::string& align = "\n");
	/// use the current gui driver to append a new button with the given label
	button_ptr add_button(const std::string& label, const std::string& options = "", const std::string& align = "\n");
	/// use this to add a new view to the gui with a given value type, gui type and init options
	template <typename T>
	data::ref_ptr<view<T> > add_view(const std::string& label, const T& value, const std::string& gui_type = "", const std::string& options = "", const std::string& align = "\n") {
		if (parent_group.empty())
			return data::ref_ptr<view<T> >();
		return parent_group->add_view(label, value, gui_type, options, align);
	}
	/// use this to add a new control to the gui with a given value type, gui type and init options
	template <typename T>
	data::ref_ptr<control<T> > add_control(const std::string& label, T& value, const std::string& gui_type = "", const std::string& options = "", const std::string& align = "\n") {
		if (parent_group.empty())
			return data::ref_ptr<control<T> >();
		return parent_group->add_control(label, value, gui_type, options,align); 
	}
	/// use this to add a new control to the %gui, where the %control is implemented with a %control provider class
	template <typename T>
	data::ref_ptr<control<T> > add_control(const std::string& label, 
		control_provider<T>* provider, const std::string& gui_type = "", 
		const std::string& options = "", const std::string& align = "\n", void* user_data = 0) {
		if (parent_group.empty())
			return data::ref_ptr<control<T> >();
		return parent_group->add_control(label, provider, gui_type, options,align,user_data); 
	}
	//! add control with callback to cgv::base::on_set method on cgv::gui::control::value_change
	/*! use this method to add a control of a member and a callback to the on_set
	    method of the cgv::base::base class. */
	template <typename T>
	void add_member_control(cgv::base::base* base_ptr, const std::string& label, T& value, const std::string& gui_type = "", const std::string& options = "", const std::string& align = "\n") {
		connect_copy(add_control(label, value, gui_type, options, align)->value_change,
			cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
	}
	//! add a collapsable node to the gui
	/*! this method provides support for tree like guis with nodes that can be opened or closed. Each
	    node is represented by a heading with the text provided in the first parameter and of heading 
		level specified in the first parameter. The state of the node is stored in a boolean variable
		"toggle" that must be supplied by the implementation of the provider. The toggle needs to be
		initialized in the constructor and is used in the create_gui method to only provide the gui of
		the subtree if toggle is true. Every time the status of the node is changed, the whole gui is
		rebuild with the post_recreate_gui method. The value of the toggle is also the return parameter 
		of add_tree_node such the typical code inside the create_gui method looks
		like
\code
if (add_tree_node("Node", toggle, 2)) {
   // create gui of subtree
}
\endcode
		*/ 
	bool add_tree_node(const std::string& label, bool& toggle, int level, 
		const std::string& a= "\n", gui_group_ptr ggp = gui_group_ptr())
	{
		if (!ggp)
			ggp = parent_group;
		int ii = 0;
		if (a.size() != 1 || a[0] != '\n') {
			ii += 1;
		}
		int size = 24-4*level;
		int off  = size+12;
		ggp->align(std::string("%x-=")+cgv::utils::to_string(off));
		connect_copy(ggp->add_control(std::string(toggle?"-":"+"), toggle, "toggle", std::string("w=")+cgv::utils::to_string(size), " ")->value_change,
			rebind(static_cast<provider*>(this), &provider::post_recreate_gui));		
		ggp->add_decorator(label, "heading", std::string("level=")+cgv::utils::to_string(level), a);
		return toggle;
	}
	/// add a composed gui for value
	template <typename T>
	bool add_gui(const std::string& label, T& value, const std::string& gui_type = "", const std::string& options = "", bool* toggles = 0) 
	{
		return cgv::gui::create_gui(this, label, &value, cgv::type::info::type_name<T>::get_name(), gui_type, options, toggles);
	}
	//@}
protected:
	/**@name callbacks*/
	//@{
	/// called by selection_change_cb whenever the gui of this provider is selected
	virtual void on_select();
	/// called by selection_change_cb whenever the gui of this provider is deselected
	virtual void on_deselect();
	/// this is called by the gui group when the selection changes
	virtual void selection_change_cb(cgv::base::base_ptr new_child, bool selected);
	//@}
public:
	/// default construction
	provider();
	/// ensure to remove posted recreation callbacks
	~provider();
	//! Derive a name for this instance that can be used in the gui as heading.
	/*! This method uses the following strategy to automatically determine
	    the name shown in guis for a provider instance:
		 - try to cast the object into cgv::base::named, if successful,
		   use get_name() method
		 - check whether get_menu_path() results in a path or name. In
		   case of a path, use the last entry of the path as name.
		 - try to cast to cgv::base::base and use get_type_name().
		 - return "unnamed" otherwise
	*/
	virtual std::string get_gui_name() const;
	//! Returns the group type that should be used by the class embedding the gui of the provider.
	/*! The default is to use a group of type "align_group". Overload this virtual method to use a 
	    different group type, such as layout group. */
	virtual std::string get_parent_type() const;
	/// call this to update all views and controls of a member
	virtual void update_member(void* member_ptr);
	/// return a path in the main menu to select the gui
	virtual std::string get_menu_path() const;
	/// return a shortcut to activate the gui without menu navigation
	virtual shortcut get_shortcut() const;
	/// you must overload this for gui creation
	virtual void create_gui() = 0;
	//! Recreate the gui of this instance right now.
	/*! Use this method to recreate the gui, dont call create gui directly.
	    Be careful when calling the method from a functor that is attached to
		a gui element generated by this provider. This can cause the gui element
		to be destroyed before the callback triggering the recreate_gui method
		has been completely finished, what might make the program crash. Use the
		post_recreate_gui method instead. */
	void recreate_gui();
	//! delayed recreation of gui
	/*! schedule the recreation of the gui for the next time the program is idle.
	    This mechanism is implemented in a thread save way. */
	virtual void post_recreate_gui();

	/**@name update of gui*/
	//@{
	/// remove a single element from the gui
	void remove_element(base_ptr);
	/// this method removes all elements from the gui and can be used in a method that rebuilds the complete gui
	void remove_all_elements();
	//! find a gui element by name in the current group, return empty pointer if not found
	base_ptr find_element(const std::string& name);
	//! find a view of a given class member
	/*! find the next view of the given value in the current group. If the index
	    pointer is given, start at the index to which the pointer points and set
		 this index to the index of the child index of the found view */
	template <typename T>
	data::ref_ptr<view<T> > find_view(const T& value, int* idx_ptr=0) { 
		if (parent_group.empty())
			return data::ref_ptr<view<T> >();
		return parent_group->find_view(value,idx_ptr); 
	}
	//! find a control of a given class member
	/** find the next control of the given value in the current group. If the index
	    pointer is given, start at the index to which the pointer points and set
		 this index to the index of the child index of the found control */
	template <typename T>
	data::ref_ptr<control<T> > find_control(T& value, int* idx_ptr=0) { 
		if (parent_group.empty())
			return data::ref_ptr<control<T> >();
		return parent_group->find_control(value,idx_ptr); 
	}
	/// access to control of untyped member pointer
	control_ptr find_control_void(void* value_ptr, int* idx_ptr);
	/// access to view of untyped member pointer
	view_ptr find_view_void(void* value_ptr, int* idx_ptr);
	//@}
};



	}
}

#include <cgv/config/lib_end.h>