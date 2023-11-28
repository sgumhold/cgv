#pragma once

#include <cgv/gui/control.h>
#include <cgv/type/variant.h>

#include "control_base.h"
#include "value_control.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

/// this interface is used to update the value of fltk_value_control s.
struct CGV_API abst_gl_value_callback {
	/// interface for value updates independent of the value and fltk Valuator type 
	virtual void update_value_if_valid(double v) = 0;
};

static void valuator_cb(control_base* control, void* valuator_ptr) {
	abst_gl_value_callback* value_callback = dynamic_cast<abst_gl_value_callback*>(static_cast<cgv::base::base*>(valuator_ptr));
	if(value_callback)
		value_callback->update_value_if_valid(static_cast<value_control*>(control)->get_value());
}

/** The control<T> is implemented with different fltk Valuator
	widgets. The fltk_value_control is parameterized over
	 the fltk Valuator control widget type FC and the type T
	 of the conrolled value. */
template <typename T, typename GC>
struct CGV_API g2d_value_control : public cgv::gui::control<T>, public abst_gl_value_callback {
	/// pointer to the fltk Widget that controls the value
	// TODO: find good name
	std::shared_ptr<GC> gl_control;
	/// construct from label, value reference and dimensions
	g2d_value_control(const std::string& label, T& value, cgv::g2d::irect rectangle) : cgv::gui::control<T>(label, &value) {
		gl_control = std::make_shared<GC>(this->get_name(), rectangle);

		// TODO: implement to set value and range
		//configure(value, fC);

		//gl_control->callback(valuator_cb, static_cast<cgv::base::base*>(this));
		gl_control->callback(valuator_cb, this);
		update();
	}
	/// destruct fltk value control
	~g2d_value_control() {}
	/// returns "fltk_value_control"
	std::string get_type_name() const {
		return "g2d_value_control";
	}
	/// updates the fltk control widget in case the controlled value has been changed externally
	void update() {
		T tmp = this->get_value();
		gl_control->set_value(cgv::type::variant<double>::get(cgv::type::info::type_name<T>::get_name(), &tmp));
	}
	/// adds to the implementation of fltk_base based on the control type
	//std::string get_property_declarations();
	/// abstract interface for the setter
	//bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	//bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return a fltk::Widget pointer
	// TODO: do we need this?
	void* get_user_data() const {
		return static_cast<control_base*>(gl_control.get());
	}
	/// interface for value updates independent of the value type T
	void update_value_if_valid(double v) {
		if(this->check_and_set_value((T)v) && gl_control->get_value() != this->get_value())
			update();
	}


	std::shared_ptr<GC> get_gl_control() {
		return gl_control;
	}
};

}
}

#include <cgv/config/lib_end.h>
