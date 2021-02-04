#pragma once

#include <vr/vr_kit.h>
#include "openvr.h"

#include <vector>

#include "lib_begin.h"

namespace vr {
	/// helper function to convert 3x4 matrix
	extern CGV_API void put_pose_matrix(const vr::HmdMatrix34_t& P, float* pose_matrix);
	/// helper function to convert 3x4 matrix
	extern CGV_API void put_hmatrix(const vr::HmdMatrix44_t& P, float* hmatrix);
	/**@name vr device management */
	//@{
	/// information provided per vr device
	class CGV_API openvr_kit : public vr_kit
	{
	protected:
		vr::IVRSystem* get_hmd();
		const vr::IVRSystem* get_hmd() const;
		/// update information common to all trackable devices
		void update_trackable_info(vr_trackable_info& TI, vr::TrackedDeviceIndex_t device_index, bool only_dynamic);
		/// update hmd info
		void update_hmd_info();
		/// update controller info
		void update_controller_info(int controller_index, vr::TrackedDeviceIndex_t tracked_device_index);
		/// update tracker info
		void update_tracker_info(int controller_index, vr::TrackedDeviceIndex_t tracked_device_index);
		/// update tracker info
		void update_tracking_reference_info(const std::string& serial, vr::TrackedDeviceIndex_t tracked_device_index);
		/// extract the controller state
		void extract_controller_state(const VRControllerState_t& input, int ci, vr_controller_state& output);
		/// query current state of vr kit and return whether this was successful
		bool query_state_impl(vr_kit_state& state, int pose_query);
	public:
		/// construct
		openvr_kit(unsigned _width, unsigned _height, vr_driver* _driver, vr::IVRSystem* _hmd, const std::string& _name);
		/// declare virtual destructor
		~openvr_kit();
		/// set the vibration strength between 0 and 1 of low and high frequency motors, return false if device is not connected anymore
		bool set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength);
		/// access to 3x4 matrix in column major format for transformation from eye (0..left, 1..right) to head coordinates
		void put_eye_to_head_matrix(int eye, float* pose_matrix) const;
		/// access to 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right) 
		void put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix, const float* hmd_pose) const;
		/// initialize render targets and framebuffer objects in current opengl context
		bool init_fbos();
		/// submit the rendered stereo frame to the hmd
		void submit_frame();
	};
}

#include <cgv/config/lib_end.h>
