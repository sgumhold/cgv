#pragma once

#include <cgv/gui/control.h>

#include "control_base.h"
#include "input_control.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

/// Implements a control for string values with the cg::g2d::input_control class
struct CGV_API g2d_string_control : public cgv::gui::control<std::string> {
	bool in_callback = false;
	/// a cg::g2d::input_control is used to implement the string control
	std::shared_ptr<input_control> gl_control;
	/// construct from label, value reference and rectangle
	g2d_string_control(const std::string& label, std::string& value, cgv::g2d::irect rectangle);
	/// destruct
	~g2d_string_control() {}
	/// give access to the protected value ptr to allow changing the value
	void public_set_value(const std::string& value) { set_value(value); }
	/// return "g2d_string_control"
	std::string get_type_name() const { return "g2d_string_control"; }
	/// update the cg::g2d::input_control in case the controled value has been changed externally
	void update();
	/// return a fltk::Widget pointer
	void* get_user_data() const { return nullptr; }
	/// return a pointer to the underlying cg::g2d::input_control
	std::shared_ptr<input_control> get_gl_control() { return gl_control; }
};

}
}

#include <cgv/config/lib_end.h>
