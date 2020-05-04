#include "choice_event.h"

namespace cgv {
	namespace gui {

/// convert a choice event type into a readable string
std::string get_choice_type_string(ChoiceEventType type)
{
	switch (type) {
	case CET_GRAB_FOCUS: return "grab_focus";
	case CET_LOOSE_FOCUS: return "loose_focus";
	case CET_SELECTED: return "selected";
	case CET_UNSELECTED: return "unselected";
	default: return "unknown";
	}
}

choice_event::choice_event(ChoiceEventType _type, unsigned char _modifiers, unsigned char _toggle_keys, double _time)
	: event(EID_CHOICE,_modifiers, _toggle_keys, _time), type(_type)
{
}

// write to stream
void choice_event::stream_out(std::ostream& os) const
{
	event::stream_out(os);
	os << " " << get_choice_type_string(type);
}

// read from stream
void choice_event::stream_in(std::istream&)
{
	std::cerr << "choice_event::stream_in not implemented yet" << std::endl;
}
/// return whether focus was grabbed
ChoiceEventType choice_event::get_type() const
{
	return type;
}

/// set the key %event action
void choice_event::set_type(ChoiceEventType  _type)
{
	type = _type;
}

	}
}
