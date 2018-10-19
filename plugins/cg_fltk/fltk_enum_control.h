#pragma once

#include <cgv/gui/control.h>
#include <cgv/utils/token.h>
#include <fltk/RadioButton.h>
#include <fltk/ToggleButton.h>
#include "fltk_base.h"

namespace fltk {
	class Choice;
	class Widget;
	class Group;
	class RadioButton;
	class ToggleButton;
}

#include "lib_begin.h"

/// Basic class for enum controls.
/// Inherited classes must implement the set_index method to visually set an index and
/// the get_index method to get the currently selected index
class CGV_API fltk_enum_control: public cgv::gui::control<int>, public fltk_base {
public:
	fltk_enum_control(const std::string &label, cgv::gui::abst_control_provider* acp, int &value, const std::string& enum_declarations);
	/// destruct enum control
	virtual ~fltk_enum_control();

	/// give access to the protected value ptr to allow changing the value
	void public_set_value(int v);
	/// returns "fltk_enum_control"
	virtual std::string get_type_name() const;
	/// updates the fltk::Choice widget in case the controled value has been changed externally
	void update();
	/// only uses the implementation of fltk_base
	virtual std::string get_property_declarations();
	
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return a fltk::Widget pointer
	void* get_user_data() const;

	
protected:
	fltk::Widget *container;
	/// vector of enumeration elements
	std::vector<std::string> enum_strings;
	/// vector of corresponding enumeration values
	std::vector<int> enum_values;
	
	int index_to_value(int idx) const;
	int value_to_index(int val) const;
	// update the visualization to reflect a certain index
	virtual void set_index(int idx) = 0;
	// get the selected index
	virtual int get_index(fltk::Widget *w) = 0;
	/// this is called if the user sets a new enum definition
	virtual void update_enums();
	// Parse the enum declarations and fille the list enum_strings
	void parse_enum_declarations(const std::string& enum_declarations);
	
	// The callback needed by fltk2
	static void choice_cb(fltk::Widget* w, void* input_ptr);
};



/// Enum control that represents the enums as a drop down list
class CGV_API fltk_enum_dropdown_control: public fltk_enum_control {
public:
	/// Construct the drop down list
	fltk_enum_dropdown_control(const std::string& label, int& value, cgv::gui::abst_control_provider* acp, const std::string& enum_declarations,
					  int x, int y, int w, int h);
	
protected:
	void set_index(int idx);
	int get_index(fltk::Widget* w);
	void update_enums();
};


/// Enum control that represents the enums as a radio button group
class CGV_API fltk_enum_radio_control: public fltk_enum_control {
public:
	/// Construct the radio button group
	fltk_enum_radio_control(const std::string& label, int& value, cgv::gui::abst_control_provider* acp, const std::string& enum_declarations,
					  int x, int y, int w, int h);
	
protected:
	void set_index(int idx);
	int get_index(fltk::Widget* w);
};



/// Enum control that represents the enums as a toggle button group
class CGV_API fltk_enum_toggle_control: public fltk_enum_control {
public:
	/// Construct the toggle button group
	fltk_enum_toggle_control(const std::string& label, int& value, cgv::gui::abst_control_provider* acp, const std::string& enum_declarations,
					  int x, int y, int w, int h);
	
protected:
	void set_index(int idx);
	int get_index(fltk::Widget* w);
};



#include <cgv/config/lib_end.h>
