#pragma once

#include <vr/vr_kit.h>
#include <vr/vr_state.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/mesh_render_info.h>

#include "lib_begin.h"

///@ingroup VR
///@{

//! provides functions bridging the basic vr support with the cgv framework
/*! the helper functions allow to compute the modelview, projection and texture matrices needed for rendering. */

///
namespace vr {
	/// convert pose to mat4
	extern CGV_API cgv::mat4 get_mat4_from_pose(const float pose_matrix[12]);
	/// compute lookat matrix for a given eye (0 ... left, 1 ... right)
	extern CGV_API cgv::mat4 get_world_to_eye_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, int eye);
	/// compute lookat matrix for a given camera (0 ... left, 1 ... right)
	extern CGV_API cgv::mat4 get_world_to_camera_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, int eye);
	/// query projection matrix for a given eye (0 ... left, 1 ... right)
	extern CGV_API cgv::mat4 get_eye_projection_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, float z_near, float z_far, int eye);
	/// query projection matrix for a given camera (0 ... left or mono, 1 ... right only for stereo cameras)
	extern CGV_API cgv::mat4 get_camera_projection_transform(const vr_kit* vr_kit_ptr, float z_near, float z_far, int eye, bool undistorted);
	/// query the texture matrix needed for projective texture mapping for a given camera (0 ... left or mono, 1 ... right only for stereo cameras)
	extern CGV_API cgv::mat4 get_texture_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, float z_near, float z_far, int eye, bool undistorted);
	//! set all uniforms of seethrough shader program for a given camera (0 ... left or mono, 1 ... right only for stereo cameras). 
	/*! For this the get_texture_transform is used to compute the texture matrix.
	    Return whether setting the uniforms was successful.*/
	extern CGV_API bool configure_seethrough_shader_program(cgv::render::context& ctx, cgv::render::shader_program& prog, 
		uint32_t frame_width, uint32_t frame_height, const vr_kit* vr_kit_ptr, const vr_kit_state& state, float z_near, float z_far, int eye, bool undistorted);
	/// enumerate for different mesh types
	enum VRMeshId {
		VRM_HMD,
		VRM_CONTROLLER,
		VRM_TRACKER,
		VRM_BASE
	};
	/// return the file name for the given vrmesh type
	extern CGV_API const std::string& get_vrmesh_file_name(VRMeshId id);
	/// set the file name for the given vrmesh type
	extern CGV_API void set_vrmesh_file_name(VRMeshId id, const std::string& file_name);
	/// return a pointer to a mesh info structure for the given mesh type (read and construct if necessary); returns 0 pointer if mesh is not available
	extern CGV_API cgv::render::mesh_render_info* get_vrmesh_render_info(cgv::render::context& ctx, VRMeshId id);
}
///@}

#include <cgv/config/lib_end.h>
