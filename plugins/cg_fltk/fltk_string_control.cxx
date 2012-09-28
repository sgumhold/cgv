#include "fltk_string_control.h"
#include "fltk_driver_registry.h"

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Input.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif

#include <iostream>


void input_cb(fltk::Widget* w, void* input_ptr)
{
	fltk_string_control*  fsc = static_cast<fltk_string_control*>(static_cast<cgv::base::base*>(input_ptr));
	fltk::Input* fI = static_cast<fltk::Input*>(w);
	fsc->set_new_value(fI->value());
	if (fsc->check_value(*fsc)) {
		std::string tmp_value = fsc->get_value();
		fsc->public_set_value(fsc->get_new_value());
		fsc->set_new_value(tmp_value);
		fsc->value_change(*fsc);
	}
	if (fsc->get_value() != fI->value())
		fI->value(fsc->get_value().c_str());
}

/// construct from label, value reference and dimensions
fltk_string_control::fltk_string_control(const std::string& _label, std::string& value, 
										 abst_control_provider* acp, int x, int y, int w, int h)
	: control<std::string>(_label, acp, &value)
{
	fI = new CW<fltk::Input>(x,y,w,h,get_name().c_str());
	fI->callback(input_cb,static_cast<cgv::base::base*>(this));
	update();
}

/// destruct fltk input
fltk_string_control::~fltk_string_control()
{
	delete fI;
}

/// give access to the protected value ptr to allow changing the value
void  fltk_string_control::public_set_value(const std::string& val)
{
	set_value(val);
}

/// returns "fltk_string_control"
std::string fltk_string_control::get_type_name() const
{
	return "fltk_string_control";
}

/// updates the fltk::Input widget in case the controled value has been changed externally
void fltk_string_control::update()
{
	fI->value(get_value().c_str());
}
/// only uses the implementation of fltk_base
std::string fltk_string_control::get_property_declarations()
{
	return fltk_base::get_property_declarations();
}
/// abstract interface for the setter
bool fltk_string_control::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	return fltk_base::set_void(fI, this, property, value_type, value_ptr);
}
/// abstract interface for the getter
bool fltk_string_control::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return fltk_base::get_void(fI, this, property, value_type, value_ptr);
}
/// return a fltk::Widget pointer
void* fltk_string_control::get_user_data() const
{
	return static_cast<fltk::Widget*>(fI);
}

struct string_control_factory : public abst_control_factory
{
	control_ptr create(const std::string& label, 
		void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
		const std::string& gui_type, int x, int y, int w, int h)
	{
		if (value_type == "string")
			return control_ptr(new fltk_string_control(label, 
				*static_cast<std::string*>(value_ptr), acp, x, y, w, h));
		return control_ptr();
	}
};

control_factory_registration<string_control_factory> string_control_fac_reg;