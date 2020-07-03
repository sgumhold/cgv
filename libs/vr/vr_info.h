#pragma once

#include <map>

#include "lib_begin.h"

///@ingroup VR
///@{

///
namespace vr {

	/// information provided for any device
	struct vr_device_info
	{
		std::string serial_number;
		std::string model_number;
		/// a map from parameter name to parameter value used for all device parameters that are not explicitly defined
		std::map<std::string, std::string> variable_parameters;
	};

	/// information provided for trackable devices
	struct vr_trackable_info : public vr_device_info
	{
	};

	/// information provided for hmd device
	struct vr_hmd_info : public vr_trackable_info
	{

	};

	/// enum for information on controller type
	enum VRControllerType {
		VRC_NONE = 0,
		VRC_CONTROLLER = 1,
		VRC_TRACKER = 2
	};

	/// different controller axis types
	enum VRAxisType {
		VRA_NONE = 0,
		VRA_TRIGGER = 1,
		VRA_PAD_X = 2,
		VRA_PAD_Y = 3,
		VRA_STICK_X = 4,
		VRA_STICK_Y = 5
	};

	/// information provided for controller devices
	struct vr_controller_info : public vr_trackable_info
	{
		VRControllerType type;
		VRAxisType axis_type[8];
		uint64_t supported_buttons;
	};

	/// information provided for a vr kit
	struct vr_kit_info
	{
		vr_hmd_info hmd;
		vr_controller_info controller[4];
	};

	/// information provided for a base station / tracking reference
	struct vr_tracking_reference_info : vr_trackable_info
	{
		float z_near, z_far;
		float frustum[4];
		std::string mode;
	};

	/// information provided for tracking system
	struct vr_tracking_system_info
	{
		std::string name;
		std::map<std::string, vr_tracking_reference_info> references;
	};
}
