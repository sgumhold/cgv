#pragma once

#include <cgv/gui/control.h>
#include "fltk_base.h"

namespace fltk {
	class Input;
}

#include "lib_begin.h"

/** implements a control for string values with the fltk::Input class */
struct CGV_API fltk_string_control : public cgv::gui::control<std::string>, public fltk_base
{
	/// a fltk::Input is used to implement the string control
	fltk::Input* fI;
	/// construct from label, value reference and dimensions
	fltk_string_control(const std::string& _label, std::string& value, 
		cgv::gui::abst_control_provider* acp, int x, int y, int w, int h);
	/// destruct fltk input
	~fltk_string_control();
	/// give access to the protected value ptr to allow changing the value
	void public_set_value(const std::string&);
	/// returns "fltk_string_control"
	std::string get_type_name() const;
	/// updates the fltk::Input widget in case the controled value has been changed externally
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

#include <cgv/config/lib_end.h>
