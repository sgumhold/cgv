#include "lifetime_event.h"

namespace cgv {
	namespace nui {

std::string get_lifetime_type_string(LifetimeEventType type)
{
	switch (type) {
	case LET_BIRTH:    return "birth";
	case LET_EVOLVE:   return "evolve";
	case LET_DEATH:   return "death";
	default: return "unknown";
	}
}

lifetime_event::lifetime_event(bool birth, unsigned char _modifiers, unsigned char _toggle_keys, double _time)
	: nui_event(gui::EID_LIFETIME, _modifiers, _toggle_keys, _time), type(birth ? LET_BIRTH : LET_DEATH), dt(0)
{
}
lifetime_event::lifetime_event(float _dt, unsigned char _modifiers, unsigned char _toggle_keys, double _time)
	: nui_event(gui::EID_LIFETIME, _modifiers, _toggle_keys, _time), type(LET_EVOLVE), dt(_dt)
{
}

/// write to stream
void lifetime_event::stream_out(std::ostream& os) const
{
	nui_event::stream_out(os);
	os << " " << get_lifetime_type_string(type);
	if (type == LET_EVOLVE)
		os << " <" << dt << ">";
}
/// read from stream
void lifetime_event::stream_in(std::istream& is)
{
	std::cerr << "lifetime_event::stream_in not implemented yet" << std::endl;
}
/// return the type of the lifetime event
LifetimeEventType lifetime_event::get_type() const
{
	return type;
}

void lifetime_event::set_type(LifetimeEventType _type)
{
	type = _type;
}

float lifetime_event::get_dt() const
{
	return dt;
}
void lifetime_event::set_dt(float _dt)
{
	dt = _dt;
}

	}
}