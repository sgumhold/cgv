#pragma once

#include "event.h"
#include "shortcut.h"
#include <cgv/math/fvec.h>

#include "lib_begin.h"

///
namespace cgv {
	///
	namespace gui {

/// different actions that a stick can perform
enum StickAction { 
	SA_TOUCH,   //!< stick touch action
	SA_PRESS,   //!< stick press action
	SA_UNPRESS, //!< stick unpress repeated press action
	SA_RELEASE, //!< stick release action
	SA_MOVE,    //!< stick moved with respect to last event
	SA_DRAG     //!< stick moved in pressed state
};

/// convert a stick action into a readable string
extern CGV_API std::string get_stick_action_string(StickAction action);

/// class to represent stick events with the EID_STICK
class CGV_API stick_event : public event
{
public:
	/// declare 2d vector type 
	typedef cgv::math::fvec<float, 2> vec2;
protected:
	// store different indices to uniquely define stick
	unsigned char player_index, controller_index, stick_index;
	/// store stick action
	unsigned char action;
	/// current stick location
	vec2 position;
	/// change in stick location
	vec2 difference;
public:
	/// construct a key event from its textual description 
	stick_event(StickAction _action, float _x, float _y, float _dx, float _dy, 
		unsigned _player_index = 0, unsigned _controller_index = 0, unsigned _stick_index = 0, double _time = 0);
	/// write to stream
	void stream_out(std::ostream& os) const;
	/// read from stream
	void stream_in(std::istream& is);
	/// return the stick action
	StickAction get_action() const;
	/// return player index
	unsigned get_player_index() const;
	/// return controller index
	unsigned get_controller_index() const;
	/// return stick index
	unsigned get_stick_index() const;
	/// return the current x value of the stick
	float get_x() const;
	/// return the current y value of the stick
	float get_y() const;
	/// return current position
	const vec2& get_position() const;
	/// return the current change in x value of the stick
	float get_dx() const;
	/// return the current change in y value of the stick
	float get_dy() const;
	/// return the vector of coordinate differences (dx,dy)
	const vec2& get_difference() const;
	/// return the last x value of the stick
	float get_last_x() const;
	/// return the last y value of the stick
	float get_last_y() const;
	/// return last position
	vec2 get_last_position() const;
};

	}
}

#include <cgv/config/lib_end.h>