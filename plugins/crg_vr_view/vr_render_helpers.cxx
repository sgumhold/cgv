#include <cgv/base/base.h>
#include "vr_render_helpers.h"
#include <cgv/math/ftransform.h>
#include <cgv/base/import.h>

///
namespace vr {
	bool& ref_vrmesh_outofdate(VRMeshId id)
	{
		static std::vector<int> outofdates;
		if ((int)outofdates.size() <= id)
			outofdates.resize(id + 1, true);
		return reinterpret_cast<bool&>(outofdates[id]);
	}
	std::string& ref_vrmesh_file_name(VRMeshId id)
	{
		static std::vector<std::string> file_names;
		if ((int)file_names.size() <= id)
			file_names.resize(id + 1);
		return file_names[id];
	}
	const std::string& get_vrmesh_file_name(VRMeshId id)
	{
		return ref_vrmesh_file_name(id);
	}
	void set_vrmesh_file_name(VRMeshId id, const std::string& file_name)
	{
		ref_vrmesh_file_name(id) = file_name;
		ref_vrmesh_outofdate(id) = true;
	}
	cgv::render::mesh_render_info* get_vrmesh_render_info(cgv::render::context& ctx, VRMeshId id)
	{
		static std::vector<cgv::render::mesh_render_info*> mesh_infos;
		if ((int)mesh_infos.size() <= id)
			mesh_infos.resize(id + 1, 0);
		// in case file name changed, delete previous mesh render info
		if (ref_vrmesh_outofdate(id) && !get_vrmesh_file_name(id).empty() && mesh_infos[id] != 0) {
			mesh_infos[id]->destruct(ctx);
			delete mesh_infos[id];
			mesh_infos[id] = 0;
		}
		// check whether mesh needs to be read from file
		if (mesh_infos[id] == 0 && !get_vrmesh_file_name(id).empty()) {
			cgv::media::mesh::simple_mesh<float> M;
			if (M.read(cgv::base::find_data_file(get_vrmesh_file_name(id), "Dc"))) {
				mesh_infos[id] = new cgv::render::mesh_render_info();
				mesh_infos[id]->construct(ctx, M);
				mesh_infos[id]->bind(ctx, ctx.ref_surface_shader_program(true), true);
			}
			ref_vrmesh_outofdate(id) = false;
		}
		return mesh_infos[id];
	}	

	/// convert pose to mat4
	cgv::mat4 get_mat4_from_pose(const float pose_matrix[12])
	{
		cgv::mat4 M;

		M.set_col(0, cgv::vec4(reinterpret_cast<const cgv::vec3&>(pose_matrix[0]), 0));
		M.set_col(1, cgv::vec4(reinterpret_cast<const cgv::vec3&>(pose_matrix[3]), 0));
		M.set_col(2, cgv::vec4(reinterpret_cast<const cgv::vec3&>(pose_matrix[6]), 0));
		M.set_col(3, cgv::vec4(reinterpret_cast<const cgv::vec3&>(pose_matrix[9]), 1));
		return M;
	}
	/// compute lookat matrix for a given eye (0 ... left, 1 ... right)
	cgv::mat4 get_world_to_eye_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, int eye)
	{
		cgv::mat4 T;
		vr_kit_ptr->put_world_to_eye_transform(eye, state.hmd.pose, T);
		return T;		
		/*
		float eye_to_head[12];
		vr_kit_ptr->put_eye_to_head_matrix(eye, eye_to_head);
		return inv(get_mat4_from_pose(state.hmd.pose) * get_mat4_from_pose(eye_to_head));		
		*/
	}
	/// compute lookat matrix for a given camera (0 ... left, 1 ... right)
	cgv::mat4 get_world_to_camera_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, int eye)
	{
		float camera_to_head[12];
		const vr_camera* camera_ptr = vr_kit_ptr->get_camera();
		if (!camera_ptr)
			return cgv::math::identity4<float>();
		camera_ptr->put_camera_to_head_matrix(eye, camera_to_head);
		return inv(get_mat4_from_pose(state.hmd.pose) * get_mat4_from_pose(camera_to_head));
	}
	/// query projection matrix for a given eye (0 ... left, 1 ... right)
	cgv::mat4 get_eye_projection_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, float z_near, float z_far, int eye)
	{
		cgv::mat4 P;
		vr_kit_ptr->put_projection_matrix(eye, z_near, z_far, &P(0, 0), state.hmd.pose);
		return P;
	}
	/// query projection matrix for a given camera (0 ... left or mono, 1 ... right only for stereo cameras)
	cgv::mat4 get_camera_projection_transform(const vr_kit* vr_kit_ptr, float z_near, float z_far, int eye, bool undistorted)
	{
		const vr_camera* camera_ptr = vr_kit_ptr->get_camera();
		if (!camera_ptr)
			return cgv::math::identity4<float>();
		cgv::mat4 TM;
		camera_ptr->put_projection_matrix(eye, undistorted, z_near, z_far, &TM(0, 0));
		return TM;
	}
	/// query the texture matrix needed for projective texture mapping for a given camera (0 ... left or mono, 1 ... right only for stereo cameras)
	cgv::mat4 get_texture_transform(const vr_kit* vr_kit_ptr, const vr_kit_state& state, float z_near, float z_far, int eye, bool undistorted)
	{
		return get_camera_projection_transform(vr_kit_ptr, z_near, z_far, eye, undistorted) * get_world_to_camera_transform(vr_kit_ptr, state, eye);
	}
	bool configure_seethrough_shader_program(cgv::render::context& ctx, cgv::render::shader_program& prog, uint32_t frame_width, uint32_t frame_height, const vr_kit* vr_kit_ptr, const vr_kit_state& state, float z_near, float z_far, int eye, bool undistorted)
	{
		vr::vr_camera* camera_ptr = vr_kit_ptr->get_camera();
		if (!camera_ptr)
			return false;
		
		int nr_cameras = camera_ptr->get_nr_cameras();
		int frame_split = camera_ptr->get_frame_split();
		cgv::vec2 focal_lengths;
		cgv::vec2 camera_center;
		camera_ptr->put_camera_intrinsics(eye, true, &focal_lengths(0), &camera_center(0));
		camera_center(0) /= frame_width;
		camera_center(1) /= frame_height;
		cgv::vec2 extent_texcrd(0.5f);

		cgv::mat4 TM = vr::get_texture_transform(vr_kit_ptr, state, z_near, z_far, eye, undistorted);

		return
			prog.set_uniform(ctx, "texture_matrix", TM) &&
			prog.set_uniform(ctx, "extent_texcrd", extent_texcrd) &&
			prog.set_uniform(ctx, "frame_split", frame_split) &&
			prog.set_uniform(ctx, "center_left", camera_center) &&
			prog.set_uniform(ctx, "center_right", camera_center) &&
			prog.set_uniform(ctx, "eye", eye);
	}
}
