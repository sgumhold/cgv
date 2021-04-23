#pragma once

#include <cgv/base/base.h>
#include <cgv/type/info/type_name.h>
#include <cgv/signal/rebind.h>
#include <cgv/utils/convert.h>
#include "gui_group.h"
#include "shortcut.h"
#include "gui_creator.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {


/// helper struct to support value references as well as value references with index
template <typename T>
struct with_index_struct
{
	const T&  value;
	int index;
	with_index_struct(const T& _value, int _index) : value(_value), index(_index) {}
};

/// helper function to support value references as well as value references with index for the tree_node functions of cgv::gui::provider
template <typename T>
with_index_struct<T> with_index(const T& _value, int _index)
{
	return with_index_struct<T>(_value, _index);
}


template <typename T> 
struct with_index_traits
{
	typedef const T* pointer_type;
	static pointer_type get_value_ptr(const T& value) { return &value; }
	static int get_index(const T& value) { return -1; }
};

template <typename T> 
struct with_index_traits<with_index_struct<T> >
{
	typedef const T* pointer_type;
	static pointer_type get_value_ptr(const with_index_struct<T>& value) { return &value.value; }
	static int get_index(const with_index_struct<T>& value) { return value.index; }
};

template <typename T> 
typename with_index_traits<T>::pointer_type wi_get_value_ptr(const T& value)
{
	return with_index_traits<T>::get_value_ptr(value);
}

template <typename T>
int wi_get_index(const T& value)
{
	return with_index_traits<T>::get_index(value);
}

/// derive from this class to provide a gui to the current viewer
class CGV_API provider : virtual public cgv::signal::tacker
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
	/// concatenate names in string to enum declaration and optionally prepend or append given additional names 
	std::string concat_enum_def(const std::vector<std::string>& names, const std::string& additional_first_name = "", const std::string& additional_last_name = "");

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
	data::ref_ptr<control<T> > add_member_control(cgv::base::base* base_ptr, const std::string& label, T& value, const std::string& gui_type = "", const std::string& options = "", const std::string& align = "\n") {
		data::ref_ptr<control<T> > cp = add_control(label, value, gui_type, options, align);
		connect_copy(cp->value_change,
			cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return cp;
	}
	//! add a collapsable node to the gui (deprecated)
	/*! This method is one possibility to support tree like guis with nodes that can be opened or closed.
	    The other prefarable possibitly builds on the functions begin_tree_node() and end_tree_node().
		
		Each node is represented by a heading with the text provided in the first parameter and of heading 
		level specified in the first parameter. The state of the node is stored in a boolean variable
		"toggle" that must be supplied by the implementation of the provider. The toggle needs to be
		initialized in the constructor and is used in the create_gui method to only provide the gui of
		the subtree if toggle is true. Every time the status of the node is changed, the whole gui is
		rebuild with the post_recreate_gui method. The value of the toggle is also the return parameter 
		of add_tree_node such the typical code inside the create_gui method looks
		like
\code
if (add_tree_node("Node", toggle, 2)) {
   align("\a"); // indent gui elements of tree node
   // create gui of subtree
   
   align("\b"); // undo indentation
}
\endcode
		*/
	bool add_tree_node(const std::string& label, bool& toggle, int level, const std::string& a= "\n", gui_group_ptr ggp = gui_group_ptr());
	//! Begin a sub tree of a tree structured gui.
	/*! This function addes a toggle button and a heading for the tree node.
		The toggle button can be used to show or hide the subtree below the tree node. The heading shows simply the label
		parameter. The function returns the visibility state of the subtree below the tree node. Therefore its contents
		should only be specified if the function returns true. In that case one needs to terminate the gui elements added
		for the tree node with the end_tree_node function. A typical example would be
\code
if (begin_tree_node("Node", composed_value)) {
   align("\a"); // indent gui elements of tree node
   // create gui of composed_value
   
   align("\b"); // undo indentation
   end_tree_node(composed_value);
}
\endcode
		The state of the toggle button is attached to a boolean flag that is globally managed by the provider.
		If your this pointer can be converted to cgv::base::base, the on_set callback with a pointer to the flag
		is called when the user toggles the tree node. To check in the on_set callback for the tree node toggle
		one can get a reference to the boolean flag with the ref_tree_node_visible_flag method. 
		
		To allocate the boolean flag the reference to a value controlled by the tree node is specified. The pointer to the controlled value is 
		used as key for a map that manages the toggle states of all tree node buttons. If there is no superior structure
		whose value is controlled by the tree node, one can specify any of the values controled by the tree node. It is just
		important that no two tree nodes use the same value and that the pointer to the value cannot change. The latter is
		for example the case, when one uses an entry in a std::vector that can change size and reallocate its values. Then
		one should use the std::vector itself as value. In order to be able to distinguish the different elements of a vector
		one can extend the key from a value reference to a pair of a value reference plus an index. The index is then the index
		of the vector element. This is done by specifying with_index(value, idx) in the value argument. An example could look as follows:
\code
if (begin_tree_node("Node", vec)) {
   align("\a"); // indent gui elements of tree node
   for (unsigned i=0; i<vec.size; ++i) {
		if (begin_tree_node(std::string("element ")+cgv::utils::to_string(i), with_index(vec,i))) {
		   align("\a"); // indent gui elements of tree node for vector element
		   // create gui of vector element
   
		   align("\b"); // undo indentation
		   end_tree_node(with_index(vec,i));
		}
	}
	align("\b"); // undo indentation
	end_tree_node(vec);
}
\endcode
	*/
	template <typename T>
	bool begin_tree_node(const std::string& label, const T& value, bool initial_visibility = false, const std::string& options = "", gui_group_ptr ggp = gui_group_ptr()) { return begin_tree_node_void(label, wi_get_value_ptr(value), wi_get_index(value), initial_visibility, options, ggp); }
	//! return a reference to the boolean flag, that tells whether the tree node for the passed value is visible
	/*! T can be with_index(v,i) as in the begin_tree_node function. You typically need this function in an on_set callback to check whether the user
	    has toggled a tree node */
	template <typename T>
	static bool& ref_tree_node_visible_flag(const T& value) { return ref_tree_node_visible_flag_void(wi_get_value_ptr(value), wi_get_index(value)); }
	/// template specialization that allows to specify value reference plus node_instance by using the result of the function with_instance(value,idx) for the value argument
	//! finish a sub tree begun with begin_tree_node 
	/*! This functions should be called only if the corresponding call to begin_tree_node returned true. */
	template <typename T>
	void end_tree_node(const T& value) { end_tree_node_void(wi_get_value_ptr(value), wi_get_index(value)); }
	/// return whether the sub tree attached to a value is visible
	template <typename T>
	bool is_tree_node_visible(const T& value) const { return is_tree_node_visible_void(wi_get_value_ptr(value), wi_get_index(value)); }
	/// set the visibility status of sub tree attached to a value. This calls the post_recreate method if needed.
	template <typename T>
	void set_tree_node_visibility(const T& value, bool is_visible) { set_tree_node_visibility_void(wi_get_value_ptr(value), wi_get_index(value), is_visible); }
	/// void version of the templated functions
	bool begin_tree_node_void(const std::string& label, const void* value_ptr, int index = -1, bool initial_visibility = false, const std::string& options = "", gui_group_ptr ggp = gui_group_ptr());
	///
	static bool& ref_tree_node_visible_flag_void(const void* value_ptr, int index = -1);
	///
	void end_tree_node_void(const void* value_ptr, int index = -1);
	///
	bool is_tree_node_visible_void(const void* value_ptr, int index) const;
	///
	void set_tree_node_visibility_void(const void* value_ptr, int index, bool is_visible);

	//! Add a composed gui of the given gui_type for the given value.
	/*! This function returns false if no cgv::base::gui_creator has been registered for the given gui_type.
	    The plugin cg_ext contains registers gui_creators for the most important types of the framework.
		The supported values for the options parameter are specific for the gui_type. Currently these are not
		documented and can only be found in the source code of the cg_ext plugin.
	*/
	template <typename T>
	bool add_gui(const std::string& label, T& value, const std::string& gui_type = "", const std::string& options = "") 
	{
		return cgv::gui::create_gui(this, label, &value, cgv::type::info::type_name<T>::get_name(), gui_type, options, 0);
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
	/// ensure that my UI is selected in the parent group in case this is a tab group, otherwise return false
	virtual bool ensure_selected_in_tab_group_parent();
	/// call this to update all views and controls of a member
	virtual void update_member(void* member_ptr);
	/// call this to update all views and controls of all member
	virtual void update_all_members();
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
	virtual void recreate_gui();
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