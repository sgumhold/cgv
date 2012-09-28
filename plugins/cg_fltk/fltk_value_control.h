#pragma once

#include <cgv/gui/control.h>
#include "fltk_base.h"
#include "fltk_driver_registry.h"

#include "lib_begin.h"

/// this interface is used to update the value of fltk_value_control s.
struct CGV_API abst_fltk_value_callback
{
	/// interface for value updates independent of the value and fltk Valuator type 
	virtual void update_value_if_valid(double v) = 0;
};

/** The control<T> is implemented with different fltk Valuator
    widgets. The fltk_value_control is parameterized over
	 the fltk Valuator control widget type FC and the type T
	 of the conrolled value. */
template <typename T, typename FC>
struct CGV_API fltk_value_control : public cgv::gui::control<T>, public abst_fltk_value_callback, public fltk_base
{
	/// pointer to the fltk Widget that controls the value
	FC* fC;
	/// construct from label, value reference and dimensions
	fltk_value_control(const std::string& _label, T& value, abst_control_provider* acp, int x, int y, int w, int h);
	/// destruct fltk value control
	~fltk_value_control();
	/// returns "fltk_value_control"
	std::string get_type_name() const;
	/// updates the fltk control widget in case the controled value has been changed externally
	void update();
	/// adds to the implementation of fltk_base based on the control type
	std::string get_property_declarations();
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return a fltk::Widget pointer
	void* get_user_data() const;
	/// interface for value updates independent of the value type T
	void update_value_if_valid(double v);
};

#include <cgv/config/lib_end.h>