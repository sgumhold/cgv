#include "openvr_kit.h"
#include <iostream>

namespace vr {

void put_pose_matrix(const vr::HmdMatrix34_t& P, float* pose_matrix)
{
	pose_matrix[0] = P.m[0][0];
	pose_matrix[1] = P.m[1][0];
	pose_matrix[2] = P.m[2][0];
	pose_matrix[3] = P.m[0][1];
	pose_matrix[4] = P.m[1][1];
	pose_matrix[5] = P.m[2][1];
	pose_matrix[6] = P.m[0][2];
	pose_matrix[7] = P.m[1][2];
	pose_matrix[8] = P.m[2][2];
	pose_matrix[9] = P.m[0][3];
	pose_matrix[10] = P.m[1][3];
	pose_matrix[11] = P.m[2][3];
}

void put_hmatrix(const vr::HmdMatrix44_t& P, float* hmatrix)
{
	hmatrix[0] = P.m[0][0];
	hmatrix[1] = P.m[1][0];
	hmatrix[2] = P.m[2][0];
	hmatrix[3] = P.m[3][0];
	hmatrix[4] = P.m[0][1];
	hmatrix[5] = P.m[1][1];
	hmatrix[6] = P.m[2][1];
	hmatrix[7] = P.m[3][1];
	hmatrix[8] = P.m[0][2];
	hmatrix[9] = P.m[1][2];
	hmatrix[10] = P.m[2][2];
	hmatrix[11] = P.m[3][2];
	hmatrix[12] = P.m[0][3];
	hmatrix[13] = P.m[1][3];
	hmatrix[14] = P.m[2][3];
	hmatrix[15] = P.m[3][3];
}

vr::IVRSystem* openvr_kit::get_hmd()
{
	return static_cast<vr::IVRSystem*>(device_handle);
}


const std::vector<std::pair<int, int> >& openvr_kit::get_controller_throttles_and_sticks(int controller_index) const
{
	static std::vector<std::pair<int, int> > throttles_and_sticks;
	if (throttles_and_sticks.empty()) {
		// add stick
		throttles_and_sticks.push_back(std::pair<int, int>(0, 1));
		// add trigger throttle
		throttles_and_sticks.push_back(std::pair<int, int>(2, -1));
	}
	return throttles_and_sticks;
}

/// for each controller provide information on throttles' and sticks' deadzone and precision values
const std::vector<std::pair<float, float> >& openvr_kit::get_controller_throttles_and_sticks_deadzone_and_precision(int controller_index) const
{
	static std::vector<std::pair<float, float> > deadzone_and_precision;
	if (deadzone_and_precision.empty()) {
		deadzone_and_precision.push_back(std::pair<float, float>(0.0f, 0.0f));
		deadzone_and_precision.push_back(std::pair<float, float>(0.0f, 0.0f));
	}
	return deadzone_and_precision;
}

/// construct
openvr_kit::openvr_kit(unsigned _width, unsigned _height, 
	vr_driver* _driver, vr::IVRSystem* _hmd,
	const std::string& _name, bool _ffb_support, bool _wireless)
	: gl_vr_display(_width, _height, _driver, _hmd, _name, _ffb_support, _wireless)
{
}

/// declare virtual destructor
openvr_kit::~openvr_kit()
{

}

void extract_controller_state(const VRControllerState_t& input, vr_controller_state& output)
{
	output.time_stamp = input.unPacketNum;
	output.button_flags = 0;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_ApplicationMenu)) != 0)
		output.button_flags += VRF_MENU;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_Grip)) != 0)
		output.button_flags += VRF_BUTTON0;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_Axis0)) != 0)
		output.button_flags += VRF_STICK;
	if ((input.ulButtonTouched & ButtonMaskFromId(k_EButton_Axis0)) != 0)
		output.button_flags += VRF_STICK_TOUCH;

	for (unsigned i = 0; i < 4; ++i) {
		output.axes[2 * i] = input.rAxis[i].x;
		output.axes[2 * i + 1] = input.rAxis[i].y;
	}
}

void extract_trackable_state(const vr::TrackedDevicePose_t& tracked_pose, vr_trackable_state& trackable)
{
	if (!tracked_pose.bDeviceIsConnected)
		trackable.status = VRS_DETACHED;
	else {
		if (!tracked_pose.bPoseIsValid)
			trackable.status = VRS_ATTACHED;
		else {
			trackable.status = VRS_TRACKED;
			put_pose_matrix(tracked_pose.mDeviceToAbsoluteTracking, trackable.pose);
		}
	}
}
/// retrieve the current state of vr kit and optionally wait for poses optimal for rendering, return false if vr_kit is not connected anymore
bool openvr_kit::query_state(vr_kit_state& state, int pose_query)
{
	vr::TrackedDeviceIndex_t left_index = get_hmd()->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_LeftHand);
	vr::TrackedDeviceIndex_t right_index = get_hmd()->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_RightHand);
//	if (pose_query == 1) {
		VRControllerState_t controller_state;
		vr::TrackedDevicePose_t tracked_pose;
		get_hmd()->GetControllerStateWithPose(TrackingUniverseRawAndUncalibrated, left_index, &controller_state, sizeof(controller_state), &tracked_pose);
		//std::cout << controller_state.rAxis[0].x << "," << controller_state.rAxis[0].y << "  " << controller_state.ulButtonPressed << ";" << controller_state.ulButtonTouched << " <" << controller_state.unPacketNum << ">" << std::endl;
		//std::cout << controller_state.ulButtonPressed << ";" << controller_state.ulButtonTouched << "   <->    ";
		extract_controller_state(controller_state, state.controller[0]);
		//if (pose_query == 1)
			extract_trackable_state(tracked_pose, state.controller[0]);

		get_hmd()->GetControllerStateWithPose(TrackingUniverseRawAndUncalibrated, right_index, &controller_state, sizeof(controller_state), &tracked_pose);
		//std::cout << controller_state.ulButtonPressed << ";" << controller_state.ulButtonTouched << std::endl;
		extract_controller_state(controller_state, state.controller[1]);
//		if (pose_query == 1)
			extract_trackable_state(tracked_pose, state.controller[1]);
		//return true;
	/*	}

	VRControllerState_t controller_state;
	if (!get_hmd()->IsInputAvailable()) {
		std::cerr << "no input" << std::endl;
	}
	get_hmd()->GetControllerState( left_index, &controller_state, sizeof(VRControllerState_t));
	extract_controller_state(controller_state, state.controller[0]);

	get_hmd()->GetControllerState(right_index, &controller_state, sizeof(VRControllerState_t));
	extract_controller_state(controller_state, state.controller[1]);
	*/
	if (pose_query != 2) 
		return true;

	static vr::TrackedDevicePose_t tracked_poses[vr::k_unMaxTrackedDeviceCount];
	vr::VRCompositor()->WaitGetPoses(tracked_poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);
	for (int device_index = 0; device_index < vr::k_unMaxTrackedDeviceCount; ++device_index)
	{
		if (tracked_poses[device_index].bPoseIsValid) {
			vr_trackable_state* tracked_pose_ptr = 0;
			switch (get_hmd()->GetTrackedDeviceClass(device_index)) {
			case TrackedDeviceClass_Controller:
				switch (get_hmd()->GetControllerRoleForTrackedDeviceIndex(device_index)) {
				case TrackedControllerRole_LeftHand:
					tracked_pose_ptr = &state.controller[0];
					break;
				case TrackedControllerRole_RightHand:
					tracked_pose_ptr = &state.controller[1];
					break;
				}
				break;
			case TrackedDeviceClass_HMD:
				tracked_pose_ptr = &state.hmd;
				break;
			}
			if (tracked_pose_ptr) {
				put_pose_matrix(tracked_poses[device_index].mDeviceToAbsoluteTracking, tracked_pose_ptr->pose);
			}
		}
	}
	return true;
}

/*
bool openvr_kit::query_key_event(VRKeys& key, KeyAction& action)
{
	// Process SteamVR events
	vr::VREvent_t event;
	if (get_hmd()->PollNextEvent(&event, sizeof(event))) {
		ProcessVREvent(event);
	}
}
*/

/// set the vibration strength between 0 and 1 of low and high frequency motors, return false if device is not connected anymore
bool openvr_kit::set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength)
{
	// some hack
	get_hmd()->TriggerHapticPulse(
		get_hmd()->GetTrackedDeviceIndexForControllerRole(controller_index == 0 ? TrackedControllerRole_LeftHand : TrackedControllerRole_RightHand),
		0, 5);
	return true;
}


/// access to 3x4 matrix in column major format for transformation from eye (0..left, 1..right) to head coordinates
void openvr_kit::put_eye_to_head_matrix(int eye, float* pose_matrix)
{
	put_pose_matrix(get_hmd()->GetEyeToHeadTransform(EVREye(eye)), pose_matrix);
}

/// access to 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right) optionally including eye to head transformation
void openvr_kit::put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix)
{
	put_hmatrix(get_hmd()->GetProjectionMatrix(EVREye(eye), z_near, z_far), projection_matrix);
}

/// submit the rendered stereo frame to the hmd
void openvr_kit::submit_frame()
{
	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)tex_id[0], vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)tex_id[1], vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
}

/// initialize render targets and framebuffer objects in current opengl context
bool openvr_kit::init_fbos()
{
	if (!gl_vr_display::init_fbos())
		return false;

	if (!vr::VRCompositor()) {
		last_error = "Compositor initialization failed. See log file for details";
		return false;
	}

	return true;
}


}


