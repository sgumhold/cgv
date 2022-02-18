#define _USE_MATH_DEFINES
#include <math.h>
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
	return static_cast<vr::IVRSystem*>(handle);
}

const vr::IVRSystem* openvr_kit::get_hmd() const
{
	return static_cast<const vr::IVRSystem*>(handle);
}

void print_error(vr::IVRSystem* hmd_ptr, vr::ETrackedPropertyError error)
{
	std::cerr << "openvr tracked property error: " << hmd_ptr->GetPropErrorNameFromEnum(error) << std::endl;
}

bool get_bool_property(vr::IVRSystem* hmd_ptr, vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, bool show_error = true, const char* name = 0)
{
	vr::ETrackedPropertyError error;
	bool value = hmd_ptr->GetBoolTrackedDeviceProperty(unDeviceIndex, prop, &error);
	if (error == vr::TrackedProp_Success) {
		if (name)
			std::cout << name << " = " << (value?"True":"False") << std::endl;
		return value;
	}
	if (show_error)
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
		std::string value(buffer, len - 1);
		if (name)
			std::cout << name << " = " << value << std::endl;
		return value;
	}
	print_error(hmd_ptr, error);
	return "";
}

int32_t get_int32_property(vr::IVRSystem* hmd_ptr, vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, bool show_error = true, const char* name = 0)
{
	vr::ETrackedPropertyError error;
	int32_t value = hmd_ptr->GetInt32TrackedDeviceProperty(unDeviceIndex, prop, &error);
	if (error == vr::TrackedProp_Success) {
		if (name)
			std::cout << name << " = " << value << std::endl;
		return value;
	}
	if (show_error)
		print_error(hmd_ptr, error);
	return 0;
}

uint64_t get_uint64_property(vr::IVRSystem* hmd_ptr, vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, const char* name = 0)
{
	vr::ETrackedPropertyError error;
	uint64_t value = hmd_ptr->GetUint64TrackedDeviceProperty(unDeviceIndex, prop, &error);
	if (error == vr::TrackedProp_Success) {
		if (name)
			std::cout << name << " = " << value << std::endl;
		return value;
	}
	print_error(hmd_ptr, error);
	return 0;
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

/// construct
openvr_kit::openvr_kit(unsigned _width, unsigned _height, vr_driver* _driver, vr::IVRSystem* _hmd, const std::string& _name)
	: vr_kit(_driver, _hmd, _name, _width, _height)
{
	camera = new openvr_camera(_hmd);
	info.force_feedback_support = true;
}

/// declare virtual destructor
openvr_kit::~openvr_kit()
{
	if (camera) {
		camera->stop();
		destruct_camera();
	}
}

void openvr_kit::extract_controller_state(const VRControllerState_t& input, int ci, vr_controller_state& output)
{
	output.time_stamp = input.unPacketNum;
	output.button_flags = 0;

	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_System)) != 0)
		output.button_flags += VRF_SYSTEM;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_ApplicationMenu)) != 0)
		output.button_flags += VRF_MENU;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_Grip)) != 0)
		output.button_flags += VRF_GRIP;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_DPad_Left)) != 0)
		output.button_flags += VRF_DPAD_LEFT;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_DPad_Right)) != 0)
		output.button_flags += VRF_DPAD_RIGHT;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_DPad_Down)) != 0)
		output.button_flags += VRF_DPAD_DOWN;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_DPad_Up)) != 0)
		output.button_flags += VRF_DPAD_UP;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_A)) != 0)
		output.button_flags += VRF_A;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_Axis0)) != 0)
		output.button_flags += VRF_INPUT0;
	if ((input.ulButtonTouched & ButtonMaskFromId(k_EButton_Axis0)) != 0)
		output.button_flags += VRF_INPUT0_TOUCH;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_Axis1)) != 0)
		output.button_flags += VRF_INPUT1;
	if ((input.ulButtonTouched & ButtonMaskFromId(k_EButton_Axis1)) != 0)
		output.button_flags += VRF_INPUT1_TOUCH;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_Axis2)) != 0)
		output.button_flags += VRF_INPUT2;
	if ((input.ulButtonTouched & ButtonMaskFromId(k_EButton_Axis2)) != 0)
		output.button_flags += VRF_INPUT2_TOUCH;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_Axis3)) != 0)
		output.button_flags += VRF_INPUT3;
	if ((input.ulButtonTouched & ButtonMaskFromId(k_EButton_Axis3)) != 0)
		output.button_flags += VRF_INPUT3_TOUCH;
	if ((input.ulButtonPressed & ButtonMaskFromId(k_EButton_Axis4)) != 0)
		output.button_flags += VRF_INPUT4;
	if ((input.ulButtonTouched & ButtonMaskFromId(k_EButton_Axis4)) != 0)
		output.button_flags += VRF_INPUT4_TOUCH;
	if ((input.ulButtonTouched & ButtonMaskFromId(k_EButton_ProximitySensor)) != 0)
		output.button_flags += VRF_PROXIMITY;
	
	const auto& CI = get_device_info().controller[ci];
	int ai = 0;
	for (int ii = 0; ii < CI.nr_inputs; ++ii) {
		if (CI.input_type[ii] == VRI_NONE)
			continue;
		output.axes[ai++] = input.rAxis[ii].x;
		if (CI.input_type[ii] == VRI_TRIGGER)
			continue;
		output.axes[ai++] = input.rAxis[ii].y;
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

void openvr_kit::update_trackable_info(vr_trackable_info& TI, vr::TrackedDeviceIndex_t device_index, bool only_dynamic)
{
	if (!only_dynamic) {
		TI.model_number = get_string_property(get_hmd(), device_index, Prop_ModelNumber_String);
		TI.device_type = get_string_property(get_hmd(), device_index, Prop_RegisteredDeviceType_String);
		TI.device_class = get_int32_property(get_hmd(), device_index, Prop_DeviceClass_Int32);
		TI.is_wireless = get_bool_property(get_hmd(), device_index, Prop_DeviceIsWireless_Bool, false);
		TI.provides_battery_charge_level = get_bool_property(get_hmd(), device_index, Prop_DeviceProvidesBatteryStatus_Bool, false);
	}
	if (TI.is_wireless && TI.provides_battery_charge_level)
		TI.battery_charge_level = get_float_property(get_hmd(), device_index, Prop_DeviceBatteryPercentage_Float);
}

void openvr_kit::update_hmd_info()
{
	vr::TrackedDeviceIndex_t device_index = 0;
	vr_hmd_info& HI = info.hmd;
	std::string serial_number = get_string_property(get_hmd(), device_index, Prop_SerialNumber_String);
	if (HI.serial_number == serial_number) {
		update_trackable_info(info.hmd, device_index, true);
		return;
	}
	update_trackable_info(HI, device_index, false);
	HI.serial_number = serial_number;
	HI.reports_time_since_vsynch = get_bool_property(get_hmd(), device_index, Prop_ReportsTimeSinceVSync_Bool);
	HI.seconds_vsynch_to_photons = get_float_property(get_hmd(), device_index, Prop_SecondsFromVsyncToPhotons_Float);
	HI.fps = get_float_property(get_hmd(), device_index, Prop_DisplayFrequency_Float);
	HI.ipd = get_float_property(get_hmd(), device_index, Prop_UserIpdMeters_Float);
	HI.head_to_eye_distance = get_float_property(get_hmd(), device_index, Prop_UserHeadToEyeDepthMeters_Float);
	bool has_camera = get_bool_property(get_hmd(), device_index, Prop_HasCamera_Bool, false);
	/*
	HI.camera_to_head_transform[2][12];
	HI.imu_to_head_transform[12];
	HI.imu_gyro_bias[3];
	HI.imu_gyro_scale[3];
	HI.imu_accelerometer_bias[3];
	HI.imu_accelerometer_scale[3];
	*/
	HI.lighthouse_2_0_features = get_bool_property(get_hmd(), device_index, Prop_ConfigurationIncludesLighthouse20Features_Bool);
	HI.has_proximity_sensor = get_bool_property(get_hmd(), device_index, Prop_ContainsProximitySensor_Bool);
	if (get_bool_property(get_hmd(), device_index, Prop_HasCamera_Bool))
		HI.number_cameras = get_int32_property(get_hmd(), device_index, Prop_NumCameras_Int32);
	else
		HI.number_cameras = 0;
}

void openvr_kit::update_controller_info(int ci, vr::TrackedDeviceIndex_t device_index)
{
	std::string serial_number = get_string_property(get_hmd(), device_index, Prop_SerialNumber_String);
	auto& CI = info.controller[ci];
	if (CI.serial_number == serial_number) {
		update_trackable_info(CI, device_index, true);
		return;
	}	
	update_trackable_info(CI, device_index, false);
	CI.type = VRC_CONTROLLER;
	CI.role = VRC_NOT_ASSIGNED;
	switch (get_hmd()->GetControllerRoleForTrackedDeviceIndex(device_index)) {
	case TrackedControllerRole_Invalid :
	case TrackedControllerRole_LeftHand :
		CI.role = VRC_LEFT_HAND;
		break;
	case TrackedControllerRole_RightHand :
		CI.role = VRC_RIGHT_HAND;
		break;
	case TrackedControllerRole_OptOut :
		break;
	case TrackedControllerRole_Treadmill :
		CI.role = VRC_TREADMILL;
		break;
	case TrackedControllerRole_Stylus :
		CI.role = VRC_STYLUS;
		break;
	}
	CI.serial_number = serial_number;
//	std::string attached_device_id = get_string_property(get_hmd(), device_index, Prop_AttachedDeviceId_String);
//	if (!attached_device_id.empty())
//		CI.variable_parameters["attached_device_id"] = attached_device_id;
	std::fill(CI.input_type, CI.input_type + vr::max_nr_controller_inputs, VRI_NONE);
	std::fill(CI.axis_type, CI.axis_type + vr::max_nr_controller_axes, VRA_NONE);
	int ai = 0;
	CI.nr_inputs = 0;
	for (int ii = 0; ii < vr::max_nr_controller_inputs; ++ii)
		switch (get_int32_property(get_hmd(), device_index, ETrackedDeviceProperty(Prop_Axis0Type_Int32 + ii), false)) {
		case k_eControllerAxis_None: 
			break;
		case k_eControllerAxis_TrackPad:
			CI.input_type[ii] = VRI_PAD;
			CI.axis_type[ai++] = VRA_PAD_X;
			CI.axis_type[ai++] = VRA_PAD_Y;
			++CI.nr_inputs;
			break;
		case k_eControllerAxis_Joystick:
			CI.input_type[ii] = VRI_STICK;
			CI.axis_type[ai++] = VRA_STICK_X;
			CI.axis_type[ai++] = VRA_STICK_Y;
			++CI.nr_inputs;
			break;
		case k_eControllerAxis_Trigger:
			CI.input_type[ii] = VRI_TRIGGER;
			CI.axis_type[ai++] = VRA_TRIGGER;
			++CI.nr_inputs;
			break;
		}
	CI.nr_axes = ai;
	// set controller input configuration to default
	for (int ii = 0; ii < CI.nr_inputs; ++ii) {
		auto cif = get_controller_input_config(ci, ii);
		cif.dead_zone = 0.1f;
		cif.precision = 0.0f;
		cif.threshold = 0.5f;
		set_controller_input_config(ci, ii, cif);
	}
	uint64_t button_flags = 0;
	uint64_t supported_buttons = get_uint64_property(get_hmd(), device_index, Prop_SupportedButtons_Uint64);
	if ((supported_buttons & ButtonMaskFromId(k_EButton_System)) != 0)
		button_flags += VRF_SYSTEM;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_ApplicationMenu)) != 0)
		button_flags += VRF_MENU;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_Grip)) != 0)
		button_flags += VRF_GRIP;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_DPad_Left)) != 0)
		button_flags += VRF_DPAD_LEFT;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_DPad_Right)) != 0)
		button_flags += VRF_DPAD_RIGHT;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_DPad_Down)) != 0)
		button_flags += VRF_DPAD_DOWN;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_DPad_Up)) != 0)
		button_flags += VRF_DPAD_UP;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_A)) != 0)
		button_flags += VRF_A;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_Axis0)) != 0)
		button_flags += VRF_INPUT0;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_Axis1)) != 0)
		button_flags += VRF_INPUT1;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_Axis2)) != 0)
		button_flags += VRF_INPUT2;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_Axis3)) != 0)
		button_flags += VRF_INPUT3;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_Axis4)) != 0)
		button_flags += VRF_INPUT4;
	if ((supported_buttons & ButtonMaskFromId(k_EButton_ProximitySensor)) != 0)
		button_flags += VRF_PROXIMITY;
	CI.supported_buttons = VRButtonStateFlags(button_flags);
}

/// update tracker info
void openvr_kit::update_tracker_info(int ci, vr::TrackedDeviceIndex_t device_index)
{
	std::string serial_number = get_string_property(get_hmd(), device_index, Prop_SerialNumber_String);
	auto& CI = info.controller[ci];
	if (CI.serial_number == serial_number) {
		update_trackable_info(CI, device_index, true);
		return;
	}
	update_trackable_info(CI, device_index, false);
	CI.serial_number = serial_number;
	CI.type = VRC_TRACKER;
	CI.role = VRC_NOT_ASSIGNED;
}

/// update tracker info
void openvr_kit::update_tracking_reference_info(const std::string& serial, vr::TrackedDeviceIndex_t device_index)
{
	auto& TSI = ref_tracking_system_info();
	if (TSI.name.empty())
		TSI.name = get_string_property(get_hmd(), device_index, Prop_TrackingSystemName_String);

	if (TSI.references.find(serial) == TSI.references.end()) {
		auto& TRI = TSI.references[serial];
		update_trackable_info(TRI, device_index, false);
		TRI.serial_number = serial;
		TRI.mode = get_string_property(get_hmd(), device_index, Prop_ModeLabel_String);
		TRI.z_near = get_float_property(get_hmd(), device_index, Prop_TrackingRangeMinimumMeters_Float);
		TRI.z_far  = get_float_property(get_hmd(), device_index, Prop_TrackingRangeMaximumMeters_Float);
		TRI.frustum[0] = float(TRI.z_near * tan(M_PI / 180 * get_float_property(get_hmd(), device_index, Prop_FieldOfViewLeftDegrees_Float)));
		TRI.frustum[1] = float(TRI.z_near * tan(M_PI / 180 * get_float_property(get_hmd(), device_index, Prop_FieldOfViewRightDegrees_Float)));
		TRI.frustum[2] = float(TRI.z_near * tan(M_PI / 180 * get_float_property(get_hmd(), device_index, Prop_FieldOfViewBottomDegrees_Float)));
		TRI.frustum[3] = float(TRI.z_near * tan(M_PI / 180 * get_float_property(get_hmd(), device_index, Prop_FieldOfViewTopDegrees_Float)));
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

	// special query for controllers assigned to hands
	bool updated[2] = { false, false };
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
			extract_controller_state(controller_state, ci, state.controller[ci]);
			update_controller_info(ci, dis[ci]);
			updated[ci] = true;
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
	for (int ci = 2; ci < vr::max_nr_controllers; ++ci)
		state.controller[ci].status = vr::VRS_DETACHED;
	clear_tracking_reference_states();
	for (int device_index = 0; device_index < vr::k_unMaxTrackedDeviceCount; ++device_index)
	{
		if (tracked_poses[device_index].bPoseIsValid) {
			vr_trackable_state* tracked_pose_ptr = 0;
			ETrackedDeviceClass tdc = get_hmd()->GetTrackedDeviceClass(device_index);
			switch (tdc) {
			case TrackedDeviceClass_HMD:
				tracked_pose_ptr = &state.hmd;
				update_hmd_info();
				break;
			case TrackedDeviceClass_Controller:
				// check for controllers corresponding to left and right hand which are assigned 
				// to controller indices 0 and 1
				if (device_index == dis[0] || device_index == dis[1]) {
					int ci = (device_index == dis[0] ? 0 : 1);
					// only update state if not yet done before
					if (!updated[ci]) {
						tracked_pose_ptr = &state.controller[ci];
						state.controller[ci].status = vr::VRS_TRACKED;
						update_controller_info(ci, device_index);
					}
				}
				// otherwise we found a new controller
				else {
					if (next_generic_controller_index < vr::max_nr_controllers) {
						tracked_pose_ptr = &state.controller[next_generic_controller_index];
						state.controller[next_generic_controller_index].status = vr::VRS_TRACKED;
						update_controller_info(next_generic_controller_index, device_index);
						++next_generic_controller_index;
					}
				}
				break;
			case TrackedDeviceClass_GenericTracker:
				if (next_generic_controller_index < vr::max_nr_controllers) {
					tracked_pose_ptr = &state.controller[next_generic_controller_index];
					state.controller[next_generic_controller_index].status = vr::VRS_TRACKED;
					update_tracker_info(next_generic_controller_index, device_index);
					++next_generic_controller_index;
				}
				break;
			case TrackedDeviceClass_TrackingReference :
				{
					std::string serial_number = get_string_property(get_hmd(), device_index, Prop_SerialNumber_String);
					tracked_pose_ptr = &ref_tracking_reference_state(serial_number);
					update_tracking_reference_info(serial_number, device_index);
				}
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
	//VRInput()->TriggerHapticVibrationAction(k_ulInvalidActionHandle, 0.1f, 0.3f, 500, 1000, k_ulInvalidInputValueHandle);
	get_hmd()->TriggerHapticPulse(
		get_hmd()->GetTrackedDeviceIndexForControllerRole(controller_index == 0 ? TrackedControllerRole_LeftHand : TrackedControllerRole_RightHand),
		0, high_frequency_strength);
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


