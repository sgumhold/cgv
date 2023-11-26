#pragma once

#include <cgv/gui/control.h>

#include "control_base.h"
#include "input_control.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

/** implements a control for string values with the fltk::Input class */
struct CGV_API g2d_string_control : public cgv::gui::control<std::string> {
	bool in_callback = false;
	/// a fltk::Input is used to implement the string control
	std::shared_ptr<input_control> gl_control;
	/// construct from label, value reference and dimensions
	g2d_string_control(const std::string& label, std::string& value, cgv::g2d::irect rectangle);
	/// destruct fltk input
	~g2d_string_control() {}
	/// give access to the protected value ptr to allow changing the value
	void public_set_value(const std::string& value);
	/// returns "fltk_string_control"
	std::string get_type_name() const;
	/// updates the fltk::Input widget in case the controled value has been changed externally
	void update();
	/// return a fltk::Widget pointer
	void* get_user_data() const;

	std::shared_ptr<input_control> get_gl_control();
};

}
}

#include <cgv/config/lib_end.h>
