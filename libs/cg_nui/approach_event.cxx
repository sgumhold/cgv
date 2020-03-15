#include "approach_event.h"

namespace cgv {
	namespace nui {

std::string get_approach_type_string(ApproachEventType type)
{
	switch (type) {
	case AET_OVER:    return "over";
	case AET_EVADE:   return "evade";
	case AET_TOUCH:   return "touch";
	case AET_UNTOUCH: return "untouch";
	case AET_PRESS:   return "press";
	case AET_RELEASE: return "release";
	default: return "unknown";
	}
}

/// construct grab or drop event
approach_event::approach_event(ApproachEventType _type, const vec3& _contact_point, unsigned char _modifiers, unsigned char _toggle_keys, double _time)
	: nui_event(gui::EID_APPROACH, _modifiers, _toggle_keys, _time), type(_type), contact_point(_contact_point)
{
}

/// write to stream
void approach_event::stream_out(std::ostream& os) const
{
	nui_event::stream_out(os);
	os << " " << get_approach_type_string(type);
	os << "<" << contact_point << ">";
}
/// read from stream
void approach_event::stream_in(std::istream& is)
{
	std::cerr << "approach_event::stream_in not implemented yet" << std::endl;
}
/// return the type of the approach event
ApproachEventType approach_event::get_type() const
{
	return type;
}

/// set the type of the approach event
void approach_event::set_type(ApproachEventType _type)
{
	type = _type;
}

/// return contact point
const approach_event::vec3& approach_event::get_contact_point() const
{
	return contact_point;
}
/// set new contact point
	void approach_event::set_contact_point(const approach_event::vec3& _contact_point)
{
		contact_point = _contact_point;
}

	}
}