#include "fltk_base.h"
#include "fltk_event.h"
#include "fltk_driver_registry.h"
#include <cgv/type/variant.h>
#include <cgv/gui/shortcut.h>
#include <cgv/gui/control.h>
#include <cgv/utils/convert_string.h>
#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Input.h>
#include <fltk/events.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif
#include <iostream>

using namespace cgv::gui;
using namespace cgv::utils;

class fltk_shortcut_control : public control<cgv::gui::shortcut>, public CW<fltk::Input>, public fltk_base
{
protected:
	std::string shortcut_str;
	cgv::gui::shortcut last_sc;
public:
	fltk_shortcut_control(const std::string& _label, cgv::gui::shortcut& value, 
		             cgv::gui::abst_control_provider* acp, int x, int y, int w, int h)
		: control<cgv::gui::shortcut>(_label, acp, &value),
		  CW<fltk::Input>(x,y,w,h,_label.c_str())
	{
		label(get_name().c_str());
		update();
	}
	std::string get_property_declarations()
	{
		return fltk_base::get_property_declarations();
	}
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
	{
		return fltk_base::set_void(this, this, property, value_type, value_ptr);
	}
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr)
	{
		return fltk_base::get_void(this, this, property, value_type, value_ptr);
	}
	void* get_user_data() const
	{
		return const_cast<fltk::Widget*>(static_cast<const fltk::Widget*>(this));
	}
	std::string get_type_name() const
	{
		return "fltk_shortcut_control";
	}
	void update()
	{
		shortcut_str = to_string(get_value());
		value(shortcut_str.c_str());
	}
	int handle(int event)
	{
		if (event == fltk::KEYUP) {
			switch (fltk::event_key()) {
			case fltk::LeftShiftKey	: 
			case fltk::RightShiftKey :
			case fltk::LeftCtrlKey	: 
			case fltk::RightCtrlKey	: 
			case fltk::CapsLockKey	: 
			case fltk::LeftMetaKey	: 
			case fltk::RightMetaKey	: 
			case fltk::LeftAltKey	: 
			case fltk::RightAltKey	: 
			case fltk::ScrollLockKey:
			case fltk::NumLockKey   :
				if (last_sc.get_key() == cgv_key(fltk::event_key())) {
					if (check_and_set_value(last_sc)) {
						shortcut_str = to_string(last_sc);
						value(shortcut_str.c_str());
						update_views();
					}
					return true;
				}
				break;
			}
			return fltk::Input::handle(event);
		}
		if (event == fltk::KEY) {
			last_sc = cgv::gui::shortcut();
			switch (fltk::event_key()) {
			case fltk::LeftShiftKey	: 
			case fltk::RightShiftKey :
			case fltk::LeftCtrlKey	: 
			case fltk::RightCtrlKey	: 
			case fltk::CapsLockKey	: 
			case fltk::LeftMetaKey	: 
			case fltk::RightMetaKey	: 
			case fltk::LeftAltKey	: 
			case fltk::RightAltKey	: 
			case fltk::ScrollLockKey:
			case fltk::TabKey       :
			case fltk::NumLockKey   :
				last_sc = cgv_shortcut(fltk::event_key()|fltk::event_state());
				return fltk::Input::handle(event);
			case fltk::DeleteKey :
				if (get_value().get_key() != 0) {
					cgv::gui::shortcut sc;
					if (check_and_set_value(sc)) {
						value("");
						update_views();
					}
					return true;
				}
				break;
			default: break;
			}
			cgv::gui::shortcut sc = cgv_shortcut(fltk::event_key()|fltk::event_state());
			if (check_and_set_value(sc)) {
				shortcut_str = to_string(sc);
				value(shortcut_str.c_str());
				update_views();
			}
			return true;
		}
		return fltk::Input::handle(event);
	}
};

struct shortcut_control_factory : public abst_control_factory
{
	control_ptr create(const std::string& label, 
		void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
		const std::string& gui_type, int x, int y, int w, int h)
	{
		if (value_type == "cgv::gui::shortcut" || value_type == "shortcut")
			return control_ptr(new fltk_shortcut_control(label, 
			*static_cast<cgv::gui::shortcut*>(value_ptr), acp, x, y, w, h));
		return control_ptr();
	}
};

control_factory_registration<shortcut_control_factory> shortcut_control_fac_reg;