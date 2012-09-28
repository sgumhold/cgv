#include "fltk_value_view.h"
#include "fltk_driver_registry.h"
#include <cgv/type/variant.h>
#include <cgv/type/standard_types.h>

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/ValueOutput.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif
#include <iostream>

template <typename T>
fltk_value_view<T>::fltk_value_view(const std::string& _label, 
	const T& value, int x, int y, int w, int h)
	: cgv::gui::view<T>(_label, value)
{
	fO = new CW<fltk::ValueOutput>(x,y,w,h,this->get_name().c_str());
	fO->user_data(static_cast<cgv::base::base*>(this));
	update();
}

/// destruct fltk value view
template <typename T>
fltk_value_view<T>::~fltk_value_view()
{
	delete fO;
}

/// returns "fltk_value_view"
template <typename T>
std::string fltk_value_view<T>::get_type_name() const
{
	return "fltk_value_view";
}
/// get the current value and view it
template <typename T>
void fltk_value_view<T>::update()
{
	fO->value(cgv::type::variant<double>::get(
		cgv::type::info::type_name<T>::get_name(), this->value_ptr));
}

/// only uses the implementation of fltk_base
template <typename T>
std::string fltk_value_view<T>::get_property_declarations()
{
	return fltk_base::get_property_declarations();
}
/// abstract interface for the setter
template <typename T>
bool fltk_value_view<T>::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	return fltk_base::set_void(fO, this, property, value_type, value_ptr);
}
/// abstract interface for the getter
template <typename T>
bool fltk_value_view<T>::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return fltk_base::get_void(fO, this, property, value_type, value_ptr);
}
/// return a fltk::Widget pointer
template <typename T>
void* fltk_value_view<T>::get_user_data() const
{
	return static_cast<fltk::Widget*>(fO);
}


struct value_view_factory : public abst_view_factory
{
	view_ptr create(const std::string& label, 
			const void* value_ptr, const std::string& value_type, 
			const std::string& gui_type, int x, int y, int w, int h)
	{
		if (value_type == "int8")
			return view_ptr(new fltk_value_view<cgv::type::int8_type>(label, 
				*static_cast<const cgv::type::int8_type*>(value_ptr), x, y, w, h));
		if (value_type == "int16")
			return view_ptr(new fltk_value_view<cgv::type::int16_type>(label, 
				*static_cast<const cgv::type::int16_type*>(value_ptr), x, y, w, h));
		if (value_type == "int32")
			return view_ptr(new fltk_value_view<cgv::type::int32_type>(label, 
				*static_cast<const cgv::type::int32_type*>(value_ptr), x, y, w, h));
		if (value_type == "int64")
			return view_ptr(new fltk_value_view<cgv::type::int64_type>(label, 
				*static_cast<const cgv::type::int64_type*>(value_ptr), x, y, w, h));
		if (value_type == "uint8")
			return view_ptr(new fltk_value_view<cgv::type::uint8_type>(label, 
				*static_cast<const cgv::type::uint8_type*>(value_ptr), x, y, w, h));
		if (value_type == "uint16")
			return view_ptr(new fltk_value_view<cgv::type::uint16_type>(label, 
				*static_cast<const cgv::type::uint16_type*>(value_ptr), x, y, w, h));
		if (value_type == "uint32")
			return view_ptr(new fltk_value_view<cgv::type::uint32_type>(label, 
				*static_cast<const cgv::type::uint32_type*>(value_ptr), x, y, w, h));
		if (value_type == "uint64")
			return view_ptr(new fltk_value_view<cgv::type::uint64_type>(label, 
				*static_cast<const cgv::type::uint64_type*>(value_ptr), x, y, w, h));
		if (value_type == "flt32")
			return view_ptr(new fltk_value_view<cgv::type::flt32_type>(label, 
				*static_cast<const cgv::type::flt32_type*>(value_ptr), x, y, w, h));
		if (value_type == "flt64")
			return view_ptr(new fltk_value_view<cgv::type::flt64_type>(label, 
				*static_cast<const cgv::type::flt64_type*>(value_ptr), x, y, w, h));
		return view_ptr();
	}
};

view_factory_registration<value_view_factory> value_view_fac_reg;

