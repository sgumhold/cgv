#pragma once

#include <iostream>
#include <map>
#include "vr_state.h"

#include "lib_begin.h"

///@ingroup VR
///@{

///
namespace vr {
	/// information provided for any device type
	struct CGV_API vr_device_info
	{
		/// unique identifier of device
		std::string serial_number;
		/// number describing the device type
		std::string model_number;
		/// a map from parameter name to parameter value used for all device parameters that are not explicitly defined
		std::map<std::string, std::string> variable_parameters;
		/// construct empty info
		vr_device_info();
	};

	/// stream out operator for device infos
	extern CGV_API std::ostream& operator << (std::ostream& os, const vr_device_info& DI);

	/// information provided for trackable devices
	struct CGV_API vr_trackable_info : public vr_device_info
	{
		/// type name of device
		std::string device_type;
		/// index of device class
		int32_t device_class;
		/// whether device is wireless
		bool is_wireless;
		/// whether one can query battery charge level
		bool provides_battery_charge_level;
		/// battery charge level in percentage (0 .. empty, 1 .. full)
		float battery_charge_level;
		/// whether device has proximity sensor
		bool has_proximity_sensor;
		/// construct with default values
		vr_trackable_info();
	};

	/// stream out operator for trackable device infos
	extern CGV_API std::ostream& operator << (std::ostream& os, const vr_trackable_info& TI);

	/// information provided for hmd device
	struct CGV_API vr_hmd_info : public vr_trackable_info
	{
		/// whether time since vsynch is reported
		bool reports_time_since_vsynch;
		/// seconds from vsynch to photons
		float seconds_vsynch_to_photons;
		/// frame rate
		float fps;
		/// interpupilar distance measured in meters
		float ipd;
		/// distance from head origin to eye centers in meters
		float head_to_eye_distance;
		/// number of cameras
		int32_t number_cameras;
		/*
		/// imu to head pose matrix
		float imu_to_head_transform[12];
		/// imu bias vector for gyro
		float imu_gyro_bias[3];
		/// imu scale vector for gyro
		float imu_gyro_scale[3];
		/// imu bias vector for accelerometer
		float imu_accelerometer_bias[3];
		/// imu scale vector for accelerometer
		float imu_accelerometer_scale[3];
		*/
		/// whether lighthouse 2.0 features are supported
		bool lighthouse_2_0_features;
		/// construct with default values
		vr_hmd_info();
	};
	/// stream out operator for hmd device infos
	extern CGV_API std::ostream& operator << (std::ostream& os, const vr_hmd_info& HI);

	/// type of controller
	enum VRControllerType {
		VRC_NONE = 0,
		VRC_CONTROLLER = 1,
		VRC_TRACKER = 2
	};
	/// role of controller
	enum VRControllerRole {
		VRC_NOT_ASSIGNED = 0,
		VRC_LEFT_HAND,
		VRC_RIGHT_HAND,
		VRC_TREADMILL,
		VRC_STYLUS,
		VRC_ROLE_END
	};
	/// different controller input types
	enum VRInputType {
		VRI_NONE,
		VRI_TRIGGER,
		VRI_PAD,
		VRI_STICK
	};
	/// different axis types of controller inputs
	enum VRAxisType {
		VRA_NONE = 0,
		VRA_TRIGGER = 1,
		VRA_PAD_X = 2,
		VRA_PAD_Y = 3,
		VRA_STICK_X = 4,
		VRA_STICK_Y = 5
	};
	/// information provided for controller devices
	struct CGV_API vr_controller_info : public vr_trackable_info
	{
		/// controller type
		VRControllerType type;
		/// controller role
		VRControllerRole role;
		/// number of used inputs
		int32_t nr_inputs;
		/// type of up to 5\c vr::max_nr_controller_inputs inputs built into the controller
		VRInputType input_type[max_nr_controller_inputs];
		/// total number of axes provided by all inputs
		int32_t nr_axes;
		//! axis type for each of the \c vr::max_nr_controller_axes axes in the state
		/*! axes are enumerated in the order of the inputs. Typically, the first is
		    a 2d input (pad or stick) with the corresponding axes indexed with 0 and 1.
			The first axis of the second input is in this case 2. */
		VRAxisType axis_type[max_nr_controller_axes];
		/// one flag per button telling whether it is supported
		VRButtonStateFlags supported_buttons;
		/// construct with default
		vr_controller_info();
	};
	/// stream out operator for controller device infos
	extern CGV_API std::ostream& operator << (std::ostream& os, const vr_controller_info& CI);

	/// information provided for a vr kit
	struct CGV_API vr_kit_info
	{
		/// whether force feedback is supported
		bool force_feedback_support;
		/// information for head mounted display
		vr_hmd_info hmd;
		/// information for attached controllers and trackers
		vr_controller_info controller[max_nr_controllers];
		/// construct with default values
		vr_kit_info();
	};
	/// stream out operator for vr kit info
	extern CGV_API std::ostream& operator << (std::ostream& os, const vr_kit_info& VI);

	/// information provided for a base station / tracking reference
	struct CGV_API vr_tracking_reference_info : vr_trackable_info
	{
		/// tracking mode string
		std::string mode;
		/// near and far clipping planes of tracking frustum in meters
		float z_near, z_far;
		/// frustum relative to z_near in the order left, right, bottom, up
		float frustum[4];
		/// construct with default values
		vr_tracking_reference_info();
	};
	/// stream out operator for tracking reference device infos
	extern CGV_API std::ostream& operator << (std::ostream& os, const vr_tracking_reference_info& TI);

	/// information provided for tracking system
	struct CGV_API vr_tracking_system_info
	{
		/// name of tracking system
		std::string name;
		/// map from serial number to info of corresponding tracking reference
		std::map<std::string, vr_tracking_reference_info> references;
		/// construct with default values
		vr_tracking_system_info();
	};
	/// stream out operator for tracking system infos
	extern CGV_API std::ostream& operator << (std::ostream& os, const vr_tracking_system_info& TSI);
}

///@}
