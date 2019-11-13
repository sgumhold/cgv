#include "vr_render_helpers.h"
#include <cgv/math/ftransform.h>
#include <cgv/math/inv.h>

///
namespace vr {
	/// convert pose to mat4
	cgv::render::render_types::mat4 get_mat4_from_pose(const float pose_matrix[12])
	{
		cgv::render::render_types::mat4 M;
		M.set_col(0, cgv::render::render_types::vec4(reinterpret_cast<const cgv::render::render_types::vec3&>(pose_matrix[0]), 0));
		M.set_col(1, cgv::render::render_types::vec4(reinterpret_cast<const cgv::render::render_types::vec3&>(pose_matrix[3]), 0));
		M.set_col(2, cgv::render::render_types::vec4(reinterpret_cast<const cgv::render::render_types::vec3&>(pose_matrix[6]), 0));
		M.set_col(3, cgv::render::render_types::vec4(reinterpret_cast<const cgv::render::render_types::vec3&>(pose_matrix[9]), 1));
		return M;
	}
	/// compute lookat matrix for a given eye (0 ... left, 1 ... right)
	cgv::render::render_types::mat4 get_world_to_eye_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, int eye)
	{
		float eye_to_head[12];
		vr_kit_ptr->put_eye_to_head_matrix(eye, eye_to_head);
		return inv(get_mat4_from_pose(state.hmd.pose) * get_mat4_from_pose(eye_to_head));

	}
	/// compute lookat matrix for a given camera (0 ... left, 1 ... right)
	cgv::render::render_types::mat4 get_world_to_camera_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, int eye)
	{
		float camera_to_head[12];
		const vr_camera* camera_ptr = vr_kit_ptr->get_camera();
		if (!camera_ptr)
			return cgv::math::identity4<float>();
		camera_ptr->put_camera_to_head_matrix(eye, camera_to_head);
		return inv(get_mat4_from_pose(state.hmd.pose) * get_mat4_from_pose(camera_to_head));
	}
	/// query projection matrix for a given eye (0 ... left, 1 ... right)
	cgv::render::render_types::mat4 get_eye_projection_transform(const vr_kit* vr_kit_ptr, float z_near, float z_far, int eye)
	{
		cgv::render::render_types::mat4 P;
		vr_kit_ptr->put_projection_matrix(eye, z_near, z_far, &P(0, 0));
		return P;
	}
	/// query projection matrix for a given camera (0 ... left or mono, 1 ... right only for stereo cameras)
	cgv::render::render_types::mat4 get_camera_projection_transform(const vr_kit* vr_kit_ptr, float z_near, float z_far, int eye, bool undistorted)
	{
		const vr_camera* camera_ptr = vr_kit_ptr->get_camera();
		if (!camera_ptr)
			return cgv::math::identity4<float>();
		cgv::render::render_types::mat4 TM;
		camera_ptr->put_projection_matrix(eye, undistorted, z_near, z_far, &TM(0, 0));
		return TM;
	}
	/// query the texture matrix needed for projective texture mapping for a given camera (0 ... left or mono, 1 ... right only for stereo cameras)
	cgv::render::render_types::mat4 get_texture_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, float z_near, float z_far, int eye, bool undistorted)
	{
		return get_camera_projection_transform(vr_kit_ptr, z_near, z_far, eye, undistorted) * get_world_to_camera_transform(vr_kit_ptr, state, eye);
	}
}
