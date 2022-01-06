#include "action_event.h"

namespace cgv {
	namespace nui {

std::string get_action_type_string(ActionEventType type)
{
	switch (type) {
	case AET_GRAB:  return "grab";
	case AET_DROP:  return "drop";
	case AET_MOVE:  return "move";
	case AET_THROW: return "throw";
	default: return "unknown";
	}
}

/// construct grab or drop event
action_event::action_event(bool grab, const vec3& _pick_point, unsigned char _modifiers, unsigned char _toggle_keys, double _time)
	: nui_event(gui::EID_ACTION, _modifiers, _toggle_keys, _time), type(grab ? AET_GRAB : AET_DROP), dt(0), pick_point(_pick_point)
{
}

/// construct move event
action_event::action_event(const vec3& _pick_point, float _dt, const quat& _rotation, const vec3& _translation, unsigned char _modifiers, unsigned char _toggle_keys, double _time)
	: nui_event(gui::EID_ACTION, _modifiers, _toggle_keys, _time), type(AET_MOVE), dt(_dt), pick_point(_pick_point)
{
	rotation = _rotation;
	translation = _translation;	
}

/// construct throw
action_event::action_event(const vec3& _pick_point, const vec3& _angular_velocity, const vec3& _linear_velocity, const quat& _rotation, unsigned char _modifiers, unsigned char _toggle_keys, double _time)
	: nui_event(gui::EID_ACTION, _modifiers, _toggle_keys, _time), type(AET_THROW), dt(0), pick_point(_pick_point)
{
	angular_velocity = _angular_velocity;
	linear_velocity = _linear_velocity;
}

/// write to stream
void action_event::stream_out(std::ostream& os) const
{
	nui_event::stream_out(os);
	os << " " << get_action_type_string(type);
	os << "<" << pick_point << ">";
	switch (type) {
		case AET_MOVE  :
			os << " " << dt << " [" << rotation << "|" << translation << "]";
			break;
		case AET_THROW :
			os << " " << " {" << angular_velocity << "|" << linear_velocity << "}";
			break;
	}
}
/// read from stream
void action_event::stream_in(std::istream& is)
{
	std::cerr << "approach_event::stream_in not implemented yet" << std::endl;
}
/// return the type of the action event
ActionEventType action_event::get_type() const
{
	return type;
}

/// set the type of the action event
void action_event::set_type(ActionEventType _type)
{
	type = _type;
}

	}
}