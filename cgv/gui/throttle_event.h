#pragma once

#include "event.h"
#include "shortcut.h"

#include "lib_begin.h"

///
namespace cgv {
	///
	namespace gui {

/// class to represent events that inform on a change in a one axis controller with the EID_THROTTLE
class CGV_API throttle_event : public event
{
protected:
	// store different indices to uniquely define throttle
	unsigned char player_index, controller_index, throttle_index;
	// current value of throttle
	float x;
	// change with respect to previous value of throttle
	float dx;
public:
	/// construct a throttle event from value and value change
	throttle_event(float _x, float _dx, unsigned _player_index = 0, unsigned _controller_index = 0, unsigned _throttle_index = 0, double _time = 0);
	/// return player index
	unsigned get_player_index() const;
	/// return controller index
	unsigned get_controller_index() const;
	/// return throttle index
	unsigned get_throttle_index() const;
	/// write to stream
	void stream_out(std::ostream& os) const;
	/// read from stream
	void stream_in(std::istream& is);
	/// return the current value of the throttle
	float get_value() const;
	/// return the change of value of the throttle
	float get_value_change() const;
	/// return the last value of the throttle
	float get_last_value() const;
};

	}
}

#include <cgv/config/lib_end.h>