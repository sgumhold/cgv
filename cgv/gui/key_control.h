#pragma once

#include <cgv/base/node.h>
#include "shortcut.h"
#include "control.h"
#include "event.h"
#include "event_handler.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {


template <typename T>
class CGV_API key_control : public control<T>, public event_handler, virtual public cgv::signal::tacker
{
private:
	double increase_pressed_time;
	double decrease_pressed_time;
protected:
	T  speed;
	T  min_value;
	T  max_value;
	bool log_scale;
	bool no_limits;
	shortcut more;
	shortcut less;
	void change_value(double dt);
public:
	key_control(const std::string& name, T& _value, const std::string& options = "");
	std::string get_type_name() const;
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	std::string get_property_declarations();
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	void on_set(void* member_ptr);
	bool handle(event& e);
	void stream_help(std::ostream& os);
	void timer_event(double time, double dt);
};

template <> class CGV_API key_control<bool> : public control<bool>, public event_handler
{
protected:
	shortcut toggle;
	void change_value(double dt);
public:
	key_control(const std::string& name, bool& _value, const std::string& options = "");
	std::string get_type_name() const;
	std::string get_property_declarations();
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	bool handle(event& e);
	void stream_help(std::ostream& os);
};


	}
}

#include <cgv/config/lib_end.h>