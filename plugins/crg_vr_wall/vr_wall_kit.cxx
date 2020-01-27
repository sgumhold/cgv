#include "vr_wall_kit.h"
#include <cgv/math/ftransform.h>
#include <vr_driver.h>
///
namespace vr {
	void vr_wall_kit::detach()
	{
		if (!is_attached())
			return;
		replace_by_pointer(this, parent_kit);
		parent_kit = 0;
	}
	/// change the parent
	bool vr_wall_kit::attach(int vr_kit_parent_index)
	{
		if (vr_kit_parent_index < 0) {
			detach();
			return true;
		}
		if (is_attached())
			detach();
		parent_kit = replace_by_index(vr_kit_parent_index, this);
		camera = parent_kit->get_camera();
		driver = const_cast<vr::vr_driver*>(parent_kit->get_driver());
		device_handle = parent_kit->get_device_handle();
		force_feedback_support = parent_kit->has_force_feedback();
		wireless = parent_kit->is_wireless();
		return true;
	}

	/// initialize render targets and framebuffer objects in current opengl context
	bool vr_wall_kit::init_fbos()
	{
		if (!gl_vr_display::fbos_initialized())
			return gl_vr_display::init_fbos();
		if (!parent_kit->fbos_initialized())
			return parent_kit->init_fbos();
		return false;
	}
	/// check whether fbos have been initialized
	bool vr_wall_kit::fbos_initialized() const
	{
		return gl_vr_display::fbos_initialized() && parent_kit->fbos_initialized();
	}
	/// destruct render targets and framebuffer objects in current opengl context
	void vr_wall_kit::destruct_fbos()
	{
		gl_vr_display::destruct_fbos();
		parent_kit->destruct_fbos();
	}
	/// enable the framebuffer object of given eye (0..left, 1..right) 
	void vr_wall_kit::enable_fbo(int eye)
	{
		parent_kit->enable_fbo(eye);
	}
	/// disable the framebuffer object of given eye
	void vr_wall_kit::disable_fbo(int eye)
	{
		parent_kit->disable_fbo(eye);
	}
	/// initialize render targets and framebuffer objects in current opengl context
	bool vr_wall_kit::blit_fbo(int eye, int x, int y, int w, int h)
	{
		if (secondary_context)
			return gl_vr_display::blit_fbo(eye, x, y, w, h);
		return parent_kit->blit_fbo(eye, x, y, w, h);
	}

	/// construct vr wall kit by attaching to another vr kit
	vr_wall_kit::vr_wall_kit(int vr_kit_parent_index, unsigned _width, unsigned _height, const std::string& _name) :
		gl_vr_display(width, height, 0, 0, _name, false, false)
	{
		secondary_context = false;
		parent_kit = 0;
		if (attach(vr_kit_parent_index))
			camera = parent_kit->get_camera();
		else
			camera = 0;
		screen_x_world = vec3(1.0f, 0.0f, 0.0f);
		screen_y_world = vec3(0.0f, 1.0f, 0.0f);
		screen_z_world = vec3(0.0f, 0.0f, 1.0f);
		screen_center_world.zeros();
		pixel_size = 0.001f;
		width = _width;
		height = _height;
		eye_center_tracker.zeros();
		eye_separation_dir_tracker = vec3(1.0f, 0, 0);
		eye_separation = 0.07f;
		hmd_trackable_index = -1;
	}

	/// use implementation of parent kit
	const std::vector<std::pair<int, int> >& vr_wall_kit::get_controller_throttles_and_sticks(int controller_index) const
	{
		return parent_kit->get_controller_throttles_and_sticks(controller_index); 
	}
	const std::vector<std::pair<float, float> >& vr_wall_kit::get_controller_throttles_and_sticks_deadzone_and_precision(int controller_index) const
	{
		return parent_kit->get_controller_throttles_and_sticks_deadzone_and_precision(controller_index); 
	}
	bool vr_wall_kit::set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength)
	{
		return parent_kit->set_vibration(controller_index, low_frequency_strength, high_frequency_strength);
	}
	/// update state by swapping information of hmd and trackable used to emulate hmd
	bool vr_wall_kit::query_state(vr_kit_state& state, int pose_query) 
	{
		bool res = parent_kit->query_state(state, pose_query);
		if (res) {
			if (hmd_trackable_index != -1)
				std::swap(state.hmd, static_cast<vr_trackable_state&>(state.controller[hmd_trackable_index]));
			if (state.hmd.status == vr::VRS_TRACKED)
				current_tracker_pose = *reinterpret_cast<mat34*>(state.hmd.pose);
		}
		return res;
	}
	/// access to 3x4 matrix in column major format for transformation from eye (0..left, 1..right) to head coordinates
	void vr_wall_kit::put_eye_to_head_matrix(int eye, float* pose_matrix) const 
	{
		mat34& pose = *reinterpret_cast<mat34*>(pose_matrix);
		mat3& orientation = *reinterpret_cast<mat3*>(pose_matrix);
		orientation.identity();
		vec3& position = *reinterpret_cast<vec3*>(pose_matrix + 9);
		position = get_eye_position_world(eye, current_tracker_pose);
	}
	/// access to 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right)
	void vr_wall_kit::put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix) const
	{
		vec3 eye_world = get_eye_position_world(eye, current_tracker_pose);
		vec3 eye_screen = (eye_world - screen_center_world) * get_screen_orientation();
		float scale = z_near/eye_screen(2);
		float width_world = get_width()*pixel_size;
		float l = scale * (-0.5f*width_world - eye_screen(0));
		float r = scale * (+0.5f*width_world - eye_screen(0));
		float height_world = get_height()*pixel_size;
		float b = scale * (-0.5f*height_world - eye_screen(1));
		float t = scale * (+0.5f*height_world - eye_screen(1));
		reinterpret_cast<mat4&>(*projection_matrix) = cgv::math::frustum4<float>(l, r, b, t, z_near, z_far);
	}
	/// compute lookat matrix for a given eye (0 ... left, 1 ... right)
	void vr_wall_kit::put_world_to_eye_transform(int eye, float* modelview_matrix) const
	{
		vec3 eye_world = get_eye_position_world(eye, current_tracker_pose);
		mat3 R = reinterpret_cast<const mat3&>(get_screen_pose());
		R.transpose();
		mat4& T = reinterpret_cast<mat4&>(*modelview_matrix);
		T.set_col(0, vec4(R.col(0), 0));
		T.set_col(1, vec4(R.col(1), 0));
		T.set_col(2, vec4(R.col(2), 0));
		T.set_col(3, vec4(-R * eye_world, 1));
	}
	/// submit the rendered stereo frame to the hmd
	void vr_wall_kit::submit_frame()
	{
		on_submit_frame(this);
	}
}
