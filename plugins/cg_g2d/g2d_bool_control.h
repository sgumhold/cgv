#pragma once

#include <cgv/gui/control.h>
#include <cgv/type/variant.h>

#include "widget.h"
#include "toggle.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

/** The control<bool> is implemented with different cg::g2d::button
	widgets. The g2d_bool_control is parameterized over
	the cg::g2d::button control widget type W. */
template <typename W>
struct CGV_API g2d_bool_control : public cgv::gui::control<bool> {
	/// pointer to the cg::g2d::widget that controls the value
	std::shared_ptr<W> control_widget;
	/// construct from label, value reference and dimensions
	g2d_bool_control(const std::string& label, bool& value, cgv::g2d::irect rectangle);
	/// destruct
	~g2d_bool_control() {}
	/// returns "g2d_bool_control"
	std::string get_type_name() const { return "g2d_bool_control"; }
	/// updates the g2d control widget in case the controlled value has been changed externally
	void update() {
		//T tmp = this->get_value();
		//control_widget->set_value(cgv::type::variant<double>::get(cgv::type::info::type_name<T>::get_name(), &tmp));
		control_widget->set_value(this->get_value());
	}
	/// give access to the protected value ptr to allow changing the value
	void public_set_value(bool value) { set_value(value); }
	/// adds to the implementation of fltk_base based on the control type
	//std::string get_property_declarations();
	/// abstract interface for the setter
	//bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	//bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return a fltk::Widget pointer
	// TODO: do we need this?
	//void* get_user_data() const { return &static_cast<std::shared_ptr<control_base>>(gl_control); }
	void* get_user_data() const { return nullptr; }
	/// interface for value updates independent of the value type T
	//void update_value_if_valid(double v) {
	//	if(this->check_and_set_value((T)v) && control_widget->get_value() != this->get_value())
	//		update();
	//}

	std::shared_ptr<W> get_widget() { return control_widget; }
};

template<typename W>
static cgv::gui::control_ptr create_bool_control(const std::string& label, void* value_ptr, const std::string& value_type, cgv::g2d::irect rectangle, std::shared_ptr<W>& control_widget) {
	auto ptr = new g2d_bool_control<W>(label, *static_cast<bool*>(value_ptr), rectangle);
	control_widget = ptr->get_widget();
	return cgv::gui::control_ptr(ptr);
}

template cgv::gui::control_ptr create_bool_control<toggle>(const std::string& label, void* value_ptr, const std::string& value_type, cgv::g2d::irect rectangle, std::shared_ptr<toggle>& control_widget);

}
}

#include <cgv/config/lib_end.h>
