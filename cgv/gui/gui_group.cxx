#include "gui_group.h"
#include "gui_driver.h"
#include "provider.h"
#include <cgv/base/named.h>

namespace cgv {
	namespace gui {

// construct from name
gui_group::gui_group(const std::string& name) : cgv::base::group(name) {}

// access to protected provider method
void gui_group::set_provider_parent(provider* p, gui_group_ptr g)
{
	p->set_parent(g);
}

void gui_group::register_object(base_ptr object, const std::string& options)
{
}

void gui_group::unregister_object(cgv::base::base_ptr object, const std::string& options)
{
}

// driver specific handle for the group gui element managing the gui built in the provider
gui_group_ptr gui_group::get_provider_parent(const provider* p)
{
	return p->parent_group;
}

bool gui_group::multiple_selection() const
{
	return false;
}

void gui_group::select_child(unsigned ci, bool exclusive)
{
	select_child(get_child(ci),exclusive);
}

/// same as version with child index
void gui_group::select_child(base_ptr c, bool exclusive)
{
	if (!is_selected(c)) {
		if (!multiple_selection())
			on_selection_change(get_selected_child(), false);
		on_selection_change(c, true);
	}
}

bool gui_group::unselect_child(unsigned ci)
{
	return unselect_child(get_child(ci));
}

bool gui_group::unselect_child(base_ptr c)
{
	if (is_selected(c)) {
		on_selection_change(c, false);
		return false;
	}
	return true;
}

int gui_group::get_selected_child_index() const
{
	return -1;
}

base_ptr gui_group::get_selected_child() const
{
	int ci = get_selected_child_index();
	if (ci == -1)
		return base_ptr();
	return get_child(ci);
}

bool gui_group::is_selected(base_ptr c) const
{
	return get_selected_child() == c;
}

bool gui_group::is_selected(unsigned ci) const
{
	return is_selected(get_child(ci));
}

/// returns whether open and close of sub groups is allowed
bool gui_group::can_open_and_close() const
{
	return false;
}

/// return whether the given child is open
bool gui_group::is_open_child_group(gui_group_ptr g) const
{
	return true;
}

/// return whether the ci-th child is an open gui group
bool gui_group::is_open_child_group(unsigned ci) const
{
	return get_child(ci)->get_interface<gui_group>() != 0;
		return true;
}

/// open the given child group
bool gui_group::open_child_group(gui_group_ptr g)
{
	return false;
}

/// close the given child group
bool gui_group::close_child_group(gui_group_ptr g)
{
	return false;
}

// add a newly created view to the group
view_ptr gui_group::add_view_void(const std::string& label, const void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align)
{
	if (get_gui_driver().empty())
		return view_ptr();
	return get_gui_driver()->add_view(gui_group_ptr(this), 
		label, value_ptr, value_type, gui_type, options, align);
}
// add a newly created control to the group
control_ptr gui_group::add_control_void(const std::string& label, 
			void* value_ptr, abst_control_provider* acp, 
			const std::string& value_type, const std::string& gui_type, 
			const std::string& options, const std::string& align, void* user_data)
{
	if (get_gui_driver().empty())
		return control_ptr();
	return get_gui_driver()->add_control(gui_group_ptr(this), 
		label, value_ptr ? value_ptr : user_data, acp, value_type, gui_type, options, align);
}
/// find a gui element by name, return empty pointer if not found
base_ptr gui_group::find_element(const std::string& name)
{
	unsigned n = get_nr_children();
	for (unsigned i=0; i<n; ++i) {
		base_ptr bp = get_child(i);
		cgv::base::named_ptr np = bp->get_named();
		if (np) {
			if (np->get_name() == name)
				return bp;
		}
	}
	return base_ptr();
}

// find a view in the group based on a const void pointer
view_ptr gui_group::find_view_void(const void* value_ptr, int* idx_ptr)
{
	if (get_gui_driver().empty())
		return view_ptr();
	return get_gui_driver()->find_view(gui_group_ptr(this), 
		value_ptr, idx_ptr);
}
// find a control in the group based on a const void pointer
control_ptr gui_group::find_control_void(void* value_ptr, int* idx_ptr)
{
	if (get_gui_driver().empty())
		return control_ptr();
	return get_gui_driver()->find_control(gui_group_ptr(this), 
		value_ptr, idx_ptr);
}

// overload to return the type name of this object
std::string gui_group::get_type_name() const
{
	return "gui_group";
}

// send pure alignment information
void gui_group::align(const std::string& _align)
{
}

// add a new group to the given parent group
gui_group_ptr gui_group::add_group(const std::string& label, const std::string& group_type, const std::string& options, const std::string& align)
{
	if (get_gui_driver().empty())
		return gui_group_ptr();
	return get_gui_driver()->add_group(gui_group_ptr(this), 
		label, group_type, options, align);
}
// add a newly created decorator to the group
base_ptr gui_group::add_decorator(const std::string& label, const std::string& decorator_type, const std::string& options, const std::string& align)
{
	if (get_gui_driver().empty())
		return base_ptr();
	return get_gui_driver()->add_decorator(gui_group_ptr(this), 
		label, decorator_type, options, align);
}
// add a newly created button to the group
button_ptr gui_group::add_button(const std::string& label, const std::string& options, const std::string& align)
{
	if (get_gui_driver().empty())
		return button_ptr();
	return get_gui_driver()->add_button(gui_group_ptr(this), 
		label, options, align);
}


	}
}
