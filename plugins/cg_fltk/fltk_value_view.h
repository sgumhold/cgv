#pragma once

#include <cgv/gui/view.h>
#include "fltk_base.h"

namespace fltk {
	class ValueOutput;
}

#include "lib_begin.h"

/** template class that implements all views of numeric values with the 
	fltk::ValueOutput class. */
template <typename T>
struct fltk_value_view : public cgv::gui::view<T>, public fltk_base
{
	/// a fltk::ValueOutput is used to implement the views of all number types
	::fltk::ValueOutput* fO;
	/// construct a value view with given dimensions
	fltk_value_view(const std::string& _label, const T& value, 
						 int x, int y, int w, int h);
	/// destruct fltk value view
	~fltk_value_view();
	/// returns "fltk_value_view"
	std::string get_type_name() const;
	/// get the current value and view it
	void update();
	/// only uses the implementation of fltk_base
	std::string get_property_declarations();
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return a fltk::Widget pointer
	void* get_user_data() const;
};

#include <cgv/config/lib_end.h>

