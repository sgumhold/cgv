#include "throttle_event.h"

namespace cgv {
	namespace gui {

/// construct a throttle event from value and value change
throttle_event::throttle_event(float _x, float _dx, unsigned _player_index, unsigned _controller_index, unsigned _throttle_index, double _time) :
	event(EID_THROTTLE,0,0,_time), x(_x), dx(_dx), player_index(_player_index), controller_index(_controller_index), throttle_index(_throttle_index)
{
}

/// return player index
unsigned throttle_event::get_player_index() const
{
	return player_index;
}
/// return controller index
unsigned throttle_event::get_controller_index() const
{
	return controller_index;
}
/// return stick index
unsigned throttle_event::get_throttle_index() const
{
	return throttle_index;
}


/// write to stream
void throttle_event::stream_out(std::ostream& os) const
{
	event::stream_out(os);
	os << x << "[";
	if (dx > 0)
		os << "+";
	os << dx << "]";
	os << "<" << (int)player_index << ":" << (int)controller_index << ":" << (int)throttle_index << ">";
}

/// read from stream
void throttle_event::stream_in(std::istream& is)
{
	std::cerr << "not implemented" << std::endl;
}

/// return the current value of the throttle
float throttle_event::get_value() const
{
	return x;
}

/// return the change of value of the throttle
float throttle_event::get_value_change() const
{
	return dx;
}

/// return the last value of the throttle
float throttle_event::get_last_value() const
{
	return get_value() - get_value_change();
}
	}
}
