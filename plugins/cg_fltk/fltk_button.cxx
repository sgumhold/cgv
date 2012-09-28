#include "fltk_button.h"

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Button.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif
#include <iostream>

/// callback for button press events
void button_cb(fltk::Widget* w, void* button_ptr)
{
	fltk_button* b = static_cast<fltk_button*>(static_cast<cgv::base::base*>(button_ptr));
	b->click(*b);
}

fltk_button::fltk_button(const std::string& _label, int x, int y, int w, int h) : cgv::gui::button(_label)
{
	fB = new CW<fltk::Button>(x, y, w, h, get_name().c_str());
	fB->callback(button_cb,static_cast<cgv::base::base*>(this));
}

/// destruct fltk button
fltk_button::~fltk_button()
{
	delete fB;
}

/// return "fltk button"
std::string fltk_button::get_type_name() const
{
	return "fltk button";
}

/// triggers a redraw
void fltk_button::update() 
{
	fB->redraw(); 
}

/// only uses the implementation of fltk_base
std::string fltk_button::get_property_declarations()
{
	return fltk_base::get_property_declarations();
}

/// abstract interface for the setter
bool fltk_button::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	return fltk_base::set_void(fB, this, property, value_type, value_ptr);
}

/// abstract interface for the getter
bool fltk_button::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return fltk_base::get_void(fB, this, property, value_type, value_ptr);
}

/// return a fltk::Widget pointer
void* fltk_button::get_user_data() const
{
	return static_cast<fltk::Widget*>(fB);
}

