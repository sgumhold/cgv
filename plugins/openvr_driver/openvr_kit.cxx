#include "openvr_kit.h"
#include <vr/vr_driver.h>
#include "openvr_camera.h"
#include <iostream>
#include <cgv_gl/gl/gl.h>

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

const vr::IVRSystem* openvr_kit::get_hmd() const
{
	return static_cast<const vr::IVRSystem*>(device_handle);
}

void print_error(vr::IVRSystem* hmd_ptr, vr::ETrackedPropertyError error)
{
	std::cerr << "openvr tracked property error: " << hmd_ptr->GetPropErrorNameFromEnum(error) << std::endl;
}

bool get_bool_property(vr::IVRSystem* hmd_ptr, vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, const char* name = 0)
{
	vr::ETrackedPropertyError error;
	bool value = hmd_ptr->GetBoolTrackedDeviceProperty(unDeviceIndex, prop, &error);
	if (error == vr::TrackedProp_Success) {
		if (name)
			std::cout << name << " = " << (value?"True":"False") << std::endl;
		return value;
	}
	print_error(hmd_ptr, error);
	return false;
}

float get_float_property(vr::IVRSystem* hmd_ptr, vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, const char* name = 0)
{
	vr::ETrackedPropertyError error;
	float value = hmd_ptr->GetFloatTrackedDeviceProperty(unDeviceIndex, prop, &error);
	if (error == vr::TrackedProp_Success) {
		if (name)
			std::cout << name << " = " << value << std::endl;
		return value;
	}
	print_error(hmd_ptr, error);
	return 0.0f;
}

std::string get_string_property(vr::IVRSystem* hmd_ptr, vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, const char* name = 0)
{
	vr::ETrackedPropertyError error;
	char buffer[k_unMaxPropertyStringSize];
	uint32_t len = hmd_ptr->GetStringTrackedDeviceProperty(unDeviceIndex, prop, buffer, k_unMaxPropertyStringSize, &error);
	if (error == vr::TrackedProp_Success) {
		std::string value(buffer, len-1);
		if (name)
			std::cout << name << " = " << value << std::endl;
		return value;
	}
	print_error(hmd_ptr, error);
	return "";
}

void analyze_tracking_reference(vr::IVRSystem* hmd_ptr, vr::TrackedDeviceIndex_t device_index)
{
	bool will_drift_in_yaw = get_bool_property(hmd_ptr, device_index, Prop_WillDriftInYaw_Bool, "WillDriftInYaw");
	bool identifiable = get_bool_property(hmd_ptr, device_index, Prop_Identifiable_Bool, "Identifiable");
	bool has_lighthouse20 = get_bool_property(hmd_ptr, device_index, Prop_ConfigurationIncludesLighthouse20Features_Bool, "IncludesLighthouse20");
	bool can_wireless_identify = get_bool_property(hmd_ptr, device_index, Prop_CanWirelessIdentify_Bool, "CanWirelessIdentify");
	float fov_l_deg = get_float_property(hmd_ptr, device_index, Prop_FieldOfViewLeftDegrees_Float, "FoV_left");
	float fov_r_deg = get_float_property(hmd_ptr, device_index, Prop_FieldOfViewRightDegrees_Float, "FoV_right");
	float fov_t_deg = get_float_property(hmd_ptr, device_index, Prop_FieldOfViewTopDegrees_Float, "FoV_top");
	float fov_b_deg = get_float_property(hmd_ptr, device_index, Prop_FieldOfViewBottomDegrees_Float, "FoV_bottom");
	float d_min_m = get_float_property(hmd_ptr, device_index, Prop_TrackingRangeMinimumMeters_Float, "RangeMinimumMeters");
	float d_max_m = get_float_property(hmd_ptr, device_index, Prop_TrackingRangeMaximumMeters_Float, "RangeMaximumMeters");
	std::string trackingSystemName = get_string_property(hmd_ptr, device_index, Prop_TrackingSystemName_String, "TrackingSystemName");
	std::string registeredDeviceType = get_string_property(hmd_ptr, device_index, Prop_RegisteredDeviceType_String, "RegisteredDeviceType");
	std::string serialNumber = get_string_property(hmd_ptr, device_index, Prop_SerialNumber_String, "SerialNumber");
	std::string manufacturerSerialNumber = get_string_property(hmd_ptr, device_index, Prop_ManufacturerSerialNumber_String, "ManufacturerSerialNumber");
	std::string hardwareRevision = get_string_property(hmd_ptr, device_index, Prop_HardwareRevision_String, "HardwareRevision");
	std::string renderModelName = get_string_property(hmd_ptr, device_index, Prop_RenderModelName_String, "RenderModelName");
	std::string computedSerialNumber = get_string_property(hmd_ptr, device_index, Prop_ComputedSerialNumber_String, "ComputedSerialNumber");
	std::string modelNumber = get_string_property(hmd_ptr, device_index, Prop_ModelNumber_String, "ModelNumber");
	std::string manufacturerName = get_string_property(hmd_ptr, device_index, Prop_ManufacturerName_String, "ManufacturerName");
	std::string modeLabel = get_string_property(hmd_ptr, device_index, Prop_ModeLabel_String, "ModeLabel");
	//Prop_Nonce_Int32
	std::cout << std::endl;
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
	camera = new openvr_camera(_hmd);
}

/// declare virtual destructor
openvr_kit::~openvr_kit()
{
	if (has_camera()) {
		camera->stop();
		destruct_camera();
	}
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
bool openvr_kit::query_state_impl(vr_kit_state& state, int pose_query)
{
	vr::TrackedDeviceIndex_t dis[2] = {
		get_hmd()->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_LeftHand),
		get_hmd()->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_RightHand)
	};
	bool controller_onlys[2] = { (pose_query & 4) != 0, (pose_query & 8) != 0 };
	bool controller_only = controller_onlys[0] || controller_onlys[1];
	pose_query = pose_query & 3;

	vr::ETrackingUniverseOrigin tuo = TrackingUniverseRawAndUncalibrated;
	if (vr::VRCompositor())
		tuo = vr::VRCompositor()->GetTrackingSpace();

	for (int ci = 0; ci < 2; ++ci) {
		if (controller_only && !controller_onlys[1 - ci])
			continue;
		if (dis[ci] == -1)
			state.controller[ci].status = VRS_DETACHED;
		else {
			VRControllerState_t controller_state;
			vr::TrackedDevicePose_t tracked_pose;
			if (pose_query == 0)
				get_hmd()->GetControllerState(dis[ci], &controller_state, sizeof(controller_state));
			else {
				get_hmd()->GetControllerStateWithPose(tuo, dis[ci],
					&controller_state, sizeof(controller_state), &tracked_pose);
				extract_trackable_state(tracked_pose, state.controller[ci]);
			}
			extract_controller_state(controller_state, state.controller[ci]);
		}
	}

	if (pose_query == 0 || (pose_query == 1) && controller_only)
		return true;

	static vr::TrackedDevicePose_t tracked_poses[vr::k_unMaxTrackedDeviceCount];
	if (vr::VRCompositor()) {
		if (pose_query == 2)
			vr::VRCompositor()->WaitGetPoses(tracked_poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);
		if (pose_query == 1)
			vr::VRCompositor()->GetLastPoses(tracked_poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);
		state.hmd.status = vr::VRS_TRACKED;
		//vr::VRCompositor()->SubmitExplicitTimingData();
	}
	else {	
		vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseRawAndUncalibrated, 0.01f, tracked_poses, vr::k_unMaxTrackedDeviceCount);
		state.hmd.status = vr::VRS_DETACHED;
	}
	int next_generic_controller_index = 2;
	state.controller[2].status = vr::VRS_DETACHED;
	state.controller[3].status = vr::VRS_DETACHED;
	clear_reference_states();
	for (int device_index = 0; device_index < vr::k_unMaxTrackedDeviceCount; ++device_index)
	{
		if (tracked_poses[device_index].bPoseIsValid) {
			vr_trackable_state* tracked_pose_ptr = 0;
			ETrackedDeviceClass tdc = get_hmd()->GetTrackedDeviceClass(device_index);
			switch (tdc) {
			case TrackedDeviceClass_HMD:
				tracked_pose_ptr = &state.hmd;
				break;
			case TrackedDeviceClass_GenericTracker :
				if (next_generic_controller_index < 4) {
					tracked_pose_ptr = &state.controller[next_generic_controller_index];
					state.controller[next_generic_controller_index].status = vr::VRS_TRACKED;
					++next_generic_controller_index;
				}
				break;
			case TrackedDeviceClass_TrackingReference :
				//analyze_tracking_reference(get_hmd(), device_index);
				tracked_pose_ptr = &ref_reference_state(
					get_string_property(get_hmd(), device_index, Prop_SerialNumber_String)
				);
				break;
			}
			if (tracked_pose_ptr) {
				put_pose_matrix(tracked_poses[device_index].mDeviceToAbsoluteTracking, tracked_pose_ptr->pose);				
				tracked_pose_ptr->status = tracked_poses[device_index].bPoseIsValid ? VRS_TRACKED : VRS_ATTACHED;
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
void openvr_kit::put_eye_to_head_matrix(int eye, float* pose_matrix) const
{
	put_pose_matrix(const_cast<vr::IVRSystem*>(get_hmd())->GetEyeToHeadTransform(EVREye(eye)), pose_matrix);
}

/// access to 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right) optionally including eye to head transformation
void openvr_kit::put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix, const float*) const
{
	put_hmatrix(const_cast<vr::IVRSystem*>(get_hmd())->GetProjectionMatrix(EVREye(eye), z_near, z_far), projection_matrix);
}

/// submit the rendered stereo frame to the hmd
void openvr_kit::submit_frame()
{
	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)tex_id[0], vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	if (!vr::VRCompositor())
		return;
	EVRCompositorError cel = vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)tex_id[1], vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	EVRCompositorError cer = vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	glFlush();
//	vr::VRCompositor()->PostPresentHandoff();
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
//	else
//		vr::VRCompositor()->SetExplicitTimingMode(VRCompositorTimingMode_Explicit_RuntimePerformsPostPresentHandoff);
//		vr::VRCompositor()->SetExplicitTimingMode(vr::VRCompositorTimingMode_Explicit_ApplicationPerformsPostPresentHandoff);
	return true;
}


}


