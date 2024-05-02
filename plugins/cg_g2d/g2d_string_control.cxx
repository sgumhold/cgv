#include "g2d_string_control.h"

namespace cg {
namespace g2d {

void input_cb(widget* w, void* c) {
	g2d_string_control* control_ptr = static_cast<g2d_string_control*>(static_cast<cgv::base::base*>(c));
	input* widget_ptr = static_cast<input*>(w);

	control_ptr->in_callback = true;

	control_ptr->set_new_value(widget_ptr->get_value());

	if(control_ptr->check_value(*control_ptr)) {
		std::string temp_value = control_ptr->get_value();
		control_ptr->public_set_value(control_ptr->get_new_value());
		control_ptr->set_new_value(temp_value);
		control_ptr->value_change(*control_ptr);
	}

	if(control_ptr->get_value() != widget_ptr->get_value())
		widget_ptr->set_value(control_ptr->get_value());

	control_ptr->in_callback = false;
}

g2d_string_control::g2d_string_control(const std::string& label, std::string& value, cgv::g2d::irect rectangle) : cgv::gui::control<std::string>(label, &value) {
	control_widget = std::make_shared<input>(this->get_name(), rectangle);
	control_widget->set_callback(input_cb, this);
	update();
}

void g2d_string_control::update() {
	if(!in_callback)
		control_widget->set_value(this->get_value());
}

}
}
