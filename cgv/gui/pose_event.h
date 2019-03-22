#pragma once

#include "event.h"
#include "shortcut.h"
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/math/quaternion.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// class to represent all pose events from tracking systems with the EID_POSE
class CGV_API pose_event : public event
{
public:
	/// declare 3d vector type 
	typedef cgv::math::fvec<float, 3> vec3;
	/// declare 3d matrix type 
	typedef cgv::math::fmat<float, 3, 3> mat3;
	/// declare 3x4 matrix type 
	typedef cgv::math::fmat<float, 3, 4> mat3x4;
	/// declare quaternion type
	typedef cgv::math::quaternion<float> quat;
protected:
	// store different indices to uniquely define trackable
	short player_index, trackable_index;
	/// pose stored as 3x4 matrix in column major format
	float pose[12];
	/// last pose as 3x4 matrix
	float last_pose[12];
public:
	/// construct a key event from its textual description 
	pose_event(const float *_pose, const float *_last_pose, unsigned short _player_index, short _trackable_index  = 0, double _time = 0);
	/// return player index
	unsigned get_player_index() const;
	/// return trackable index
	int get_trackable_index() const;
	/// write to stream
	void stream_out(std::ostream& os) const;
	/// read from stream
	void stream_in(std::istream& is);
	/// return current orientation matrix
	const mat3& get_orientation() const;
	/// return current position
	const vec3& get_position() const;
	/// return current pose matrix
	const mat3x4& get_pose_matrix() const;
	/// return current orientation quaternion
	quat get_quaternion() const;
	/// return last orientation matrix
	const mat3& get_last_orientation() const;
	/// return last position
	const vec3& get_last_position() const;
	/// return last pose matrix
	const mat3x4& get_last_pose_matrix() const;
	/// return last orientation quaternion
	quat get_last_quaternion() const;
	/// return difference vector from last to current position
	vec3 get_different() const;
	/// return rotation matrix between from the last to current orientation
	mat3 get_rotation_matrix() const;
	/// return rotation quaternion between from the last to current orientation
	quat get_rotation_quaternion() const;
};

	}
}

#include <cgv/config/lib_end.h>