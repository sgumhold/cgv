#pragma once

#include <vr/vr_kit.h>
#include <vr/vr_state.h>
#include <cgv/render/render_types.h>

#include "lib_begin.h"

///@ingroup VR
///@{

//! provides functions bridging the basic vr support with the cgv framework
/*! the helper functions allow to compute the modelview, projection and texture matrices needed for rendering. */

///
namespace vr {
	/// convert pose to mat4
	extern CGV_API cgv::render::render_types::mat4 get_mat4_from_pose(const float pose_matrix[12]);
	/// compute lookat matrix for a given eye (0 ... left, 1 ... right)
	extern CGV_API cgv::render::render_types::mat4 get_world_to_eye_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, int eye);
	/// compute lookat matrix for a given camera (0 ... left, 1 ... right)
	extern CGV_API cgv::render::render_types::mat4 get_world_to_camera_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, int eye);
	/// query projection matrix for a given eye (0 ... left, 1 ... right)
	extern CGV_API cgv::render::render_types::mat4 get_eye_projection_transform(const vr_kit* vr_kit_ptr, float z_near, float z_far, int eye);
	/// query projection matrix for a given camera (0 ... left or mono, 1 ... right only for stereo cameras)
	extern CGV_API cgv::render::render_types::mat4 get_camera_projection_transform(const vr_kit* vr_kit_ptr, float z_near, float z_far, int eye, bool undistorted);
	/// query the texture matrix needed for projective texture mapping for a given camera (0 ... left or mono, 1 ... right only for stereo cameras)
	extern CGV_API cgv::render::render_types::mat4 get_texture_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, float z_near, float z_far, int eye, bool undistorted);
}

#include <cgv/config/lib_end.h>
