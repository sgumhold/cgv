#include <cgv/gui/provider.h>
#include <cgv/gui/gui_driver.h>
#include <cgv/gui/trigger.h>
#include <cgv/base/base_generator.h>

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

/// concatenate names in string to enum declaration and optionally prepend or append given additional names 
std::string provider::concat_enum_def(const std::vector<std::string>& names, const std::string& additional_first_name, const std::string& additional_last_name)
{
	std::string result = "enums='";
	if (!additional_first_name.empty())
		result += additional_first_name + "=-1,";
	for (unsigned i = 0; i < names.size(); ++i) {
		if (i > 0)
			result += ',';
		result += names[i];
	}
	if (!additional_last_name.empty()) {
		result += ',';
		result += additional_last_name;
	}
	result += "'";
	return result;
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
void update_group_members(cgv::base::group_ptr gp)
{
	if (!gp)
		return;
	for (unsigned i=0; i<gp->get_nr_children(); ++i) {
		base_ptr bp = gp->get_child(i);
		abst_view* v = bp->get_interface<abst_view>();
		if (v)
			v->update();
		cgv::base::group_ptr cgp = bp->cast<cgv::base::group>();
		if (cgp)
			update_group_members(cgp);
	}
}

/// call this to update all views and controls of all member
void provider::update_all_members()
{
	update_group_members(parent_group);
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
void provider::integrate_object_gui(base_ptr object)
{
	provider* p = object->get_interface<provider>();
	if (!p)
		return;
	p->set_parent(parent_group);
	p->parent_provider = this;
}

// inline the gui of another object that must be derived from provider.
void provider::inline_object_gui(base_ptr object)
{
	provider* p = object->get_interface<provider>();
	if (!p)
		return;
	if (p->get_parent_group() != parent_group) {
		p->set_parent(parent_group);
		p->parent_provider = this;
	}
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

bool provider::add_tree_node(const std::string& label, bool& toggle, int level, const std::string& a, gui_group_ptr ggp)
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

std::map<std::pair<const void*,int>, bool>& get_tree_node_toggle_map()
{
	static std::map<std::pair<const void*,int>, bool> tree_node_toggle_map;
	return tree_node_toggle_map;
}

///
bool provider::begin_tree_node_void(const std::string& label, const void* value_ptr, int index, bool initial_visibility, const std::string& options, gui_group_ptr ggp)
{
	if (!ggp)
		ggp = parent_group;
	bool decorated = true;	  cgv::base::has_property(options, "decorated", decorated, true);
	int level = 2;			  cgv::base::has_property(options, "level", level, true);
	std::string align("\n");  cgv::base::has_property(options, "align", align, true);
	std::string child_opt;    cgv::base::has_property(options, "options", child_opt, true);
	std::string button_opt;   cgv::base::has_property(options, "button_options", button_opt, true);
	int size = 20;/*- 4 *level*/;    cgv::base::has_property(options, "size", size, true);
	int relative_offset = 12; cgv::base::has_property(options, "relative_offset", relative_offset, true);
	int off  = size+relative_offset;

	if (!button_opt.empty())
		button_opt = std::string(";")+button_opt;
	button_opt = std::string("w=")+cgv::utils::to_string(size) + button_opt;

	std::map<std::pair<const void*,int>, bool>::iterator it = get_tree_node_toggle_map().find(std::pair<const void*,int>(value_ptr,index));
	if (it == get_tree_node_toggle_map().end())
		get_tree_node_toggle_map()[std::pair<const void*,int>(value_ptr,index)] = initial_visibility;

	bool& toggle = get_tree_node_toggle_map()[std::pair<const void*,int>(value_ptr,index)];

	data::ref_ptr<control<bool> > control_ptr;
	if (decorated) {
		ggp->align(std::string("%x-=")+cgv::utils::to_string(off));
		control_ptr = ggp->add_control(std::string(toggle ? "@-6thinminus" : "@-6thinplus"), toggle, "toggle", button_opt, " ");

		if (!child_opt.empty())
			child_opt = std::string(";")+child_opt;
		child_opt = std::string("level=")+cgv::utils::to_string(level) + child_opt;
		ggp->add_decorator(label, "heading", child_opt, align);
	}
	else {
		control_ptr = ggp->add_control(std::string(toggle ? "@-6thinminus" : "@-6thinplus"), toggle, "toggle", button_opt, align);
	}

	if (control_ptr) {
		connect_copy(control_ptr->value_change,	rebind(static_cast<provider*>(this), &provider::post_recreate_gui));
		// if this class is derived from cgv::base::base
		cgv::base::base* bp = dynamic_cast<cgv::base::base*>(this);
		// connect to on_set callback
		if (bp) {
			connect_copy(control_ptr->value_change, rebind(bp, &cgv::base::base::on_set, cgv::signal::_c((void*)(&toggle))));
		}
	}
	return toggle;
}

///
bool& provider::ref_tree_node_visible_flag_void(const void* value_ptr, int index)
{
	auto& map = get_tree_node_toggle_map();
	std::pair<const void*, int> key(value_ptr, index);
	auto iter = map.find(key);
	if (iter == map.end())
		return map[key] = false;
	else
		return iter->second;
}

///
void provider::end_tree_node_void(const void* value_ptr, int)
{
}

///
bool provider::is_tree_node_visible_void(const void* value_ptr, int index) const
{
	return get_tree_node_toggle_map()[std::pair<const void*,int>(value_ptr,index)];
}

///
void provider::set_tree_node_visibility_void(const void* value_ptr, int index, bool is_visible)
{
	bool& toggle = get_tree_node_toggle_map()[std::pair<const void*,int>(value_ptr,index)];
	if (toggle != is_visible) {
		toggle = is_visible;
		post_recreate_gui();
	}
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
	if (!parent_group.empty()) {
		parent_group->remove_all_children();
		parent_group->release_all_managed_objects();
	}
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
		size_t pos = mp.find_last_of('/');
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
/// ensure that my UI is selected in the closest parent that is a tab group
bool provider::ensure_selected_in_tab_group_parent()
{
	cgv::gui::gui_group_ptr my_group = get_parent_group();
	if (my_group) {
		cgv::gui::gui_group_ptr tab_group = my_group->get_parent()->cast<cgv::gui::gui_group>();
		if (tab_group) {
			cgv::base::base_ptr c = my_group;
			if (c) {
				tab_group->select_child(c, true);
				return true;
			}
		}
	}
	return false;
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
	// ensure that layout is available
	parent_group->set("dolayout", true);
	int xscroll = parent_group->get<int>("xscroll");
	int yscroll = parent_group->get<int>("yscroll");
	remove_all_elements();
	create_gui();
	parent_group->set("dolayout", true);
	parent_group->set("xscroll", xscroll);
	parent_group->set("yscroll", yscroll);
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
		if (parent_group) {
			// ref_mutex().lock();
			std::set<provider*>& ps = ref_providers();
			if (ps.find(this) == ps.end()) {
				bool dont_insert = false;
				if (!ref_one_shot_trigger().is_scheduled() && get_trigger_server())
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
}
