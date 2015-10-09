#include "fltk_tree_group.h"

#include <fltk/ItemGroup.h>
#include "fltk_driver.h"
#include <cgv/utils/scan.h>
#include <cgv/base/attach_slot.h>
#include <cgv/gui/provider.h>
#include <cgv/utils/convert_string.h>
#include <fltk/events.h>

using namespace cgv::base;
using namespace cgv::utils;
using namespace cgv::gui;

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/ToggleItem.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif


class fltk_item : public cgv::gui::button, public attach_slot, public fltk_base
{
public:
	bool selected;
	/// store pointer to fltk Button
	CW<fltk::Item>* fI;
	/// callback for button press events
	static void item_cb(fltk::Widget* w, void* item_ptr)
	{
		fltk_item* i = static_cast<fltk_item*>(static_cast<cgv::base::base*>(item_ptr));
		i->click(*i);
	}

	fltk_item(const std::string& _label, int x, int y, int w, int h) : cgv::gui::button(_label)
	{
		selected = false;
		fI = new CW<fltk::Item>(get_name().c_str());
		fI->callback(item_cb,static_cast<cgv::base::base*>(this));
	}
	std::string get_type_name() const { return "fltk_item"; }
	void update() { fI->redraw(); }
	std::string get_property_declarations() { return fltk_base::get_property_declarations(); }
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr) { return fltk_base::set_void(fI, this, property, value_type, value_ptr); }
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr) { return fltk_base::get_void(fI, this, property, value_type, value_ptr); }
	/// return a fltk::Widget pointer
	void* get_user_data() const { return static_cast<fltk::Widget*>(fI); }

};



class fltk_item_group : public cgv::gui::gui_group, public fltk_gui_group, public attach_slot, public CW<fltk::ItemGroup>
{
public:
	bool selected, open;
	fltk_item_group(int x, int y, int w, int h, const std::string& _name) : cgv::gui::gui_group(_name)
	{
		selected = open = false;
		label(get_name().c_str());
		user_data(static_cast<cgv::base::base*>(this));
	}
	std::string get_type_name() const {return "fltk_item_group"; }
	/// return a fltk::Widget pointer that can be cast into a fltk::Group
	void* get_user_data() const { return (fltk::Widget*) this; }
	/// put default sizes into dimension fields and set inner_group to be active
	void prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h) { }
	/// align last element and add element to group
	void finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& align, cgv::base::base_ptr element) 
	{
		add(static_cast<Widget*>(element->get_user_data()));
		cgv::base::group::append_child(element); 
	} 
	/// only uses the implementation of fltk_base
	std::string get_property_declarations() { return fltk_base::get_property_declarations(); }
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
	{
		return fltk_base::set_void(this, this, property, value_type, value_ptr);
	}
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr)
	{
		return fltk_base::get_void(this, this, property, value_type, value_ptr);
	}
	/// return whether the given child is selected
	bool is_selected(base_ptr c) const
	{
		return static_cast<fltk::Widget*>(c->get_user_data())->selected();
	}
	/// remove all elements of the vector that point to child, return the number of removed children
	unsigned int remove_child(base_ptr child) { 
		return static_cast<fltk_gui_group*>(this)->remove_child(cgv::gui::gui_group_ptr(this), child); 
	}
	/// remove all children
	void remove_all_children() { return static_cast<fltk_gui_group*>(this)->remove_all_children(cgv::gui::gui_group_ptr(this)); }
	/// overload to trigger initialization of alignment
	void remove_all_children(cgv::gui::gui_group_ptr ggp) { fltk_gui_group::remove_all_children(ggp); }
	/// add a new group to the given parent group
	gui_group_ptr add_group(const std::string& label, const std::string& group_type, const std::string& options, const std::string& align)
	{
		ensure_lock();
		int x,y,w,h;
		prepare_new_element(this,x,y,w,h);
		gui_group_ptr gg = gui_group_ptr(new fltk_item_group(x,y,w,h,label));
		if (!gg.empty()) {
			if (!options.empty())
				gg->multi_set(options, true);
			finalize_new_element(this, align, gg);
		}
		return gg;
	}
	/// add a newly created button to the group
	button_ptr add_button(const std::string& label, const std::string& options, const std::string& align)
	{
		ensure_lock();
		int x,y,w,h;
		prepare_new_element(this,x,y,w,h);
		button_ptr b(new fltk_item(label,x,y,w,h));

		if (!b.empty()) {
			if (!options.empty())
				b->multi_set(options, true);
			finalize_new_element(this,align, b);
		}
		return b;
	}
	CGVBrowser* find_browser()
	{
		fltk::Group* g = this;
		do {
			CGVBrowser* B = dynamic_cast<CGVBrowser*>(g);
			if (B)
				return B;
			g = g->parent();
		}
		while (g);
		return 0;
	}
	/// try to open given child group and return whether this was successful
	bool open_child_group(gui_group_ptr g)
	{
		CGVBrowser* B = find_browser();
		if (!B)
			return false;

		if (!goto_item(B, static_cast<fltk::Widget*>(g->get_user_data())))
			return false;
		B->set_item_opened(true);
		B->ensure_state_change();
		return true;
	}

	/// try to close given child group and return whether this was successful
	bool close_child_group(gui_group_ptr g)
	{
		CGVBrowser* B = find_browser();
		if (!B)
			return false;

		if (!goto_item(B, static_cast<fltk::Widget*>(g->get_user_data())))
			return false;
		B->set_item_opened(false);
		B->ensure_state_change();
		return true;
	}

};

void browser_callback(fltk::Widget* w, void*)
{
	CGVBrowser* b = static_cast<CGVBrowser*>(w);
	if (!b->item())
		return;
	if (!b->ensure_state_change()) {
		button_ptr B = static_cast<cgv::base::base*>(b->item()->user_data())->cast<button>();
		if (B)
			B->click(*B);
	}
}

CGVBrowser::CGVBrowser(int x,int y,int w,int h,const char *l) : CW<fltk::Browser>(x,y,w,h,l)
{
	type(MULTI);
	when(fltk::WHEN_ENTER_KEY_CHANGED);
}

bool CGVBrowser::ensure_state_change()
{
	bool change_detected = false;
	gui_group_ptr ggp = static_cast<cgv::base::base*>(user_data())->cast<gui_group>();
	if (!item())
		return false;
	base_ptr c(static_cast<cgv::base::base*>(item()->user_data()));
	if (item_is_parent()) {
		fltk_item_group* g = static_cast<fltk_item_group*>(item());
		if (item()->selected() != g->selected) {
			g->selected = item()->selected();
			ggp->on_selection_change(c, g->selected);
			change_detected = true;
		}
		if (item_is_open() != g->open) {
			g->open = item_is_open();
			ggp->on_open_state_change(c->cast<gui_group>(), g->open);
			change_detected = true;
		}
	}
	else {
		fltk_item* i = static_cast<cgv::base::base*>(item()->user_data())->get_interface<fltk_item>();
		if (item()->selected() != i->selected) {
			i->selected = item()->selected();
			ggp->on_selection_change(c, i->selected);
			change_detected = true;
		}
	}
	return change_detected;
}

int CGVBrowser::handle(int event)
{
	int res = fltk::Browser::handle(event);
	if (event == fltk::KEY && fltk::event_key() == fltk::LeftKey)
		ensure_state_change();
	return res;
}


fltk_tree_group::fltk_tree_group(int x, int y, int w, int h, const std::string& _name) : 
	cgv::gui::gui_group(_name), CGVBrowser(x, y, w, h, "")
{
	label(get_name().c_str());
	callback(browser_callback, static_cast<cgv::base::base*>(this));
	indented(true);
}

std::string fltk_tree_group::get_type_name() const
{
	return "fltk_tree_group";
}

/// only uses the implementation of fltk_base
std::string fltk_tree_group::get_property_declarations()
{
	std::string decl = fltk_base::get_property_declarations();
	decl+=";nr_columns:uint32";
	for (unsigned ci=0; ci<column_infos.size(); ++ci) {
		decl += ";";
		decl += "column_heading_";
		decl += to_string(ci);
		decl += ":string;column_width_";
		decl += to_string(ci);
		decl += ":uint32";
	}
	return decl;
}

void fltk_tree_group::ensure_column_infos()
{
	const char **col_labs = new const char*[column_infos.size()+1];
	int* col_widths = new int[column_infos.size()];
	unsigned i;
	for (i = 0; i < column_infos.size(); ++i) {
		col_labs[i] = column_infos[i].first.c_str();
		col_widths[i] = column_infos[i].second;
	}
	col_labs[i] = 0;
	column_labels(col_labs);
	column_widths(col_widths);
	delete [] col_labs;
	delete [] col_widths;
}

/// abstract interface for the setter
bool fltk_tree_group::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "nr_columns") {
		unsigned n;
		get_variant(n, value_type, value_ptr);
		column_infos.resize(n);
	}
	else if (property.substr(0,15) == "column_heading_") {
		unsigned ci = atoi(property.substr(15).c_str());
		if (ci >= column_infos.size())
			column_infos.resize(ci+1);
		get_variant(column_infos[ci].first, value_type, value_ptr);
		ensure_column_infos();
	}
	else if (property.substr(0,13) == "column_width_") {
		unsigned ci = atoi(property.substr(13).c_str());
		if (ci >= column_infos.size())
			column_infos.resize(ci+1);
		get_variant(column_infos[ci].second, value_type, value_ptr);
		ensure_column_infos();
	}
	else 
		return fltk_base::set_void(this, this, property, value_type, value_ptr);
	return true;
}


/// abstract interface for the getter
bool fltk_tree_group::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "nr_columns") {
		unsigned n = (unsigned)column_infos.size();
		set_variant(n, value_type, value_ptr);
	}
	else if (property.substr(0,15) == "column_heading_") {
		unsigned ci = atoi(property.substr(15).c_str());
		if (ci >= column_infos.size())
			return "";
		set_variant(column_infos[ci].first, value_type, value_ptr);
	}
	else if (property.substr(0,15) == "column_width_") {
		unsigned ci = atoi(property.substr(13).c_str());
		if (ci >= column_infos.size())
			return 0;
		set_variant(column_infos[ci].second, value_type, value_ptr);
	}
	else 
		return fltk_base::get_void(this, this, property, value_type, value_ptr);

	return true;
}

/// return a fltk::Widget pointer that can be cast into a fltk::Group
void* fltk_tree_group::get_user_data() const
{
	return (fltk::Widget*)(this);
}


/// return whether several children of the group can be selected at the same time
bool fltk_tree_group::multiple_selection() const
{
	return true;
}

/// same as version with child index
void fltk_tree_group::select_child(base_ptr ci, bool exclusive)
{
	select(static_cast<fltk::Widget*>(ci->get_user_data()), 1, 1);
	if (exclusive)
		select_only_this(1);
}

/// same as version with child index
bool fltk_tree_group::unselect_child(base_ptr ci)
{
	return select(static_cast<fltk::Widget*>(ci->get_user_data()), 0, 1);
}

/// return whether the given child is selected
bool fltk_tree_group::is_selected(base_ptr c) const
{
	return static_cast<fltk::Widget*>(c->get_user_data())->selected();
}

/// returns whether open and close of sub groups is allowed
bool fltk_tree_group::can_open_and_close() const
{
	return true;
}


bool goto_item(fltk::Browser* B, fltk::Widget* e)
{
	if (!B->item()) B->goto_top();
	fltk::Widget* i = B->item();
	fltk::Widget* c;
	while (B->item()!=e) {
		c = B->next_visible();
		if (c==i || (!c && (c=B->goto_top())==i)) return false;
	}
	return true;
}

/// try to open given child group and return whether this was successful
bool fltk_tree_group::open_child_group(gui_group_ptr g)
{
	if (!goto_item(this, static_cast<fltk::Widget*>(g->get_user_data())))
		return false;
	set_item_opened(true);
	ensure_state_change();
	return true;
}

/// try to close given child group and return whether this was successful
bool fltk_tree_group::close_child_group(gui_group_ptr g)
{
	if (!goto_item(this, static_cast<fltk::Widget*>(g->get_user_data())))
		return false;
	set_item_opened(false);
	ensure_state_change();
	return true;
}

/// return whether the given child is open
bool fltk_tree_group::is_open_child_group(gui_group_ptr g) const
{
	return static_cast<fltk::Widget*>(g->get_user_data())->flag(fltk::OPENED);
}

/// put default sizes into dimension fields and set inner_group to be active
void fltk_tree_group::prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h)
{
}


/// overload to trigger initialization of alignment
void fltk_tree_group::remove_all_children(cgv::gui::gui_group_ptr ggp)
{
	fltk_gui_group::remove_all_children(ggp);
}


/// align last element and add element to group
void fltk_tree_group::finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& _align, cgv::base::base_ptr element)
{
	add(static_cast<Widget*>(element->get_user_data()));
	cgv::base::group::append_child(element);
}


/// add a new group to the given parent group
gui_group_ptr fltk_tree_group::add_group(const std::string& label, const std::string& group_type, const std::string& options, const std::string& align)
{
	ensure_lock();
	int x,y,w,h;
	prepare_new_element(this,x,y,w,h);
	gui_group_ptr gg = gui_group_ptr(new fltk_item_group(x,y,w,h,label));
	if (!gg.empty()) {
		if (!options.empty())
			gg->multi_set(options, true);
		finalize_new_element(this, align, gg);
	}
	return gg;
}

/// add a newly created decorator to the group
base_ptr fltk_tree_group::add_decorator(const std::string& label, const std::string& decorator_type, const std::string& options, const std::string& align)
{
	return base_ptr();
}

/// add a newly created button to the group
button_ptr fltk_tree_group::add_button(const std::string& label, const std::string& options, const std::string& align)
{
	ensure_lock();
	int x,y,w,h;
	prepare_new_element(this,x,y,w,h);
	button_ptr b(new fltk_item(label,x,y,w,h));

	if (!b.empty()) {
		if (!options.empty())
			b->multi_set(options, true);
		finalize_new_element(this,align, b);
	}
	return b;
}

/// add a newly created view to the group
view_ptr fltk_tree_group::add_view_void(const std::string& label, const void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align)
{
	return view_ptr();
}

/// add a newly created control to the group
control_ptr fltk_tree_group::add_control_void(const std::string& label, void* value_ptr, abst_control_provider* acp, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align, void* user_data)
{
	return control_ptr();
}


void fltk_tree_group::register_object(base_ptr object, const std::string& options)
{
	if (!cgv::utils::is_element(get_name(),options))
		return;
	provider* p = object->get_interface<cgv::gui::provider>();
	if (p && get_provider_parent(p).empty()) {
		set_provider_parent(p,gui_group_ptr(this));
		p->create_gui();
	}
}

///
void fltk_tree_group::unregister_object(cgv::base::base_ptr object, const std::string& options)
{
	if (!options.empty() && !cgv::utils::is_element(get_name(),options))
		return;

	provider* p = object->get_interface<cgv::gui::provider>();
	if (p && get_provider_parent(p) == this)
		remove_all_children();
}


