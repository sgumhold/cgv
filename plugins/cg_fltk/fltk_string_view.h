#pragma once

#include <cgv/gui/view.h>
#include "fltk_base.h"

namespace fltk {
	class Output;
}

#include "lib_begin.h"

/** implements a view for string values with the fltk::Output class */
struct CGV_API fltk_string_view : public cgv::gui::view<std::string>, public fltk_base
{
	/// a fltk::Output is used to implement the string view
	fltk::Output* fO;
	/// construct from label, value reference and dimensions
	fltk_string_view(const std::string& _label, const std::string& value, 
						  int x, int y, int w, int h);
	/// returns "fltk_string_view"
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
