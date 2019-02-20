#include "pose_event.h"
#include "shortcut.h"

namespace cgv {
	namespace gui {

/// construct a key event from its textual description 
pose_event::pose_event(const float *_pose, unsigned short _player_index, short _trackable_index, double _time)
	: event(EID_POSE,0,0,_time), player_index(_player_index), trackable_index(_trackable_index)
{
	std::copy(_pose, _pose + 12, pose);
}

/// return player index
unsigned pose_event::get_player_index() const
{
	return player_index;
}
/// return trackable index
int pose_event::get_trackable_index() const
{
	return trackable_index;
}
/// write to stream
void pose_event::stream_out(std::ostream& os) const
{
	event::stream_out(os);
	os << "x(" << pose[0] << "," << pose[0] << "," << pose[2] << ");";
	os << "y(" << pose[3] << "," << pose[4] << "," << pose[5] << ");";
	os << "z(" << pose[6] << "," << pose[7] << "," << pose[8] << ");";
	os << "O(" << pose[9] << "," << pose[10] << "," << pose[11] << ");";
	os << "<" << player_index << ":" << trackable_index << ">";
}

/// read from stream
void pose_event::stream_in(std::istream& is)
{
	std::cerr << "not implemented" << std::endl;
}

/// return the key being a capital letter, digit or a value from the Keys enum
const float* pose_event::get_pose() const
{
	return pose;
}

	}
}
