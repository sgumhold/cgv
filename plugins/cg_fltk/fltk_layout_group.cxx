#include "fltk_layout_group.h"


#include <cgv/utils/scan.h>
#include <cgv/gui/provider.h>

using namespace cgv::base;
using namespace cgv::gui;

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Group.h>
#include <fltk/draw.h>
#include <fltk/Cursor.h>
#include <fltk/events.h>
#include <fltk/ScrollGroup.h>
#include <fltk/PackedGroup.h>
#include <fltk/Button.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif


fltk_layout_group::fltk_layout_group(int x, int y, int w, int h, const std::string& _name) : 
	cgv::gui::gui_group(_name), CG<fltk::Group>(x, y, w, h, "")
{
	label(get_name().c_str());
	user_data(static_cast<cgv::base::base*>(this));
}


/// only uses the implementation of fltk_base
std::string fltk_layout_group::get_property_declarations()
{
	std::string decl = fltk_base::get_property_declarations();
	decl+=";layout:string;border-style:string;";
	if (formatter)
		decl+=formatter->get_property_declarations();

	return decl;
}


/// abstract interface for the setter
bool fltk_layout_group::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "layout") {
		get_variant(formatter_name, value_type, value_ptr);

		if (formatter_name == "table")
			formatter = new layout_table(cgv::base::group_ptr(this));

		if (formatter_name == "inline")
			formatter = new layout_inline(cgv::base::group_ptr(this));
	} else 
	if (property == "border-style") {
		get_variant(border_style, value_type, value_ptr);
		update_box_type();
		redraw();
	} else 
	if (formatter && formatter->set_void(property, value_type, value_ptr))
		return true;
	else {
		fltk_base::set_void(this, this, property, value_type, value_ptr);
	}

	return true;
}


/// abstract interface for the getter
bool fltk_layout_group::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "layout")
		set_variant(formatter_name, value_type, value_ptr);
	else 
	if (property == "border-style")
		set_variant(border_style, value_type, value_ptr);
	else 
	if (formatter && formatter->get_void(property, value_type, value_ptr))
		return true;
	else {
//		if (property == "label") {
//			std::cout << "layout label" << std::endl;
//		}
		return fltk_base::get_void(this, this, property, value_type, value_ptr);
	}
	return true;
}


void fltk_layout_group::update_box_type() 
{
	if (border_style == "sunken")
		box(fltk::DOWN_BOX);
	else
	if (border_style == "lifted")
		box(fltk::UP_BOX);
	else
	if (border_style == "thinsunken")
		box(fltk::THIN_DOWN_BOX);
	else
	if (border_style == "thinlifted")
		box(fltk::THIN_UP_BOX);
	else
	if (border_style == "framed")
		box(fltk::BORDER_BOX);

	// more types can be added using: 
	// http://www.fltk.org/doc-2.0/html/group__boxes.html
}


/// return a fltk::Widget pointer that can be cast into a fltk::Group
void* fltk_layout_group::get_user_data() const
{
	return (fltk::Widget*)(this);
}


/// put default sizes into dimension fields and set inner_group to be active
void fltk_layout_group::prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h)
{
	x = 0;
	y = 0;
	w = 200;
	h = 20;
	begin();
}


/// overload to trigger initialization of alignment
void fltk_layout_group::remove_all_children(cgv::gui::gui_group_ptr ggp)
{
	fltk_gui_group::remove_all_children(ggp);
}


/// align last element and add element to group
void fltk_layout_group::finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& _align, cgv::base::base_ptr element)
{
	end();

	// save the alignment information
	element->set<std::string>("alignment", _align);
	// the default width and height
	element->set<int>("dw", element->get<int>("w"));
	element->set<int>("dh", element->get<int>("h"));
	

	cgv::base::group::append_child(element);
}

void fltk_layout_group::register_object(base_ptr object, const std::string& options)
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
void fltk_layout_group::unregister_object(cgv::base::base_ptr object, const std::string& options)
{
	if (!options.empty() && !cgv::utils::is_element(get_name(),options))
		return;

	provider* p = object->get_interface<cgv::gui::provider>();
	if (p && get_provider_parent(p) == this)
		remove_all_children();
}


void fltk_layout_group::layout()
{
	if (!formatter.empty())
		formatter->resize(w(), h());

	// relayout children
	//for (int i=0; i<fltk::Group::children(); i++)
	//	fltk::Group::child(i)->layout();
//	fltk::Group::layout();
}

