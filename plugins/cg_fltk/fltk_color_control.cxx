#include "fltk_base.h"
#include "fltk_driver_registry.h"
#include <cgv/signal/signal.h>
#include <cgv/gui/control.h>
#include <cgv/media/color.h>
#include <cgv/type/standard_types.h>

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Widget.h>
#include <fltk/events.h>
#include <fltk/Button.h>
#include <fltk/Color.h>
#include <fltk/ColorChooser.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif

using namespace cgv::media;
using namespace cgv::type;
using namespace cgv::gui;

#include <iostream>

template <typename T, AlphaModel am>
struct choose_color
{
	static bool choose(color<T,RGB,am>& c)
	{
		return false;
	}
};

template <>
struct choose_color<float,NO_ALPHA>
{
	static bool choose(color<float,RGB,NO_ALPHA>& c)
	{
		return fltk::color_chooser("choose a new color", c.R(), c.G(), c.B());
	}
};

template <>
struct choose_color<float,OPACITY>
{
	static bool choose(color<float,RGB,OPACITY>& c)
	{
		return fltk::color_chooser("choose a new color", c.R(), c.G(), c.B(), c.opacity());
	}
};

template <>
struct choose_color<unsigned char,NO_ALPHA>
{
	static bool choose(color<unsigned char,RGB,NO_ALPHA>& c)
	{
		return fltk::color_chooser("choose a new color", c.R(), c.G(), c.B());
	}
};

template <>
struct choose_color<unsigned char,OPACITY>
{
	static bool choose(color<unsigned char,RGB,OPACITY>& c)
	{
		return fltk::color_chooser("choose a new color", c.R(), c.G(), c.B(), c.opacity());
	}
};

struct on_click_cb_handler
{
	virtual void on_click() = 0;
};

void color_button_cb(fltk::Widget* w, void* user_data)
{
	static_cast<cgv::base::base*>(user_data)->get_interface<on_click_cb_handler>()->on_click();
}

template <typename T, AlphaModel am>
struct fltk_color_control : public control<color<T,RGB,am> >, public fltk_base, public on_click_cb_handler, public cgv::signal::tacker
{
	CW<fltk::Button>* view;
	fltk_color_control(const std::string& label, color<T,RGB,am>* cp,
		abst_control_provider* acp, int x, int y, int w, int h) 
			: control<color<T,RGB,am> >(label,acp,cp)

	{
		view = new CW<fltk::Button>(x,y,w,h);
		view->label(this->get_name().c_str());
		view->align(fltk::ALIGN_LEFT);
		view->callback(color_button_cb, static_cast<cgv::base::base*>(this));
		update();
	}
	void update()
	{
		fltk::Color c = fltk::color(
			(unsigned char)	(255.0*this->get_value().R()/color_one<T>::value()),
			(unsigned char)	(255.0*this->get_value().G()/color_one<T>::value()),
			(unsigned char)	(255.0*this->get_value().B()/color_one<T>::value()));
		if (c == 0)
			c = fltk::color(1,1,1);
		view->color(c);
		view->redraw();
	}
	void on_click()
	{
		color<T,RGB,am> c = this->get_value();
		if (choose_color<T,am>::choose(c)) {
			this->set_new_value(c);
			if (this->check_value(*this)) {
				c = this->get_value();
				this->set_value(this->get_new_value());
				this->set_new_value(c);
				this->value_change(*this);
				update();
			}
		}
	}
	std::string get_type_name() const
	{
		return "fltk_line_decorator";
	}
	std::string get_property_declarations()
	{
		return fltk_base::get_property_declarations();
	}
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
	{
		if (fltk_base::set_void(view, this, property, value_type, value_ptr))
			return true;
		return false;
	}
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr)
	{
		if (fltk_base::get_void(view, this, property, value_type, value_ptr))
			return true;
		return false;
	}
	void* get_user_data() const
	{
		return view;
	}
};

struct color_control_factory : public abst_control_factory
{
	control_ptr create(const std::string& label, 
		void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
		const std::string& gui_type, int x, int y, int w, int h)
	{
		if (value_type == cgv::type::info::type_name<cgv::media::color<cgv::type::flt32_type> >::get_name())
			return control_ptr(new fltk_color_control<flt32_type,NO_ALPHA>(
				label, static_cast<color<flt32_type,RGB>*>(value_ptr), 
				acp, x, y, w, h));
		if (value_type == cgv::type::info::type_name<cgv::media::color<cgv::type::uint8_type> >::get_name())
			return control_ptr(new fltk_color_control<uint8_type,NO_ALPHA>(
				label, static_cast<color<uint8_type,RGB>*>(value_ptr), 
				acp, x, y, w, h));
		if (value_type == cgv::type::info::type_name<cgv::media::color<flt32_type,RGB,EXTINCTION> >::get_name() ||
			value_type == cgv::type::info::type_name<cgv::media::color<flt32_type,RGB,OPACITY> >::get_name() || 
			value_type == cgv::type::info::type_name<cgv::media::color<flt32_type,RGB,TRANSPARENCY> >::get_name())
			return control_ptr(new fltk_color_control<flt32_type,OPACITY>(
				label, static_cast<color<flt32_type,RGB,OPACITY>*>(value_ptr), 
				acp, x, y, w, h));
		if (value_type == cgv::type::info::type_name<cgv::media::color<uint8_type,RGB,EXTINCTION> >::get_name() ||
			value_type == cgv::type::info::type_name<cgv::media::color<uint8_type,RGB,OPACITY> >::get_name() || 
			value_type == cgv::type::info::type_name<cgv::media::color<uint8_type,RGB,TRANSPARENCY> >::get_name())
			return control_ptr(new fltk_color_control<uint8_type,OPACITY>(
				label, static_cast<color<uint8_type,RGB,OPACITY>*>(value_ptr), 
				acp, x, y, w, h));
		return control_ptr();
	}
};

control_factory_registration<color_control_factory> color_control_fac_reg;
