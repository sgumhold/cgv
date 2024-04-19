#pragma once

#include <cgv/gui/control.h>
#include <cgv/type/variant.h>

#include "widget.h"
#include "slider.h"
#include "valuator.h"
#include "value_input.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

/// this interface is used to update the value of g2d_value_control s.
struct CGV_API abst_g2d_value_callback {
	/// interface for value updates independent of the value and cg::g2d::valuator type 
	virtual void update_value_if_valid(double v) = 0;
};

template <typename T, class W>
static void configure(T& value, std::shared_ptr<W> w) {
	if(cgv::type::info::type_id<T>::get_id() < cgv::type::info::TI_FLT16) {
		w->set_step(1.0);
		w->set_range(cgv::dvec2(
			static_cast<double>(std::numeric_limits<T>::min()),
			static_cast<double>(std::numeric_limits<T>::max())
		));
	} else {
		w->set_step(0.1);
	}
}

template <typename T>
static void configure(T& value, std::shared_ptr<value_input> w) {
	if(cgv::type::info::type_id<T>::get_id() < cgv::type::info::TI_FLT16)
		w->input_widget.set_type(input::Type::kInteger);
	else
		w->set_step(0.1);
}

static void valuator_cb(widget* widget_ptr, void* valuator_ptr) {
	abst_g2d_value_callback* value_callback = dynamic_cast<abst_g2d_value_callback*>(static_cast<cgv::base::base*>(valuator_ptr));
	if(value_callback)
		value_callback->update_value_if_valid(static_cast<valuator*>(widget_ptr)->get_value());
}

/** The control<T> is implemented with different cg::g2d::valuator
	widgets. The g2d_value_control is parameterized over
	the cg::g2d::valuator control widget type W and the type T
	of the controlled value. */
template <typename T, typename W>
struct CGV_API g2d_value_control : public cgv::gui::control<T>, public abst_g2d_value_callback {
	/// pointer to the cg::g2d::widget that controls the value
	std::shared_ptr<W> control_widget;
	/// construct from label, value reference and dimensions
	g2d_value_control(const std::string& label, T& value, cgv::g2d::irect rectangle) : cgv::gui::control<T>(label, &value) {
		control_widget = std::make_shared<W>(this->get_name(), rectangle);
		configure(value, control_widget);
		control_widget->set_callback(valuator_cb, this);
		update();
	}
	/// destruct
	~g2d_value_control() {}
	/// returns "g2d_value_control"
	std::string get_type_name() const { return "g2d_value_control"; }
	/// updates the g2d control widget in case the controlled value has been changed externally
	void update() {
		T tmp = this->get_value();
		control_widget->set_value(cgv::type::variant<double>::get(cgv::type::info::type_name<T>::get_name(), &tmp));
	}
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
	void update_value_if_valid(double v) {
		if(this->check_and_set_value((T)v) && control_widget->get_value() != this->get_value())
			update();
	}

	std::shared_ptr<W> get_widget() { return control_widget; }
};

template<typename T, typename W>
static cgv::gui::control_ptr create_valuator_void(const std::string& label, void* value_ptr, const std::string& value_type, cgv::g2d::irect rectangle, std::shared_ptr<W>& control_widget) {
	auto ptr = new g2d_value_control<T, W>(label, *static_cast<T*>(value_ptr), rectangle);
	control_widget = ptr->get_widget();
	return cgv::gui::control_ptr(ptr);
}

template<typename W>
static cgv::gui::control_ptr create_value_control(const std::string& label, void* value_ptr, const std::string& value_type, cgv::g2d::irect rectangle, std::shared_ptr<W>& control_widget) {
	if(value_type == "int8")
		return create_valuator_void<cgv::type::int8_type, W>(label, value_ptr, value_type, rectangle, control_widget);
	if(value_type == "int16")
		return create_valuator_void<cgv::type::int16_type, W>(label, value_ptr, value_type, rectangle, control_widget);
	if(value_type == "int32")
		return create_valuator_void<cgv::type::int32_type, W>(label, value_ptr, value_type, rectangle, control_widget);
	if(value_type == "int64")
		return create_valuator_void<cgv::type::int64_type, W>(label, value_ptr, value_type, rectangle, control_widget);
	if(value_type == "uint8")
		return create_valuator_void<cgv::type::uint8_type, W>(label, value_ptr, value_type, rectangle, control_widget);
	if(value_type == "uint16")
		return create_valuator_void<cgv::type::uint16_type, W>(label, value_ptr, value_type, rectangle, control_widget);
	if(value_type == "uint32" || (sizeof(size_t) == 4 && value_type == "m"))
		return create_valuator_void<cgv::type::uint32_type, W>(label, value_ptr, value_type, rectangle, control_widget);
	if(value_type == "uint64" || (sizeof(size_t) == 8 && value_type == "m"))
		return create_valuator_void<cgv::type::uint64_type, W>(label, value_ptr, value_type, rectangle, control_widget);
	if(value_type == "flt32")
		return create_valuator_void<cgv::type::flt32_type, W>(label, value_ptr, value_type, rectangle, control_widget);
	if(value_type == "flt64")
		return create_valuator_void<cgv::type::flt64_type, W>(label, value_ptr, value_type, rectangle, control_widget);
	if(value_type == cgv::type::info::type_name<size_t>::get_name())
		return create_valuator_void<size_t, W>(label, value_ptr, value_type, rectangle, control_widget);
	return cgv::gui::control_ptr();
}

template cgv::gui::control_ptr create_value_control<slider>(const std::string& label, void* value_ptr, const std::string& value_type, cgv::g2d::irect rectangle, std::shared_ptr<slider>& control_widget);
template cgv::gui::control_ptr create_value_control<value_input>(const std::string& label, void* value_ptr, const std::string& value_type, cgv::g2d::irect rectangle, std::shared_ptr<value_input>& control_widget);

}
}

#include <cgv/config/lib_end.h>
