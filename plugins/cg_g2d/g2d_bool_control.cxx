#include "g2d_bool_control.h"

namespace cg {
namespace g2d {

void bool_button_cb(widget* w, void* c) {
	g2d_bool_control<toggle>* control_ptr = static_cast<g2d_bool_control<toggle>*>(static_cast<cgv::base::base*>(c));
	toggle* widget_ptr = static_cast<toggle*>(w);

	control_ptr->set_new_value(widget_ptr->get_value());

	if(control_ptr->check_value(*control_ptr)) {
		bool temp_value = control_ptr->get_value();
		control_ptr->public_set_value(control_ptr->get_new_value());
		control_ptr->set_new_value(temp_value);
		control_ptr->value_change(*control_ptr);
	}

	if(control_ptr->get_value() != widget_ptr->get_value())
		widget_ptr->set_value(control_ptr->get_value());
}

template<typename W>
g2d_bool_control<W>::g2d_bool_control(const std::string& label, bool& value, cgv::g2d::irect rectangle) : cgv::gui::control<bool>(label, &value) {
	control_widget = std::make_shared<W>(this->get_name(), rectangle);
	control_widget->set_callback(bool_button_cb, this);
	update();
}

}
}
