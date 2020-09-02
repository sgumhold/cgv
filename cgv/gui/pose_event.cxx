#include "pose_event.h"
#include "shortcut.h"

namespace cgv {
	namespace gui {

/// construct a key event from its textual description 
pose_event::pose_event(const float *_pose, const float *_last_pose, unsigned short _player_index, short _trackable_index, double _time)
	: event(EID_POSE,0,0,_time), player_index(_player_index), trackable_index(_trackable_index)
{
	std::copy(_pose, _pose + 12, pose);
	std::copy(_last_pose, _last_pose + 12, last_pose);
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
	os << "x(" << pose[0] << "," << pose[1] << "," << pose[2] << ");";
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

/// return current orientation matrix
const pose_event::mat3& pose_event::get_orientation() const
{
	return reinterpret_cast<const mat3&>(pose[0]);
}

/// return current position
const pose_event::vec3& pose_event::get_position() const
{
	return reinterpret_cast<const vec3&>(pose[9]);
}
/// return current pose matrix
const pose_event::mat3x4& pose_event::get_pose_matrix() const
{
	return reinterpret_cast<const mat3x4&>(pose[0]);
}
/// return current orientation quaternion
pose_event::quat pose_event::get_quaternion() const
{
	return quat(get_orientation());
}
/// return last orientation matrix
const pose_event::mat3& pose_event::get_last_orientation() const
{
	return reinterpret_cast<const mat3&>(last_pose[0]);
}
/// return last position
const pose_event::vec3& pose_event::get_last_position() const
{
	return reinterpret_cast<const vec3&>(last_pose[9]);
}
/// return last pose matrix
const pose_event::mat3x4& pose_event::get_last_pose_matrix() const
{
	return reinterpret_cast<const mat3x4&>(last_pose[0]);
}

/// return last orientation quaternion
pose_event::quat pose_event::get_last_quaternion() const
{
	return quat(get_last_orientation());
}

/// return difference vector from last to current position
pose_event::vec3 pose_event::get_different() const
{
	return get_position() - get_last_position();
}
/// return rotation matrix between from the last to current orientation
pose_event::mat3 pose_event::get_rotation_matrix() const
{
	return get_orientation() * transpose(get_last_orientation());
}
/// return rotation quaternion between from the last to current orientation
pose_event::quat pose_event::get_rotation_quaternion() const
{
	return quat(get_rotation_matrix());
}

	}
}
