#include "fltk_enum_control.h"
#include "fltk_driver_registry.h"
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/scan_enum.h>
#include <cgv/type/variant.h>
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



void fltk_enum_control::choice_cb(fltk::Widget* w, void* input_ptr)
{	
	fltk_enum_control*  fsc = static_cast<fltk_enum_control*>(static_cast<cgv::base::base*>(input_ptr));
		
	int idx = fsc->get_index(w);
	int val = fsc->index_to_value(idx);

	fsc->set_new_value(val);
	if (fsc->check_value(*fsc)) {
		int tmp_value = fsc->get_value();
		fsc->public_set_value(fsc->get_new_value());
		fsc->set_new_value(tmp_value);
		fsc->value_change(*fsc);
	}

	int idx_new = fsc->value_to_index(fsc->get_value());
	fsc->set_index(idx_new);	
}





fltk_enum_control::fltk_enum_control(const std::string &label, cgv::gui::abst_control_provider* acp, int &value, const std::string& enum_declarations): control<int>(label, acp, &value)
{
	parse_enum_declarations(enum_declarations);
	container = 0;
}


fltk_enum_control::~fltk_enum_control()
{
	if (container)
		delete container;
}




std::string fltk_enum_control::get_property_declarations()
{
	return fltk_base::get_property_declarations();
}



std::string fltk_enum_control::get_type_name() const
{
	return "fltk_enum_control";
}



void fltk_enum_control::update()
{
	set_index(value_to_index(get_value()));
}


int fltk_enum_control::index_to_value(int idx) const
{
	return enum_values[idx];
}


int fltk_enum_control::value_to_index(int val) const
{
	return (int)cgv::utils::find_enum_index(val, enum_values);
}


void fltk_enum_control::parse_enum_declarations(const std::string& enum_declarations)
{
	std::vector<token> enum_names;
	cgv::utils::parse_enum_declarations(enum_declarations, enum_names, enum_values);
	enum_strings.resize(enum_names.size());
	for (unsigned i=0; i<enum_names.size(); ++i)
		enum_strings[i] = to_string(enum_names[i]);
}



void fltk_enum_control::public_set_value(int v)
{
	set_value(v);
}



bool fltk_enum_control::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	return fltk_base::set_void(container, this, property, value_type, value_ptr);
}


bool fltk_enum_control::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return fltk_base::get_void(container, this, property, value_type, value_ptr);
}


void* fltk_enum_control::get_user_data() const
{
	return container;
}







fltk_enum_dropdown_control::fltk_enum_dropdown_control(const std::string& label, int& value, abst_control_provider* acp, const std::string& enum_declarations, int x, int y, int w, int h):
fltk_enum_control(label, acp, value, enum_declarations)
{
	container = new CW<fltk::Choice>(x,y,w,h, get_name().c_str());
	for (unsigned int i=0; i<enum_strings.size(); ++i) {
		static_cast<fltk::Choice*>(container)->add(enum_strings[i].c_str(), (void*)i);
	}
	container->callback(choice_cb,static_cast<cgv::base::base*>(this));
	
	update();
}



void fltk_enum_dropdown_control::set_index(int idx)
{
	static_cast<fltk::Choice*>(container)->value(idx);
}


int fltk_enum_dropdown_control::get_index(fltk::Widget* w)
{
	return static_cast<fltk::Choice*>(container)->value();
}




fltk_enum_radio_control::fltk_enum_radio_control(const std::string& label, int& value, abst_control_provider* acp, const std::string& enum_declarations, int x, int y, int w, int h): 
fltk_enum_control(label, acp, value, enum_declarations)
{
	const int padding = 20;
	container = new fltk::Group(x, y, w, enum_strings.size()*h+padding, get_name().c_str());

	// container->box(fltk::BORDER_BOX);
	container->flags(fltk::ALIGN_INSIDE_TOPLEFT);
	container->labelfont(fltk::TIMES_BOLD);
	container->labelsize(13); 
	
	
//	static_cast<fltk::Group*>(container)->add(label);
	for (unsigned int i=0; i<enum_strings.size(); ++i) {
		fltk::RadioButton *but = new fltk::RadioButton(0,i*h+padding,w,h,enum_strings[i].c_str());
		but->callback(choice_cb,static_cast<cgv::base::base*>(this));
		static_cast<fltk::Group*>(container)->add(but);
	}
	
	update();

}



int fltk_enum_radio_control::get_index(fltk::Widget* w)
{
	fltk::Group *g = static_cast<fltk::Group*>(container);
	int idx = 0;
	for (int i=0; i<g->children(); i++) {
		fltk::RadioButton *but = static_cast<fltk::RadioButton*>(g->child(i));
		if (but->state()) {
			idx = i;
			break;
		}
	}
	return idx;
}



void fltk_enum_radio_control::set_index(int idx)
{
	fltk::Group *g = static_cast<fltk::Group*>(container);
	static_cast<fltk::RadioButton*>(g->child(idx))->state(1); 
}




fltk_enum_toggle_control::fltk_enum_toggle_control(const std::string& label, int& value, abst_control_provider* acp, const std::string& enum_declarations, int x, int y, int w, int h): 
fltk_enum_control(label, acp, value, enum_declarations)
{
	const int padding = 20;
	const int margin = 8;
	container = new fltk::Group(x, y, w, enum_strings.size()*(h+margin)+padding, get_name().c_str());

	// container->box(fltk::BORDER_BOX);
	container->flags(fltk::ALIGN_INSIDE_TOPLEFT);
	container->labelfont(fltk::TIMES_BOLD);
	container->labelsize(13); 
	
	
	for (unsigned int i=0; i<enum_strings.size(); ++i) {
		fltk::ToggleButton *but = new fltk::ToggleButton(0,i*(h+margin)+padding,w,h,enum_strings[i].c_str());
		but->callback(choice_cb,static_cast<cgv::base::base*>(this));
		static_cast<fltk::Group*>(container)->add(but);
	}
	
	update();
}



int fltk_enum_toggle_control::get_index(fltk::Widget* w)
{
	fltk::Group *g = static_cast<fltk::Group*>(container);
	int idx = 0;
	for (int i=0; i<g->children(); i++) {
		if (w == g->child(i)) {
			idx = i;
		}
	}
	return idx;
}



void fltk_enum_toggle_control::set_index(int idx)
{
	fltk::Group *g = static_cast<fltk::Group*>(container);
	for (int i=0; i<g->children(); ++i) {
		fltk::ToggleButton *but = static_cast<fltk::ToggleButton*>(g->child(i));
		but->value(idx == i);
	}
}




// Helper class to parse the "enums" option  
struct enums_getter : public cgv::base::base
{
	std::string enums;
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return rh.reflect_member("enums", enums);		
	}
};


struct enum_control_factory : public abst_control_factory
{	
	/// Create a control with options provided
	control_ptr create_with_options(const std::string& label, 
		void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, int x, int y, int w, int h)
	{
		// If the value type does not match, then return an empty control
		if (value_type != "enum")
			return control_ptr();
		
		enums_getter eg;
		eg.multi_set(options);
		// If the enum field was not specified or is empty then assume that the enums are
		// provided via the old style (mis)using the gui_type field
		if (eg.enums.empty()) {
			std::cerr<<"Warning: Specifying enumerations via the gui type is deprecated."<<std::endl
					  <<"Please use the new format: add_control(label, variable, \"dropdown\", \"enums='a,b,c'\")"<<std::endl
					  <<"to place the enumeration strings in the options field. Instead of \"dropdown\" you"<<std::endl
					  <<"can also try \"radio\" or \"toggle\"."<<std::endl;
			return control_ptr(new fltk_enum_dropdown_control(label, 
				*static_cast<int*>(value_ptr), acp, gui_type, x, y, w, h));
		}
		
		// Otherwise check which gui type to use to represent this enum
		if (gui_type == "radio") 
			return control_ptr(new fltk_enum_radio_control(label, 
				*static_cast<int*>(value_ptr), acp, eg.enums, x, y, w, h)); 
		else 
		if (gui_type == "dropdown") 
			return control_ptr(new fltk_enum_dropdown_control(label, 
				*static_cast<int*>(value_ptr), acp, eg.enums, x, y, w, h));
		else
		if (gui_type == "toggle") 
			return control_ptr(new fltk_enum_toggle_control(label, 
				*static_cast<int*>(value_ptr), acp, eg.enums, x, y, w, h)); 
		else {
			std::cerr<<"Unknown gui type "<<gui_type<<" for enumeration."<<std::endl;
			return control_ptr(new fltk_enum_dropdown_control(label, 
				*static_cast<int*>(value_ptr), acp, eg.enums, x, y, w, h));
		}
	}

	/// Create a control without options. Just calls the version with options and 
	/// leaves the appropriate field empty
	control_ptr create(const std::string& label, 
		void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
		const std::string& gui_type, int x, int y, int w, int h) {
		return create_with_options(label, value_ptr, acp, value_type, gui_type, "", x, y, w, h);
	}	
};

control_factory_registration<enum_control_factory> enum_control_fac_reg;