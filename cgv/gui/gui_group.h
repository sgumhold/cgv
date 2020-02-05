#pragma once

#include <cgv/base/group.h>
#include <cgv/type/cond/is_enum.h>
#include <cgv/type/info/type_name.h>
#include <cgv/base/register.h>
#include "button.h"
#include "view.h"
#include "control.h"

#include "lib_begin.h"

using cgv::base::base_ptr;

template <typename T, bool is_enum = cgv::type::cond::is_enum<T>::value>
struct enum_aware_type_name : public cgv::type::info::type_name<T>
{
};

template <typename T>
struct enum_aware_type_name<T,true> 
{
	static const char* get_name() { return "enum"; }
};

namespace cgv {
	namespace gui {

class CGV_API gui_group;
class CGV_API provider;

/// ref counted pointer to a %gui %group
typedef data::ref_ptr<gui_group,true> gui_group_ptr;

/// ref counted pointer to const %gui %group
typedef data::ref_ptr<const gui_group,true> const_gui_group_ptr;

/// %gui independent group class which is a container for %gui elements
class CGV_API gui_group : public cgv::base::group, public cgv::base::registration_listener
{
protected:
	friend class provider;
	/// managed objects can be add to the group such that 
	std::vector<cgv::base::base_ptr> managed_objects;
	/// access to protected provider method
	static void set_provider_parent(provider* p, gui_group_ptr g);
	/// %driver specific handle for the group gui element managing the gui built in the provider
	static gui_group_ptr get_provider_parent(const provider* p);
public:
	/// interface of adding an object
	void register_object(cgv::base::base_ptr object, const std::string& options);
	///
	void unregister_object(cgv::base::base_ptr object, const std::string& options);
	/// construct from name
	gui_group(const std::string& name = "");
	/// overload to return the %type name of this object
	std::string get_type_name() const;
	
	/**@name managed objects */
	//@{
	//! add the passed object as an managed object. 
	/*! This will simply add a reference counted pointer to the object into a list
	    of managed elements. This pointer is released whenever the gui group is 
		destroyed or when the gui is recreated. This functionality can be used by
		gui_creators to manage objects that are not managed by the user of the gui
		library. An example usage can be found in the bit_field_controler found in the
		plugin plugins/cg_ext. */
	void add_managed_objects(cgv::base::base_ptr object);
	/// release all managed objects
	void release_all_managed_objects();
	/// release a specific managed object
	void release_managed_objects(cgv::base::base_ptr object);
	/// check whether an object is managed by this gui group
	bool is_managed_object(cgv::base::base_ptr object);
	//@/
	/**@name selection of children */
	//@{
	/// return whether several children of the group can be selected at the same time
	virtual bool multiple_selection() const;
	//! select the ci-th child of the group.
	/*! If multiple_selection() returns true, the \c exclusive flag can be used to
	    unselect all other previously selected children except the newly selected
		one. Typically, an exclusively selected child will gain input focus. */
	virtual void select_child(unsigned ci, bool exclusive = false);
	/// same as version with child index
	virtual void select_child(base_ptr ci, bool exclusive = false);
	//! unselect the ci-th child.
	/*! If no multiple_selection is allowed, unselection can fail if the group does
	    not support an empty selection.*/
	virtual bool unselect_child(unsigned ci);
	/// same as version with child index
	virtual bool unselect_child(base_ptr ci);
	//! return the index of the currently selected child.
	/*! In case of multiple_selection, this function returns the index of the first selected child.
	    In this case it can also happen that the returned index is -1 if no child is selected. */
	virtual int get_selected_child_index() const;
	//! return the currently selected child.
	/*! In case of multiple_selection, this function returns the first selected child.
	    In this case it can also happen that the returned base_ptr is empty if no child
		is selected. */
	virtual base_ptr get_selected_child() const;
	/// return whether the given child is selected
	virtual bool is_selected(base_ptr c) const;
	//! return whether the given child is selected.
	/*! the implementation simply calls the virtual variant with get_child(ci). */
	bool is_selected(unsigned ci) const;
	//! This signal is emitted for every change of the selection of a child.
	/*! The first argument is simply the pointer to the child whose selection state
	    has been changed. The second argument is the new selection state. */
	cgv::signal::signal<base_ptr, bool> on_selection_change;
	//@}

	/**@name opening and closing of child groups*/
	//@{
	/// returns whether open and close of sub groups is allowed
	virtual bool can_open_and_close() const;
	/// return whether the given child is open
	virtual bool is_open_child_group(gui_group_ptr g) const;
	/// return whether the ci-th child is an open gui group
	bool is_open_child_group(unsigned ci) const;
	/// try to open given child group and return whether this was successful
	virtual bool open_child_group(gui_group_ptr g);
	/// try to close given child group and return whether this was successful
	virtual bool close_child_group(gui_group_ptr g);
	//! this signal is emitted, when a child group is opened or closed
	/*! The first argument specifies the group and the second whether
	    it has been opened. */
	cgv::signal::signal<gui_group_ptr,bool> on_open_state_change;
	//@}

	/**@name adding new elements to the group */
	//@{
	/// send pure alignment information
	virtual void align(const std::string& _align);
	/// add a new group to the given parent group
	virtual gui_group_ptr add_group(const std::string& label, const std::string& group_type, const std::string& options, const std::string& align);
	/// add a newly created decorator to the group
	virtual base_ptr add_decorator(const std::string& label, const std::string& decorator_type, const std::string& options, const std::string& align);
	/// add a newly created button to the group
	virtual button_ptr add_button(const std::string& label, const std::string& options, const std::string& align);
	/// add a newly created view to the group
	virtual view_ptr add_view_void(const std::string& label, const void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align);
	/// add a newly created control to the group
	virtual control_ptr add_control_void(const std::string& label, void* value_ptr, abst_control_provider* acp, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align, void* user_data);
	/// add a newly created view to the group for the given value with the given %gui %type, init and align options
	template <typename T>
	inline data::ref_ptr<view<T> > add_view(const std::string& label, const T& value, const std::string& gui_type = "", const std::string& options = "", const std::string& align = "\n") {
		view_ptr vp = add_view_void(label, &value, 
						enum_aware_type_name<T>::get_name(), 
						gui_type, options, align);
		return vp.up_cast<view<T> >();
	}
	/// add a newly created control to the group for the given value with the given %gui %type, init and align options
	template <typename T>
	inline data::ref_ptr<control<T> > add_control(const std::string& label, T& value, const std::string& gui_type = "", const std::string& options = "", const std::string& align = "\n") {
		control_ptr cp = add_control_void(label, &value, 0,
					enum_aware_type_name<T>::get_name(), 
					gui_type, options, align, 0);
		return cp.up_cast<control<T> >(); 
	} 
	/// add a newly created control to the group which is controlled by a control_provider
	template <typename T>
	inline data::ref_ptr<control<T> > add_control(const std::string& label, 
		control_provider<T>* provider, const std::string& gui_type = "", 
		const std::string& options = "", const std::string& align = "\n", void* user_data = 0) {
		control_ptr cp = add_control_void(label, 0, provider, 
					enum_aware_type_name<T>::get_name(), 
					gui_type, options, align, user_data);
		return cp.up_cast<control<T> >(); 
	}
	//@}

	/**@name finding elements in the group*/
	//@{
	/// find a gui element by name, return empty pointer if not found
	base_ptr find_element(const std::string& name);
	/// find a view in the group based on a const void pointer
	view_ptr find_view_void(const void* value_ptr, int* idx_ptr);
	/// find a control in the group based on a const void pointer
	control_ptr find_control_void(void* value_ptr, int* idx_ptr);
	//! find the next view of the given value in the current group. 
	/*! If the index
	    pointer is given, start at the index to which the pointer points and set
		 this index to the index of the child index of the found view */
	template <typename T>
	inline data::ref_ptr<view<T> > find_view(const T& value, int* idx_ptr=0) {
		view_ptr vp = find_view_void(&value,idx_ptr);
		return vp.up_cast<view<T> >(); 
	}
	//! find the next control of the given value in the current group. 
	/*! If the index
	    pointer is given, start at the index to which the pointer points and set
		 this index to the index of the child index of the found control */
	template <typename T>
	inline data::ref_ptr<control<T> > find_control(T& value, int* idx_ptr=0) {
		control_ptr cp = find_control_void(&value,idx_ptr);
		return cp.up_cast<control<T> >(); 
	}
	//@}
};


#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<gui_group>;
CGV_TEMPLATE template class CGV_API data::ref_ptr<const gui_group>;
#endif


	}
}

#include <cgv/config/lib_end.h>
