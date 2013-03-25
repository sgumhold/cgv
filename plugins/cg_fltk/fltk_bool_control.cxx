#include "fltk_bool_control.h"

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/ToggleButton.h>
#include <fltk/CheckButton.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif
#include <iostream>

void bool_button_cb(fltk::Widget* w, void* button_ptr)
{
	fltk_bool_control<CW<fltk::ToggleButton> >*  fbc = static_cast<fltk_bool_control<CW<fltk::ToggleButton> >*>(
		static_cast<cgv::base::base*>(button_ptr));
	fltk::Button* fB = static_cast<fltk::Button*>(w);
	fbc->set_new_value(fB->value());
	if (fbc->check_value(*fbc)) {
		bool tmp_value = fbc->get_value();
		fbc->public_set_value(fbc->get_new_value());
		fbc->set_new_value(tmp_value);
		if (fB->value() != fbc->get_value())
			fB->value(fbc->get_value());
		fbc->value_change(*fbc);
	}
	else
		if (fB->value() != fbc->get_value())
			fB->value(fbc->get_value());
}

template <typename FB>
fltk_bool_control<FB>::fltk_bool_control(const std::string& _label, 
		bool& value, abst_control_provider* acp, int x, int y, int w, int h) : control<bool>(_label,acp,&value)
{
	fB = new FB(x,y,w,h, get_name().c_str());
	cgv::base::base* bp = this;
	fB->callback(bool_button_cb,bp);
	fltk_base* fb = bp->get_interface<fltk_base>();
	update();
}
/// destruct fltk control
template <typename FB>
fltk_bool_control<FB>::~fltk_bool_control()
{
	delete fB;
}



/// give access to the protected value ptr to allow changing the value
template <typename FB>
void fltk_bool_control<FB>::public_set_value(bool b)
{
	set_value(b);
}

/// returns "fltk bool control"
template <typename FB>
std::string fltk_bool_control<FB>::get_type_name() const
{
	return "fltk bool button";
}

/// updates the fltk Button widget in case the controled value has been changed externally
template <typename FB>
void fltk_bool_control<FB>::update()
{
	fB->value(get_value());
}

/// only uses the implementation of fltk_base
template <typename FB>
std::string fltk_bool_control<FB>::get_property_declarations()
{
	return fltk_base::get_property_declarations();
}

/// abstract interface for the setter
template <typename FB>
bool fltk_bool_control<FB>::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	return fltk_base::set_void(fB, this, property, value_type, value_ptr);
}

/// abstract interface for the getter
template <typename FB>
bool fltk_bool_control<FB>::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return fltk_base::get_void(fB, this, property, value_type, value_ptr);
}

/// return a fltk::Widget pointer
template <typename FB>
void* fltk_bool_control<FB>::get_user_data() const
{
	return static_cast<fltk::Widget*>(fB);
}

cgv::gui::control_ptr bool_control_factory::create(const std::string& label, 
			void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
			const std::string& gui_type, int x, int y, int w, int h)
{
	if (value_type != "bool")
		return cgv::gui::control_ptr();
	if (gui_type == "check" || gui_type.empty()) {
		fltk_bool_control<CW<fltk::CheckButton> >* c = 
			new fltk_bool_control<CW<fltk::CheckButton> >(
						label, *static_cast<bool*>(value_ptr), acp, x, y, w, h);
		c->fB->flags(c->fB->flags()|fltk::ALIGN_LEFT);
		return cgv::gui::control_ptr(c);
	}
	else if (gui_type == "toggle")
		return control_ptr(new fltk_bool_control<CW<fltk::ToggleButton> >(
		label, *static_cast<bool*>(value_ptr), acp, x, y, w, h));
	return control_ptr();
}

control_factory_registration<bool_control_factory> bool_control_fac_reg;



