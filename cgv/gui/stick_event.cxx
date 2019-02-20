#include "stick_event.h"

namespace cgv {
	namespace gui {


/// convert a key action into a readable string
std::string get_stick_action_string(StickAction action)
{
	switch (action) {
	case SA_TOUCH:   return "touch";
	case SA_PRESS:   return "press";
	case SA_UNPRESS: return "unpress";
	case SA_RELEASE: return "release";
	case SA_MOVE:    return "move";
	case SA_DRAG:    return "drag";
	default: return "unknown";
	}
}

/// construct a key event from its textual description 
stick_event::stick_event(StickAction _action, float _x, float _y, float _dx, float _dy, 
	unsigned _player_index, unsigned _controller_index, unsigned _stick_index, double _time)
	: event(EID_STICK, 0,0,_time), action(_action), x(_x), y(_y), dx(_dx), dy(_dy),
	player_index(_player_index), controller_index(_controller_index), stick_index(_stick_index)
{}

/// return player index
unsigned stick_event::get_player_index() const
{
	return player_index;
}
/// return controller index
unsigned stick_event::get_controller_index() const
{
	return controller_index;
}
/// return stick index
unsigned stick_event::get_stick_index() const
{
	return stick_index;
}

/// write to stream
void stick_event::stream_out(std::ostream& os) const
{
	event::stream_out(os);
	os << get_stick_action_string(StickAction(action)) << "(" << x << "," << y << ")[";
	if (dx > 0)
		os << "+";
	os << dx << ",";
	if (dy > 0)
		os << "+";
	os << dy << "]";
	os << "<" << (int)player_index << ":" << (int)controller_index << ":" << (int)stick_index << ">";
}
/// read from stream
void stick_event::stream_in(std::istream& is)
{
	std::cout << "not implemented" << std::endl;
}
/// return the stick action
StickAction stick_event::get_action() const
{
	return StickAction(action);
}

/// return the current x value of the stick
float stick_event::get_x() const
{
	return x;
}
/// return the current y value of the stick
float stick_event::get_y() const
{
	return y;
}
/// return the current change in x value of the stick
float stick_event::get_dx() const
{
	return dx;
}
/// return the current change in y value of the stick
float stick_event::get_dy() const
{
	return dy;
}
/// return the last x value of the stick
float stick_event::get_last_x() const
{
	return x - dx;
}
/// return the last y value of the stick
float stick_event::get_last_y() const
{
	return y - dy;
}

	}
}
