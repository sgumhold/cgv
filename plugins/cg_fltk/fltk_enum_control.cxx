#include "fltk_enum_control.h"
#include "fltk_driver_registry.h"
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/scan_enum.h>
#include <algorithm>

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Choice.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif

#include <iostream>

using namespace cgv::utils;

void choice_cb(fltk::Widget* w, void* input_ptr)
{
	fltk_enum_control*  fsc = static_cast<fltk_enum_control*>(static_cast<cgv::base::base*>(input_ptr));
	fltk::Choice* fC = static_cast<fltk::Choice*>(w);
	int val = fsc->index_to_value(fC->value());
	fsc->set_new_value(val);
	if (fsc->check_value(*fsc)) {
		int tmp_value = fsc->get_value();
		fsc->public_set_value(fsc->get_new_value());
		fsc->set_new_value(tmp_value);
		fsc->value_change(*fsc);
	}
	int idx = fsc->value_to_index(fsc->get_value());
	if (idx != fC->value())
		fC->value(idx);
}

///
void fltk_enum_control::parse_enum_declarations(const std::string& enum_declarations)
{
	std::vector<token> enum_names;
	cgv::utils::parse_enum_declarations(enum_declarations, enum_names, enum_values);
	enum_strings.resize(enum_names.size());
	for (unsigned i=0; i<enum_names.size(); ++i)
		enum_strings[i] = to_string(enum_names[i]);
}

/// construct from label, value reference and dimensions
fltk_enum_control::fltk_enum_control(const std::string& _label, int& value, abst_control_provider* acp, const std::string& enum_declarations,
								     int x, int y, int w, int h)
	: control<int>(_label, acp, &value)
{
	parse_enum_declarations(enum_declarations);
	fC = new CW<fltk::Choice>(x,y,w,h,get_name().c_str());
	for (unsigned int i=0; i<enum_strings.size(); ++i) {
		fC->add(enum_strings[i].c_str(), (void*)i);
	}
	fC->callback(choice_cb,static_cast<cgv::base::base*>(this));
	update();
}

/// destruct enum control
fltk_enum_control::~fltk_enum_control()
{
	delete fC;
}


int fltk_enum_control::index_to_value(int idx) const
{
	return enum_values[idx];
}

int fltk_enum_control::value_to_index(int val) const
{
	return (int)cgv::utils::find_enum_index(val, enum_values);
}

/// give access to the protected value ptr to allow changing the value
void fltk_enum_control::public_set_value(int i)
{
	set_value(i);
}
/// returns "fltk_enum_control"
std::string fltk_enum_control::get_type_name() const
{
	return "fltk_enum_control";
}

/// updates the fltk::Input widget in case the controled value has been changed externally
void fltk_enum_control::update()
{
	fC->value(value_to_index(get_value()));
}
/// only uses the implementation of fltk_base
std::string fltk_enum_control::get_property_declarations()
{
	return fltk_base::get_property_declarations();
}
/// abstract interface for the setter
bool fltk_enum_control::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	return fltk_base::set_void(fC, this, property, value_type, value_ptr);
}
/// abstract interface for the getter
bool fltk_enum_control::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return fltk_base::get_void(fC, this, property, value_type, value_ptr);
}
/// return a fltk::Widget pointer
void* fltk_enum_control::get_user_data() const
{
	return static_cast<fltk::Widget*>(fC);
}

struct enum_control_factory : public abst_control_factory
{
	control_ptr create(const std::string& label, 
		void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
		const std::string& gui_type, int x, int y, int w, int h)
	{
		if (value_type == "enum")
			return control_ptr(new fltk_enum_control(label, 
				*static_cast<int*>(value_ptr), acp, gui_type, x, y, w, h));
		return control_ptr();
	}
};

control_factory_registration<enum_control_factory> enum_control_fac_reg;