#include "fltk_tab_group.h"
#include "fltk_dragger.h"
#include <cgv/utils/scan.h>
#include <cgv/base/base_generator.h>
#include <cgv/gui/provider.h>
#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/TabGroup.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif

using namespace cgv::base;
using namespace cgv::gui;

fltk_tab_group::fltk_tab_group(int x, int y, int w, int h, const std::string& _name) 
: cgv::gui::gui_group(_name.c_str())
{
	tab_group = new CG<fltk::TabGroup>(x,y,w,h,get_name().c_str());
	tab_group->callback(fltk_select_cb,this);
	last_selected_child = -1;
}

/// destruct fltk group realization
fltk_tab_group::~fltk_tab_group()
{
	delete tab_group;
}


/// return a pointer to the fltk tab group
fltk::TabGroup* fltk_tab_group::get_fltk_tab_group() const
{
	return tab_group;
}

/// only uses the implementation of fltk_base
std::string fltk_tab_group::get_property_declarations()
{
	return fltk_base::get_property_declarations()+";selected:string";
}
/// abstract interface for the setter
bool fltk_tab_group::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "selected") {
		std::string name = variant<std::string>::get(value_type, value_ptr);
		base_ptr b = find_element(name);
		if (b)
			select_child(b, true);
		return true;
	}
	return fltk_base::set_void(tab_group, this, property, value_type, value_ptr);
}
/// abstract interface for the getter
bool fltk_tab_group::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "selected") {
		std::string name;
		base_ptr b = get_selected_child();
		if (b) {
			if (b->get_named())
				name = b->get_named()->get_name();
			else
				name = b->get_type_name();
		}
		variant<std::string>::set(name, value_type, value_ptr);
		return true;
	}
	return fltk_base::get_void(tab_group, this, property, value_type, value_ptr);
}


struct base_access : public cgv::base::base
{
	/// used for internal purposes
	void* get_user_data_access() { return get_user_data(); }
};

void fltk_tab_group::fltk_select_cb(fltk::Widget* w, void*ud)
{
	fltk_tab_group* g = static_cast<fltk_tab_group*>(ud);
	base_ptr c = g->get_selected_child();
	if (g->last_selected_child != -1)
		g->tab_group->value(g->last_selected_child);
	g->select_child(c, true);
}

/// return the index of the currently selected child.
int fltk_tab_group::get_selected_child_index() const
{
	return tab_group->value();
}

void fltk_tab_group::select_child(base_ptr c, bool exclusive)
{
//	std::cout << "select " << c->get_type_name().c_str() << std::endl;
	gui_group::select_child(c, exclusive);
	fltk::Widget* w  = static_cast<fltk::Widget*>(c->get_user_data());
	if (w == 0) {
		for (int i=0; i<tab_group->children(); ++i) {
			void* ud = tab_group->child(i)->user_data();
			if (ud == c.operator ->()) {
				tab_group->selected_child(tab_group->child(i));
				break;
			}
		}
	}
	else {
		while (w->parent() && w->parent() != tab_group)
			w = w->parent();
		if (w)
			tab_group->selected_child(w);
		else
			std::cerr << "could not select child" << std::endl;
	}
	last_selected_child = tab_group->value();
}
/// this virtual method allows to pass application specific data for internal purposes
void* fltk_tab_group::get_user_data() const
{
	return static_cast<fltk::Widget*>(tab_group);
}

/// put default sizes into dimension fields and set inner_group to be active
void fltk_tab_group::prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h)
{
	x = y = 0;
	w = tab_group->w();
	h = tab_group->h()-30;
	tab_group->begin();
}
/// align last element and add element to group
void fltk_tab_group::finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& , cgv::base::base_ptr element)
{
	tab_group->end();
	cgv::base::group::append_child(element);
}

///
void fltk_tab_group::unregister_object(cgv::base::base_ptr object, const std::string& options)
{
	if (!options.empty() && !cgv::utils::is_element(get_name(),options))
		return;

	provider* p = object->get_interface<cgv::gui::provider>();
	if (p)
		get_interface<fltk_gui_group>()->remove_child(gui_group_ptr(this),get_provider_parent(p));
}

void fltk_tab_group::register_object(base_ptr object, const std::string& options)
{
	provider* p = object->get_interface<cgv::gui::provider>();
	if (p && get_provider_parent(p).empty()) {
		// construct gui in an align group
		cgv::gui::gui_group_ptr g = add_group(p->get_gui_name(), p->get_parent_type(), "", "");
		set_provider_parent(p,g);
		p->create_gui();

		// construct dragger widget
		fltk::Group* fg = static_cast<fltk::Group*>(
			static_cast<fltk::Widget*>(g->get_user_data()));
		// select newly created child
		select_child(g, true);
		update();
	} 
}
