#define _USE_MATH_DEFINES
#include <cmath>
#include "key_control.h"
#include "key_event.h"
#include "trigger.h"
#include <cgv/type/variant.h>
#include <cgv/type/info/type_name.h>
#include <cgv/utils/convert.h>

using namespace cgv::utils;

namespace cgv {
	namespace gui {

template <typename T>
std::string key_control<T>::get_type_name() const
{
	return cgv::type::info::type_name<key_control<T> >::get_name();
}

template <typename T>
bool key_control<T>::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return 
		srh.reflect_member("speed", speed) &&
		srh.reflect_member("min", min_value) &&
		srh.reflect_member("max", max_value) &&
		srh.reflect_member("log", log_scale) &&
		srh.reflect_member("no_limits", no_limits);
}

template <typename T>
std::string key_control<T>::get_property_declarations()
{
	return cgv::base::base::get_property_declarations()+";more:shortcut;less:shortcut";
}

template <typename T>
bool key_control<T>::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (cgv::base::base::set_void(property, value_type, value_ptr))
		return true;
	if (property == "more") {
		if (value_type == "cgv::gui::shortcut")
			more = *((const shortcut*)value_ptr);
		else if (value_type == "string")
			return from_string(more, *((const std::string*)value_ptr));
		else
			return false;
		return true;
	}
	if (property == "less") {
		if (value_type == "cgv::gui::shortcut")
			less = *((const shortcut*)value_ptr);
		else if (value_type == "string")
			return from_string(less, *((const std::string*)value_ptr));
		else
			return false;
		return true;
	}
	return false;
}

template <typename T>
bool key_control<T>::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return cgv::base::base::get_void(property, value_type, value_ptr);
}

template <typename T>
void key_control<T>::on_set(void* member_ptr)
{
	if (member_ptr == &min_value || member_ptr == &max_value)
		no_limits = false;
}

template <typename T>
key_control<T>::key_control(const std::string& _name, T& _value, const std::string& options) : control<T>(_name,_value)
{
	speed = 1;
	min_value = 0;
	max_value = 1;
	log_scale = false;
	no_limits = true;
	connect(get_animation_trigger().shoot, this, &key_control<T>::timer_event);

	increase_pressed_time = 0;
	decrease_pressed_time = 0;

	this->multi_set(options, true);
}

template <typename T>
void key_control<T>::change_value(double dt)
{
	if (no_limits || (dt > 0 && control<T>::get_value() < max_value) || (dt < 0 && control<T>::get_value() > min_value)) {
		T nv = control<T>::get_value();
		if (log_scale)
			nv *= (T)exp(speed*dt);
		else
			nv += (T)(speed*dt);
		if (!no_limits) {
			if (nv > max_value)
				nv = max_value;
			else if (nv < min_value)
				nv = min_value;
		}
		if (nv != control<T>::get_value())
			this->check_and_set_value(nv);
	}
}

template <typename T>
bool key_control<T>::handle(event& e)
{
	if (e.get_kind() != EID_KEY)
		return false;
	key_event& ke = static_cast<key_event&>(e);
	if (ke.get_action() == KA_PRESS) {
		if (ke.get_key() == more.get_key() && ke.get_modifiers() == more.get_modifiers()) {
			if (increase_pressed_time == 0)
				increase_pressed_time = e.get_time();
			return true;
		}
		if (ke.get_key() == less.get_key() && ke.get_modifiers() == less.get_modifiers()) {
			if (decrease_pressed_time == 0)
				decrease_pressed_time = e.get_time();
			return true;
		}
	}
	else {
		if (ke.get_key() == more.get_key() && increase_pressed_time != 0) {
			change_value(ke.get_time()-increase_pressed_time);
			increase_pressed_time = 0;
			return true;
		}
		if (ke.get_key() == less.get_key() && decrease_pressed_time != 0) {
			change_value(decrease_pressed_time-ke.get_time());
			decrease_pressed_time = 0;
			return true;
		}
	}
	return false; 
}


template <typename T>
void key_control<T>::stream_help(std::ostream& os)
{
	os << this->get_name() << ": <" << less << "," << more << ">" << std::endl;
}

template <typename T>
void key_control<T>::timer_event(double time, double dt)
{
	if (increase_pressed_time != 0) {
		double time = get_animation_trigger().get_current_time();
		change_value(time-increase_pressed_time);
		increase_pressed_time = time;
	}
	if (decrease_pressed_time != 0) {
		double time = get_animation_trigger().get_current_time();
		change_value(decrease_pressed_time-time);
		decrease_pressed_time = time;
	}
}

std::string key_control<bool>::get_type_name() const
{
	return "key_control<bool>";
}

std::string key_control<bool>::get_property_declarations()
{
	return "toggle:shortcut";
}

bool key_control<bool>::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "toggle") {
		if (value_type == "shortcut")
			toggle = *((const shortcut*)value_ptr);
		else if (value_type == "string")
			return from_string(toggle, *((const std::string*)value_ptr));
		else
			return false;
		return true;
	}
	return false;
}

bool key_control<bool>::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return cgv::base::base::get_void(property, value_type, value_ptr);
}

key_control<bool>::key_control(const std::string& _name, bool& _value, const std::string& options): control<bool>(_name,_value)
{
	multi_set(options, true);
}

bool key_control<bool>::handle(event& e)
{
	if (e.get_kind() != EID_KEY)
		return false;
	key_event& ke = static_cast<key_event&>(e);
	if (ke.get_action() != KA_PRESS)
		return false;
	if (ke.get_key() == toggle.get_key() && ke.get_modifiers() == toggle.get_modifiers()) {
		check_and_set_value(!get_value());
		return true;
	}
	return false;
}

void key_control<bool>::stream_help(std::ostream& os)
{
	os << "toggle " << get_name() << ": <" << toggle << ">" << std::endl;
}


#include <cgv/gui/lib_begin.h>
template class CGV_API key_control<double>;
template class CGV_API key_control<float>;
#include <cgv/config/lib_end.h>


	}
}
