#pragma once

#include "event.h"
#include "shortcut.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// class to represent all possible keyboard events with the EID_KEY
class CGV_API pose_event : public event
{
protected:
	short player_index, trackable_index;
	/// pose stored as 3x4 matrix in column major format
	float pose[12];
public:
	/// construct a key event from its textual description 
	pose_event(const float *_pose, unsigned short _player_index, short _trackable_index  = 0, double _time = 0);
	/// return player index
	unsigned get_player_index() const;
	/// return trackable index
	int get_trackable_index() const;
	/// write to stream
	void stream_out(std::ostream& os) const;
	/// read from stream
	void stream_in(std::istream& is);
	/// return the key being a capital letter, digit or a value from the Keys enum
	const float* get_pose() const;
};

	}
}

#include <cgv/config/lib_end.h>