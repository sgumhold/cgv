#pragma once

#include <cgv/gui/control.h>
#include "fltk_base.h"
#include "fltk_driver_registry.h"

#include "lib_begin.h"

/** The control<bool> is implemented with different fltk Button
    widgets. The fltk_bool_control is therefore parameterized over
	 the fltk Button type FB. */
template <typename FB>
struct fltk_bool_control : public control<bool>, public fltk_base
{
	/// pointer to the fltk button	
	FB* fB;
	/// construct from label, value reference and dimensions
	fltk_bool_control(const std::string& _label, bool& value, abst_control_provider* acp, int x, int y, int w, int h);
	/// destruct fltk control
	~fltk_bool_control();
	/// give access to the protected value ptr to allow changing the value
	void public_set_value(bool b);
	/// returns "fltk bool control"
	std::string get_type_name() const;
	/// updates the fltk Button widget in case the controled value has been changed externally
	void update();
	/// only uses the implementation of fltk_base
	std::string get_property_declarations();
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return a fltk::Widget pointer
	void* get_user_data() const;
};

/** the bool control factory creates bool controls with different
    gui types implemented with the fltk bool control template
	 instantiated with different fltk Button widgets.*/
struct CGV_API bool_control_factory : public abst_control_factory
{
	/// creation method
	cgv::gui::control_ptr create(const std::string& label, 
		void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
		const std::string& gui_type, int x, int y, int w, int h);
};

#include <cgv/config/lib_end.h>