#include "fltk_dockable_group.h"

fltk_dockable_group::fltk_dockable_group(int x, int y, int w, int h, const std::string& _name) : cgv::gui::gui_group(_name)
{
	group = new CG<DockableGroup>(x,y,w,h,get_name().c_str());
	group->user_data(static_cast<cgv::base::base*>(this));
}

/// destruct group realization
fltk_dockable_group::~fltk_dockable_group()
{
	delete group;
}

/// only uses the implementation of fltk_base
std::string fltk_dockable_group::get_property_declarations()
{
	return fltk_base::get_property_declarations();
}
/// abstract interface for the setter
bool fltk_dockable_group::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	return fltk_base::set_void(group, this, property, value_type, value_ptr);
}
/// abstract interface for the getter
bool fltk_dockable_group::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return fltk_base::get_void(group, this, property, value_type, value_ptr);
}

/// return a fltk::Widget pointer that can be cast into a fltk::Group
void* fltk_dockable_group::get_user_data() const
{
	return static_cast<fltk::Widget*>(group);
}


/// put default sizes into dimension fields and set inner_group to be active
void fltk_dockable_group::prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h)
{
	x = 0;
	y = 0;
	w = 100;
	h = 100;
	group->begin();
}

/// align last element and add element to group
void fltk_dockable_group::finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& _align, cgv::base::base_ptr element)
{
	group->end();
	cgv::base::group::append_child(element);
	int s = -1;
	bool res = false;
	if (_align.size() > 0 && _align[0] == 'R') 
		res = true;
	if (_align.size() > 0) {
		char l = _align[_align.size()-1];
		if (l >= '0' && l <= '3')
			s = l - '0';
	}

	if (get_nr_children() != group->children())
		std::cerr << "fltk_dockable_group has different number of children " 
		<< get_nr_children() << " as fltk representation: " << group->children() 
		<< std::endl;

	fltk::Widget* element_widget = static_cast<fltk::Widget*>(element->get_user_data());
	fltk::Widget* last_widget = group->child(group->children()-1);
	if (element_widget != last_widget)
		std::cerr << "user data of element widget does not point to last element of group after creation." << std::endl;

	if (s == -1)
		group->resizable(element_widget);
	else
		group->dock(element_widget, s, res);

}

