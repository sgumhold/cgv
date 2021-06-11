#include "fltk_value_control.h"
#include "fltk_event.h"
#include "fltk_driver_registry.h"
#include <cgv/type/variant.h>
#include <cgv/type/info/type_id.h>
#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Slider.h>
#include <fltk/ValueSlider.h>
#include <fltk/Adjuster.h>
#include <fltk/events.h>
#include <fltk/ThumbWheel.h>
#include <fltk/Dial.h>
#include <fltk/ValueInput.h>
#include <fltk/Group.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif
#include <iostream>


using namespace cgv::type;

/** implement property management of standard valuator properties
    in a template independent fashion only once */
struct valuator_property_interface_base
{
	/// returns declarations for the reflected properties of a fltk Valuator
	static std::string get_property_declarations(fltk_base& fb)
	{
		return fb.get_property_declarations()+";min:flt64;max:flt64;step:flt64";
	}
	/// set a property of a fltk Valuator
	static bool set_void(fltk_base& fb, cgv::base::named* nam, fltk::Valuator* fv, 
		const std::string& property, 
			const std::string& value_type, const void* value_ptr)
	{
		if (fb.set_void(fv, nam, property, value_type, value_ptr))
			return true;
		if (property == "min")
			fv->minimum(variant<double>::get(value_type,value_ptr));
		else if (property == "max")
			fv->maximum(variant<double>::get(value_type,value_ptr));
		else if (property == "step")
			fv->step(variant<double>::get(value_type,value_ptr));
		else 
			return false;
		fv->redraw();
		if (fv->parent())
			fv->parent()->redraw();
		return true;
	}
	/// get a property of a fltk Valuator
	static bool get_void(fltk_base& fb, cgv::base::named* nam, fltk::Valuator* fv, 
			const std::string& property, 
			const std::string& value_type, void* value_ptr)
	{
		if (fb.get_void(fv, nam, property, value_type, value_ptr))
			return true;
		if (property == "min")
			set_variant(fv->minimum(),value_type,value_ptr);
		else if (property == "max")
			set_variant(fv->maximum(), value_type,value_ptr);
		else if (property == "step")
			set_variant(fv->step(), value_type,value_ptr);
		else 
			return false;
		return true;
	}
};

template <class FC>
struct valuator_property_interface : public valuator_property_interface_base
{
};

template <>
struct valuator_property_interface<fltk::Slider>
{
	/// return a semicolon separated list of property declarations of the form "name:type", by default an empty list is returned
	static std::string get_property_declarations(fltk_base& fb)
	{
		return valuator_property_interface_base::get_property_declarations(fb)+";ticks:bool;log:bool";
	}
	static bool set_void(fltk_base& fb, cgv::base::named* nam, fltk::Slider* fs, 
		const std::string& property, 
		const std::string& value_type, const void* value_ptr)
	{
		if (valuator_property_interface_base::set_void(fb,nam,fs,property,value_type,value_ptr))
			return true;
		if (property == "ticks") {
			if (variant<bool>::get(value_type,value_ptr))
				fs->type(fs->type()|fltk::Slider::TICK_ABOVE);
			else
				fs->type(fs->type()&~fltk::Slider::TICK_ABOVE);
		}
		else if (property == "log") {
			if (variant<bool>::get(value_type,value_ptr))
				fs->type(fs->type()|fltk::Slider::LOG);
			else
				fs->type(fs->type()&~fltk::Slider::LOG);
		}
		else
			return false;
		fs->redraw();
		if (fs->parent())
			fs->parent()->redraw();
		return true;
	}
	static bool get_void(fltk_base& fb, cgv::base::named* nam, fltk::Slider* fs, 
		const std::string& property, 
		const std::string& value_type, void* value_ptr)
	{
		if (valuator_property_interface_base::get_void(fb,nam,fs,property,value_type,value_ptr))
			return true;
		if (property == "ticks")
			set_variant(((fs->type()&fltk::Slider::TICK_ABOVE) != 0), value_type,value_ptr);
		else if (property == "log")
			set_variant(((fs->type()&fltk::Slider::LOG) != 0), value_type,value_ptr);
		else
			return false;
		return true;
	}
};

template <>
struct valuator_property_interface<fltk::ValueSlider> 
	: public valuator_property_interface<fltk::Slider>
{
};

template <typename T, class FC>
void configure(T&, FC* vi)
{
	if (cgv::type::info::type_id<T>::get_id() < cgv::type::info::TI_FLT16) {
		vi->step(1.0);
		vi->type(fltk::FloatInput::INT);
	}
}

template <typename T>
void configure(T& value, fltk::FloatInput* vi)
{
	if (cgv::type::info::type_id<T>::get_id() < cgv::type::info::TI_FLT16)
		vi->type(fltk::FloatInput::INT);
}

template <typename T>
void configure(T&, fltk::ValueSlider* vs)
{
	if (cgv::type::info::type_id<T>::get_id() < cgv::type::info::TI_FLT16) {
		vs->step(1.0);
		vs->type(fltk::FloatInput::INT);
	}
}

void valuator_cb(fltk::Widget* w, void* valuator_ptr)
{
	ref_current_modifiers() = cgv_modifiers(fltk::event_state());
	abst_fltk_value_callback* fvc = dynamic_cast<abst_fltk_value_callback*>(
		static_cast<cgv::base::base*>(valuator_ptr));
	if (fvc)
		fvc->update_value_if_valid(static_cast<fltk::Valuator*>(w)->value());
	else
		std::cerr << "could not locate valuator" << std::endl;
}

/// construct from label, value reference and dimensions
template <typename T, typename FC>
fltk_value_control<T,FC>::fltk_value_control(const std::string& _label, T& value, abst_control_provider* acp, int x, int y, int w, int h)
	: cgv::gui::control<T>(_label, acp, &value)
{
	fC = new CW<FC>(x,y,w,h, this->get_name().c_str());
	fC->flags((fC->flags()&~fltk::ALIGN_MASK)|fltk::ALIGN_LEFT);
	configure(value, fC);
	fC->callback(valuator_cb,static_cast<cgv::base::base*>(this));
	update();
}

/// destruct fltk value control
template <typename T, typename FC>
fltk_value_control<T,FC>::~fltk_value_control()
{
	delete fC;
}

/// returns "fltk_value_control"
template <typename T, typename FC>
std::string fltk_value_control<T,FC>::get_type_name() const
{
	return "fltk_value_control";
}

/// updates the fltk control widget in case the controled value has been changed externally
template <typename T, typename FC>
void fltk_value_control<T,FC>::update()
{
	T tmp = this->get_value();
	fC->value(cgv::type::variant<double>::get(
		cgv::type::info::type_name<T>::get_name(), &tmp));
}

/// adds to the implementation of fltk_base based on the control type
template <typename T, typename FC>
std::string fltk_value_control<T,FC>::get_property_declarations()
{
	return valuator_property_interface<FC>::get_property_declarations(*this);
}

/// abstract interface for the setter
template <typename T, typename FC>
bool fltk_value_control<T,FC>::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	bool res = valuator_property_interface<FC>::set_void(*this, this, fC, property, value_type, value_ptr);
	if (property == "step") {
		T tmp = this->get_value();
		double v = cgv::type::variant<double>::get(cgv::type::info::type_name<T>::get_name(), &tmp);
		fC->value(v+1.0);
		fC->value(v);
	}
	return res;
}

/// abstract interface for the getter
template <typename T, typename FC>
bool fltk_value_control<T,FC>::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return valuator_property_interface<FC>::get_void(
		*this, this, fC, property, value_type, value_ptr);
}

/// return a fltk::Widget pointer
template <typename T, typename FC>
void* fltk_value_control<T,FC>::get_user_data() const
{
	return static_cast<fltk::Widget*>(fC);
}

/// interface for value updates independent of the value type T
template <typename T, typename FC>
void fltk_value_control<T,FC>::update_value_if_valid(double v)
{
	if (this->check_and_set_value((T)v) && fC->value() != this->get_value())
		update();
}


template <class B>
control_ptr create_valuator_1(const std::string& label, void* value_ptr, 
	abst_control_provider* acp, const std::string& value_type, int x, int y, int w, int h)
{
	if (value_type == "int8")
		return control_ptr(new fltk_value_control<cgv::type::int8_type,B>(
			label, *static_cast<cgv::type::int8_type*>(value_ptr), acp, x, y, w, h));
	if (value_type == "int16")
		return control_ptr(new fltk_value_control<cgv::type::int16_type,B>(
			label, *static_cast<cgv::type::int16_type*>(value_ptr), acp, x, y, w, h));
	if (value_type == "int32")
		return control_ptr(new fltk_value_control<cgv::type::int32_type,B>(
			label, *static_cast<cgv::type::int32_type*>(value_ptr), acp, x, y, w, h));
	if (value_type == "int64")
		return control_ptr(new fltk_value_control<cgv::type::int64_type,B>(
			label, *static_cast<cgv::type::int64_type*>(value_ptr), acp, x, y, w, h));
	if (value_type == "uint8")
		return control_ptr(new fltk_value_control<cgv::type::uint8_type,B>(
			label, *static_cast<cgv::type::uint8_type*>(value_ptr), acp, x, y, w, h));
	if (value_type == "uint16")
		return control_ptr(new fltk_value_control<cgv::type::uint16_type,B>(
			label, *static_cast<cgv::type::uint16_type*>(value_ptr), acp, x, y, w, h));
	if (value_type == "uint32")
		return control_ptr(new fltk_value_control<cgv::type::uint32_type,B>(
			label, *static_cast<cgv::type::uint32_type*>(value_ptr), acp, x, y, w, h));
	if (value_type == "uint64")
		return control_ptr(new fltk_value_control<cgv::type::uint64_type,B>(
			label, *static_cast<cgv::type::uint64_type*>(value_ptr), acp, x, y, w, h));
	if (value_type == "flt32")
		return control_ptr(new fltk_value_control<cgv::type::flt32_type,B>(
			label, *static_cast<cgv::type::flt32_type*>(value_ptr), acp, x, y, w, h));
	if (value_type == "flt64")
		return control_ptr(new fltk_value_control<cgv::type::flt64_type,B>(
			label, *static_cast<cgv::type::flt64_type*>(value_ptr), acp, x, y, w, h));
	if (value_type == cgv::type::info::type_name<size_t>::get_name())
		return control_ptr(new fltk_value_control<size_t,B>(
			label, *static_cast<size_t*>(value_ptr), acp, x, y, w, h));
	return control_ptr();
}

struct fltk_value_control_factory : public abst_control_factory
{
	control_ptr create(const std::string& label, 
		void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
		const std::string& gui_type, int x, int y, int w, int h)
	{
		if (gui_type == "slider")
			return create_valuator_1<fltk::Slider>(label, value_ptr, acp, value_type, x, y, w, h);
		else if (gui_type == "value_slider")
			return create_valuator_1<fltk::ValueSlider>(label, value_ptr, acp, value_type, x, y, w, h);
		else if (gui_type == "wheel")
			return create_valuator_1<fltk::ThumbWheel>(label, value_ptr, acp, value_type, x, y, w, h);
		else if (gui_type == "dial")
			return create_valuator_1<fltk::Dial>(label, value_ptr, acp, value_type, x, y, w, h);
		else if (gui_type == "adjuster")
			return create_valuator_1<fltk::Adjuster>(label, value_ptr, acp, value_type, x, y, w, h);
		else 
			return create_valuator_1<fltk::ValueInput>(label, value_ptr, acp, value_type, x, y, w, h);
	}
};

control_factory_registration<fltk_value_control_factory> fltk_value_fac_reg;

