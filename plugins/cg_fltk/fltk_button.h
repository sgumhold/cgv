#pragma once

#include <cgv/gui/button.h>
#include "fltk_base.h"

namespace fltk {
	class Button;
}

#include "lib_begin.h"

/// implements the button gui element with a fltk Button
struct CGV_API fltk_button : public cgv::gui::button, public fltk_base
{
	/// store pointer to fltk Button
	fltk::Button* fB;
	/// construct fltk button from label and dimensions
	fltk_button(const std::string& _label, int x, int y, int w, int h);
	/// destruct fltk button
	~fltk_button();
	/// return "fltk button"
	std::string get_type_name() const;
	/// triggers a redraw
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