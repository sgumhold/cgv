#include "vr_kit.h"
#include "vr_driver.h"
#include <cgv/math/fmat.h>

namespace vr {
	/// initialize to defaults (dead_zone=precision=0;threshold=0.5)
	controller_input_config::controller_input_config()
	{
		dead_zone = precision = 0.0f;
		threshold = 0.5f;
	}
	/// 
	vr_trackable_state& vr_kit::ref_tracking_reference_state(const std::string& serial_nummer) 
	{
		return driver->ref_tracking_reference_state(serial_nummer); 
	}
	/// write access to tracking system info
	vr_tracking_system_info& vr_kit::ref_tracking_system_info()
	{
		return driver->tracking_system_info;
	}
	/// remove all reference states
	void vr_kit::clear_tracking_reference_states() 
	{
		driver->clear_tracking_reference_states(); 
	}
	/// mark all reference states as untracked
	void vr_kit::mark_tracking_references_as_untracked() 
	{
		driver->mark_tracking_references_as_untracked(); 
	}
	/// destruct camera
	void vr_kit::destruct_camera()
	{
		delete camera;
		camera = nullptr;
	}
	/// construct
	vr_kit::vr_kit(vr_driver* _driver, void* _handle, const std::string& _name, unsigned _width, unsigned _height, unsigned _nr_multi_samples) :
		gl_vr_display(_width, _height, _nr_multi_samples), driver(_driver), handle(_handle), name(_name), camera(nullptr), skip_calibration(false) {}
	/// declare virtual destructor
	vr_kit::~vr_kit()
	{
		if (camera)
			camera->stop();
	}
	/// return driver
	const vr_driver* vr_kit::get_driver() const { return driver; }
	/// return device handle
	void* vr_kit::get_handle() const { return handle; }
	/// return camera
	vr_camera * vr_kit::get_camera() const { return camera; }
	/// return name of vr_kit
	const std::string& vr_kit::get_name() const { return name; }
	
	float dot(int n, const float* a, const float* b, int step_a = 1, int step_b=1)  
	{
		float res = 0;
		for (int i = 0; i < n; ++i)
			res += a[i*step_a] * b[i*step_b];
		return res; 
	}
	void homogenize_matrix(float* A, int n, const float* B = 0)
	{
		if (B == 0)
			B = A;
		for (int j = n; j >= 0; --j) {
			A[j*(n + 1) + n] = (j == n ? 1.0f : 0.0f);
			for (int i = n - 1; i >= 0; --i)
				A[j*(n + 1) + i] = B[j*n + i];
		}
	}
	void invert_rigid_body_transformation(float* T)
	{
		int i;
		float t[3];
		for (i = 0; i < 3; ++i)
			t[i] = -dot(3, T + 4 * i, T + 12);
		for (i = 0; i < 3; ++i) {
			T[12 + i] = t[i];
			for (int j = i + 1; j < 3; ++j)
				std::swap(T[4 * j + i], T[4 * i + j]);
		}
	}
	void matrix_multiplication(const float* A, const float* B, float* C)
	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				C[4 * j + i] = dot(4, A + i, B + 4 * j, 4);
	}
	/// access to 4x4 modelview transformation matrix of given eye in column major format, which is computed in default implementation from given 3x4 pose matrix and eye to head transformation
	void vr_kit::put_world_to_eye_transform(int eye, const float* hmd_pose, float* modelview_matrix) const
	{
		float eye_to_head[16];
		put_eye_to_head_matrix(eye, eye_to_head);
		homogenize_matrix(eye_to_head, 3);
		invert_rigid_body_transformation(eye_to_head);
		float head_to_world[16];
		homogenize_matrix(head_to_world, 3, hmd_pose);
		invert_rigid_body_transformation(head_to_world);
		matrix_multiplication(eye_to_head, head_to_world, modelview_matrix);
	}
	bool vr_kit::query_state(vr_kit_state& state, int pose_query)
	{
		bool res = query_state_impl(state, pose_query);
		const vr_driver* dp = get_driver();
		if (pose_query == 0 || skip_calibration || !dp->is_calibration_transformation_enabled())
			return res;
		if (state.hmd.status == VRS_TRACKED)
			dp->calibrate_pose(state.hmd.pose);
		for (int i = 0; i < max_nr_controllers; ++i)
			if (state.controller[i].status == VRS_TRACKED)
				dp->calibrate_pose(state.controller[i].pose);
		return res;
	}
	/// return information on the currently attached devices
	const vr_kit_info& vr_kit::get_device_info() const
	{
		return info;
	}
	void vr_kit::set_controller_input_config(int ci, int ii, const controller_input_config& cic)
	{
		input_configs[ci][ii] = cic;
	}
	/// query the configuration of a controller input 
	const controller_input_config& vr_kit::get_controller_input_config(int ci, int ii) const
	{
		return input_configs[ci][ii];
	}

}


