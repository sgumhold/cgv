#include <cgv/gui/provider.h>
#include <cgv/gui/gui_driver.h>
#include <cgv/gui/trigger.h>

//#include <cgv/os/mutex.h>

#include <iostream>
#include <set>

namespace cgv {
	namespace gui {

// update the parent group
void provider::update_parent()
{
	parent_group->set("label",get_gui_name());
}

/// add a newly created view to the group
view_ptr provider::add_view_void(const std::string& label, const void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align)
{
	if (parent_group.empty())
		return view_ptr();
	return parent_group->add_view_void(label,value_ptr,value_type,gui_type,options,align);
}
/// add a newly created control to the group
control_ptr provider::add_control_void(const std::string& label, void* value_ptr, abst_control_provider* acp, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align, void* user_data)
{
	if (parent_group.empty())
		return control_ptr();
	return parent_group->add_control_void(label,value_ptr,acp,value_type,gui_type,options,align,user_data);
}

// send pure alignment information
void provider::align(const std::string& _align)
{
	if (!parent_group.empty())
		parent_group->align(_align);
}
/*
// add a new group to the given parent group, not supported yet
gui_group_ptr provider::add_group(const std::string& label, const std::string& group_type, const std::string& options, const std::string& align)
{
	if (parent_group.empty())
		return gui_group_ptr();
	return parent_group->add_group(label, group_type, options, align);
}
*/

// call this to update all views and controls of a member
void provider::update_member(void* member_ptr)
{
	update_views(member_ptr);
	/*
	int i=0;
	do {
		cgv::gui::control_ptr cp = find_control_void(member_ptr, &i);
		if (cp) {
			cp->update();
			cp->update_views();
		}
		else
			break;
		++i;
	} while (true);
	i = 0;
	do {
		cgv::gui::view_ptr vp = find_view_void(member_ptr, &i);
		if (vp) {
			vp->update();
			vp->update_views();
		}
		else
			break;
		++i;
	} while (true);
	*/
}

gui_group_ptr provider::add_object_gui(base_ptr object, const std::string& label, const std::string& group_type, const std::string& options, const std::string& align)
{
	provider* p = object->get_interface<provider>();
	if (!p)
		return gui_group_ptr();

	
	gui_group_ptr g = parent_group->add_group(label, group_type, options, align);

//	gui_group_ptr g = add_group(label,group_type,options,align);
	p->set_parent(parent_group);
	// p->create_gui();
	return g;
}

// inline the gui of another object that must be derived from provider.
void provider::inline_object_gui(base_ptr object)
{
	provider* p = object->get_interface<provider>();
	if (!p)
		return;
	p->set_parent(parent_group);
	p->parent_provider = this;
	p->create_gui();
}

/// add a newly created subgroup to the group
gui_group_ptr provider::add_group(const std::string& label, const std::string& group_type, const std::string& options, const std::string& align)
{
	if (parent_group.empty())
		return gui_group_ptr();
	return parent_group->add_group(label, group_type, options, align);
}

// add a newly created decorator to the group, not implemented yet
base_ptr provider::add_decorator(const std::string& label, const std::string& decorator_type, const std::string& options, const std::string& align)
{
	if (parent_group.empty())
		return base_ptr();
	return parent_group->add_decorator(label, decorator_type, options, align);
}

// use the current gui driver to append a new button with the given label
button_ptr provider::add_button(const std::string& label, const std::string& options, const std::string& align)
{
	if (parent_group.empty())
		return button_ptr();
	return parent_group->add_button(label, options, align);
}

// remove a single element from the gui
void provider::remove_element(base_ptr e)
{
	if (!parent_group.empty())
		parent_group->remove_child(e);
}

// this method removes all elements in the gui and can be used in a method that rebuilds the complete gui
void provider::remove_all_elements()
{
	if (!parent_group.empty())
		parent_group->remove_all_children();
}

//! find a gui element by name in the current group, return empty pointer if not found
base_ptr provider::find_element(const std::string& name)
{
	if (parent_group.empty())
		return base_ptr();
	return parent_group->find_element(name);
}


// access to control of untyped member pointer
control_ptr provider::find_control_void(void* value_ptr, int* idx_ptr)
{
	if (parent_group.empty())
		return control_ptr();
	return parent_group->find_control_void(value_ptr,idx_ptr);
}

// access to view of untyped member pointer
view_ptr provider::find_view_void(void* value_ptr, int* idx_ptr)
{
	if (parent_group.empty())
		return view_ptr();
	return parent_group->find_view_void(value_ptr,idx_ptr);
}


// default construction
provider::provider()
{
	parent_provider = 0;
}

// called by selection_change_cb whenever the gui of this provider is selected
void provider::on_select()
{
}

// called by selection_change_cb whenever the gui of this provider is deselected
void provider::on_deselect()
{
}


// this is called by the gui group when the selection changes
void provider::selection_change_cb(cgv::base::base_ptr new_child, bool selected)
{
	if (new_child == parent_group) {
		if (selected)
			on_select();
		else
			on_deselect();
	}
}

// the gui window sets the parent group through this method
void provider::set_parent(gui_group_ptr _parent_group)
{
	if (parent_group) {
		gui_group_ptr ctrl_grp = parent_group->get_parent()->get_interface<gui_group>();
		if (ctrl_grp)
			disconnect(ctrl_grp->on_selection_change, this, &provider::selection_change_cb);
	}
	parent_group = _parent_group;
	gui_group_ptr ctrl_grp = parent_group->get_parent()->get_interface<gui_group>();
	if (ctrl_grp)
		connect(ctrl_grp->on_selection_change, this, &provider::selection_change_cb);
}


/** this method uses the following strategy to automatically determine
    the name shown in guis for a provider instance:
	 - try to cast the object into cgv::base::named, if successful,
	   use get_name() method
	 - check whether get_menu_path() results in a path or name. In
	   case of a path, use the last entry of the path as name.
	 - try to cast to cgv::base::base and use get_type_name().
	 - return "unnamed" otherwise
*/
std::string provider::get_gui_name() const
{
	const cgv::base::base* bp = dynamic_cast<const cgv::base::base*>(this);
	if (bp) {
		if (const_cast<cgv::base::base*>(bp)->get_named())
			return const_cast<cgv::base::base*>(bp)->get_named()->get_name();
	}
	std::string mp = get_menu_path();
	if (!mp.empty()) {
		unsigned int pos = (unsigned int) mp.find_last_of('/');
		if (pos == std::string::npos || pos == mp.size()-1)
			return mp;
		return mp.substr(pos+1);
	}
	if (bp)
		return bp->get_type_name();
	return "unnamed";
}

std::string provider::get_parent_type() const
{
	return "align_group";
}

// return a path in the main menu to select the gui
std::string provider::get_menu_path() const
{
	return "";
}

// return a shortcut to activate the gui without menu navigation
shortcut provider::get_shortcut() const
{
	return shortcut();
}

// use this method to recreate the gui, dont call create gui directly
void provider::recreate_gui()
{
	if (!parent_group)
		return;
	int selected_child_index = parent_group->get_selected_child_index();
	parent_group->remove_all_children();
	create_gui();
	if (selected_child_index != -1 && selected_child_index < (int) (parent_group->get_nr_children()))
		parent_group->select_child(selected_child_index);
}

std::set<provider*>& ref_providers()
{
	static std::set<provider*> providers;
	return providers;
}

/*
cgv::os::mutex& ref_mutex()
{
	static cgv::os::mutex m;
	return m;
}
*/

void trigger_callback(double,double)
{

	//	ref_mutex().lock();

	std::set<provider*>& ps = ref_providers();
	while (ps.size() > 0) {
		provider* p = *ps.begin();
		p->recreate_gui();
		ps.erase(p);
	}

	//	ref_mutex().unlock();

}

trigger& ref_one_shot_trigger()
{
	static trigger t;
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		connect(t.shoot, &trigger_callback);
	}
	return t;
}

// ensure to remove posted recreation callbacks
provider::~provider()
{
	std::set<provider*>& ps = ref_providers();
	if (ps.empty())
		return;

	//	ref_mutex().lock();
	
	if (ps.find(this) != ps.end())
		ps.erase(this);

	//	ref_mutex().unlock();

} 

// schedule the recreation of the gui for the next time the program is idle
void provider::post_recreate_gui()
{
	if (parent_provider)
		parent_provider->post_recreate_gui();
	else {

		// ref_mutex().lock();

		std::set<provider*>& ps = ref_providers();
		if (ps.find(this) == ps.end()) {
			bool dont_insert = false;
			if (!ref_one_shot_trigger().is_scheduled())
				if (!ref_one_shot_trigger().schedule_one_shot(0))
					dont_insert = true;
			if (!dont_insert)
				ps.insert(this);
		}

		// ref_mutex().unlock();

	}
}


	}
}
