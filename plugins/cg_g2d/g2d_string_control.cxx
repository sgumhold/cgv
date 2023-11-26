#include "g2d_string_control.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

void input_cb(control_base* control, void* input_ptr) {
	g2d_string_control* control_ptr = static_cast<g2d_string_control*>(static_cast<cgv::base::base*>(input_ptr));
	control_ptr->in_callback = true;
	//auto control_ptr->get_gl_control();

	 //static_cast<fltk::Input*> =

	//fltk::Input* fI = static_cast<fltk::Input*>(w);
	input_control* gl_control = static_cast<input_control*>(control);

	//fsc->set_new_value(fI->value());
	control_ptr->set_new_value(gl_control->get_value());

	if(control_ptr->check_value(*control_ptr)) {
		std::string tmp_value = control_ptr->get_value();
		control_ptr->public_set_value(control_ptr->get_new_value());
		control_ptr->set_new_value(tmp_value);
		control_ptr->value_change(*control_ptr);
	}

	if(control_ptr->get_value() != gl_control->get_value())
		gl_control->set_value(control_ptr->get_value());

	control_ptr->in_callback = false;
}

g2d_string_control::g2d_string_control(const std::string& label, std::string& value, cgv::g2d::irect rectangle) : cgv::gui::control<std::string>(label, &value) {
	gl_control = std::make_shared<input_control>(this->get_name(), rectangle);
	//gl_control->when(fltk::WHEN_CHANGED);
	//gl_control->callback(input_cb, static_cast<cgv::base::base*>(this));
	//gl_control->callback(input_cb, static_cast<cgv::base::base*>(this));
	gl_control->callback(input_cb, this);
	update();
}

void g2d_string_control::public_set_value(const std::string& value) {
	set_value(value);
}

std::string g2d_string_control::get_type_name() const {
	return "gl_string_control";
}

void g2d_string_control::update() {
	if(!in_callback)
		gl_control->set_value(this->get_value());
}

void* g2d_string_control::get_user_data() const {
	return nullptr;
}

std::shared_ptr<input_control> g2d_string_control::get_gl_control() {
	return gl_control;
}

}
}

#include <cgv/config/lib_end.h>
