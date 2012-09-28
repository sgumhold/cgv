#include "fltk_string_view.h"
#include "fltk_driver_registry.h"

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Output.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif
#include <iostream>

/// construct from label, value reference and dimensions
fltk_string_view::fltk_string_view(const std::string& _label, const std::string& value, int x, int y, int w, int h)
	: view<std::string>(_label, value)
{
	fO = new CW<fltk::Output>(x,y,w,h,get_name().c_str());
	fO->user_data(static_cast<cgv::base::base*>(this));
	update();
}

/// returns "fltk_string_view"
std::string fltk_string_view::get_type_name() const
{
	return "fltk_string_view";
}
/// get the current value and view it
void fltk_string_view::update()
{
	fO->value(value_ptr->c_str());
}
/// only uses the implementation of fltk_base
std::string fltk_string_view::get_property_declarations()
{
	return fltk_base::get_property_declarations();
}
/// abstract interface for the setter
bool fltk_string_view::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	return fltk_base::set_void(fO, this, property, value_type, value_ptr);
}
/// abstract interface for the getter
bool fltk_string_view::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return fltk_base::get_void(fO, this, property, value_type, value_ptr);
}
/// return a fltk::Widget pointer
void* fltk_string_view::get_user_data() const
{
	return static_cast<fltk::Widget*>(fO);
}

struct string_view_factory : public abst_view_factory
{
	view_ptr create(const std::string& label, 
		const void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, int x, int y, int w, int h)
	{
		if (value_type == "string")
			return view_ptr(new fltk_string_view(label, 
				*static_cast<const std::string*>(value_ptr), x, y, w, h));
		return view_ptr();
	}
};

view_factory_registration<string_view_factory> string_view_fac_reg;