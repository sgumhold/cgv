#pragma once

#include <cgv/gui/control.h>
#include <cgv/utils/token.h>
#include "fltk_base.h"

namespace fltk {
	class Choice;
}

#include "lib_begin.h"

/** implements a control for enums the fltk::Choice widget*/
struct CGV_API fltk_enum_control : public cgv::gui::control<int>, public fltk_base
{
	/// a fltk::Input is used to implement the string control
	fltk::Choice* fC;
	/// vector of enum strings
	std::vector<std::string> enum_strings;
	/// vector of enum values
	std::vector<int> enum_values;
	int index_to_value(int idx) const;
	int value_to_index(int val) const;
	///
	void parse_enum_declarations(const std::string& enum_declarations);
	/// construct from label, value reference and dimensions
	fltk_enum_control(const std::string& _label, int& value, cgv::gui::abst_control_provider* acp, const std::string& enum_declarations,
					  int x, int y, int w, int h);
	/// destruct enum control
	~fltk_enum_control();
	/// give access to the protected value ptr to allow changing the value
	void public_set_value(int v);
	/// returns "fltk_enum_control"
	std::string get_type_name() const;
	/// updates the fltk::Choice widget in case the controled value has been changed externally
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
