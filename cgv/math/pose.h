#pragma	once

#include "fvec.h"
#include "fmat.h"
#include "quaternion.h"
#include <cassert>

/**@file
   helper functions to work with poses that can be represented with 3x4 matrix or quaternion plus vector
*/

namespace cgv { ///
	namespace math { ///

/// extract orientation matrix from pose matrix
template <typename T> fmat<T, 3, 3>& pose_orientation(fmat<T, 3, 4>& pose) { return reinterpret_cast<fmat<T, 3, 3>&>(pose); }
template <typename T> const fmat<T, 3, 3>& pose_orientation(const fmat<T, 3, 4>& pose) { return reinterpret_cast<const fmat<T, 3, 3>&>(pose); }
/// extract position vector from pose matrix
template <typename T> fvec<T, 3>& pose_position(fmat<T, 3, 4>& pose) { return reinterpret_cast<fvec<T, 3>&>(pose(0, 3)); }
template <typename T> const fvec<T, 3>& pose_position(const fmat<T, 3, 4>& pose) { return reinterpret_cast<const fvec<T, 3>&>(pose(0, 3)); }

/// transform point with pose matrix
template <typename T>
fvec<T, 3> pose_transform_point(const fmat<T, 3, 4>& pose, const fvec<T, 3>& p) { return pose_orientation(pose)* p + pose_position(pose); }
/// transform point with pose quaternion-position pair
template <typename T>
fvec<T, 3> pose_transform_point(const quaternion<T>& q, const fvec<T,3>& pos, const fvec<T, 3>& p) { return q.get_rotated(p) + pos; }
/// transform vector with pose matrix
template <typename T>
fvec<T, 3> pose_transform_vector(const fmat<T, 3, 4>& pose, const fvec<T, 3>& v) { return pose_orientation(pose) * v; }
/// transform point with inverse of pose matrix
template <typename T>
fvec<T, 3> inverse_pose_transform_point(const fmat<T, 3, 4>& pose, const fvec<T, 3>& p) { return (p - pose_position(pose))*pose_orientation(pose); }
/// transform vector with inverse of pose matrix
template <typename T>
fvec<T, 3> inverse_pose_transform_vector(const fmat<T, 3, 4>& pose, const fvec<T, 3>& v) { return v * pose_orientation(pose); }
/// inplace inversion of pose transformation
template <typename T> void invert_pose(fmat<T, 3, 4>& pose) { pose_orientation(pose).transpose(); pose_position(pose) = -pose_orientation(pose)*pose_position(pose); }
/// return a pose matrix with the inverse pose transformation
template <typename T> fmat<T, 3, 4> pose_inverse(const fmat<T, 3, 4>& pose) { fmat<T, 3, 4> inv_pose = pose; invert_pose(inv_pose); return inv_pose; }
/// construct pose from rotation matrix and position vector
template <typename T> fmat<T, 3, 4> pose_construct(const fmat<T, 3, 3>& orientation, const fvec<T, 3>& position) { fmat<T, 3, 4> pose; pose_orientation(pose) = orientation; pose_position(pose) = position; return pose; }
/// construct pose from rotation quaternion and position vector
template <typename T> fmat<T, 3, 4> pose_construct(const quaternion<T>& orientation, const fvec<T, 3>& position) { fmat<T, 3, 4> pose; orientation.put_matrix(pose_orientation(pose)); pose_position(pose) = position; return pose; }
/// inplace concatenation of a pose matrix
template <typename T> void pose_append(fmat<T, 3, 4>& pose_1, const fmat<T, 3, 4>& pose_2) {
	pose_position(pose_1) += pose_orientation(pose_1)*pose_position(pose_2);
	pose_orientation(pose_1) *= pose_orientation(pose_2);
}
/// inplace transformation of a pose matrix with another pose transformation matrix
template <typename T> void pose_transform(const fmat<T, 3, 4>& pose_transform, fmat<T, 3, 4>& pose) {
	pose_position(pose) = pose_position(pose_transform) + pose_orientation(pose_transform)*pose_position(pose);
	pose_orientation(pose) = pose_orientation(pose_transform)*pose_orientation(pose);
}
/// return concatenate of two pose transformations
template <typename T> fmat<T, 3, 4> pose_concat(const fmat<T, 3, 4>& pose_1, const fmat<T, 3, 4>& pose_2) {
	fmat<T, 3, 4> pose = pose_1;
	pose_append(pose, pose_2);
	return pose;
}

	}
}