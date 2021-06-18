#include "vr_info.h"
#include "iomanip"

namespace vr {
	/// construct empty info
	vr_device_info::vr_device_info()
	{
	}
	/// stream out operator for device infos
	std::ostream& operator << (std::ostream& os, const vr_device_info& di)
	{
		os << "serial number = " << di.serial_number << std::endl;
		os << "model number = " << di.model_number << std::endl;
		for (const auto& pp : di.variable_parameters)
			os << pp.first << " = " << pp.second << std::endl;
		return os;
	}
	vr_trackable_info::vr_trackable_info()
	{
		is_wireless = false;
		provides_battery_charge_level = false;
		battery_charge_level = 0;
		device_class = -1;
		has_proximity_sensor = false;
	}
	std::ostream& operator << (std::ostream& os, const vr_trackable_info& TI)
	{
		os << static_cast<const vr_device_info&>(TI);
		os << "device_type = " << TI.device_type << std::endl;
		os << "device_class = " << TI.device_class << std::endl;
		if (TI.is_wireless)
			os << "is wireless" << std::endl;
		if (TI.provides_battery_charge_level)
			os << "battery charge level = " << TI.battery_charge_level << std::endl;
		if (TI.has_proximity_sensor)
			os << "has proximity sensor" << std::endl;
		return os;
	}
	vr_hmd_info::vr_hmd_info()
	{
		reports_time_since_vsynch = false;
		seconds_vsynch_to_photons = 0;
		fps = 60;
		ipd = 0;
		head_to_eye_distance = 0;
		number_cameras = 0;

		/*
		std::fill(camera_to_head_transform[0], camera_to_head_transform[0] + 12, 0.0f);
		std::fill(camera_to_head_transform[1], camera_to_head_transform[1] + 12, 0.0f);
		std::fill(imu_to_head_transform, imu_to_head_transform + 12, 0.0f);
		imu_gyro_bias[0] = imu_gyro_bias[1] = imu_gyro_bias[2] = 0.0f;
		imu_gyro_scale[0] = imu_gyro_scale[1] = imu_gyro_scale[2] = 1.0f;
		imu_accelerometer_bias[0]= imu_accelerometer_bias[1]=imu_accelerometer_bias[2]=0.0f;
		imu_accelerometer_scale[0]= imu_accelerometer_scale[1]= imu_accelerometer_scale[2]=1.0f;
		*/
		lighthouse_2_0_features = false;
	}
	void stream_vec3(std::ostream& os, const float vec[3])
	{
		os << "[";
		for (int i = 0; i < 3; ++i) {
			if (i > 0)
				os << ", ";
			os << vec[i];
		}
		os << "]" << std::endl;
	}
	void stream_pose(std::ostream& os, const float mat[12])
	{
		os << "\n";
		for (int i = 0; i < 3; ++i) {
			os << "  |";
			for (int j = 0; j < 4; ++j)
				os << std::setw(9) << std::setprecision(3) << mat[i + 3 * j];
			os << "|\n";
		}
	}
	std::ostream& operator << (std::ostream& os, const vr_hmd_info& HI)
	{
		os << static_cast<const vr_trackable_info&>(HI);
		if (HI.reports_time_since_vsynch)
			os << "reports time since vsynch with " << HI.seconds_vsynch_to_photons << " seconds to photons" << std::endl;
		os << "fps = " << HI.fps << std::endl;
		os << "ipd = " << HI.ipd << std::endl;
		if (HI.number_cameras > 0)
			os << "number cameras = " << HI.number_cameras << std::endl;
		/*
		os << "head_to_eye_distance = " << HI.head_to_eye_distance << std::endl;
		if (HI.number_cameras == 1)
			os << "camera to head transform = "; stream_pose(os, HI.camera_to_head_transform[0]);
		if (HI.number_cameras == 2) {
			os << "left camera to head transform = "; stream_pose(os, HI.camera_to_head_transform[0]);
			os << "right camera to head transform = "; stream_pose(os, HI.camera_to_head_transform[1]);
		}
		os << "" << HI.imu_to_head_transform[12] << std::endl;
		os << "imu gyro bias = "; stream_vec3(os, HI.imu_gyro_bias);
		os << "imu gyro scale = ";  stream_vec3(os, HI.imu_gyro_scale);
		os << "imu accel bias = "; stream_vec3(os, HI.imu_accelerometer_bias);
		os << "imu accel scale = "; stream_vec3(os, HI.imu_accelerometer_scale);
		*/
		if (HI.lighthouse_2_0_features)
			os << "has lighthouse 2.0 features" << std::endl;
		return os;
	}
	/// construct with default
	vr_controller_info::vr_controller_info()
	{
		type = VRC_NONE;
		nr_inputs = 0;
		std::fill(input_type, input_type + max_nr_controller_inputs, VRI_NONE);
		nr_axes = 0;
		std::fill(axis_type, axis_type + max_nr_controller_axes, VRA_NONE);
		supported_buttons = VRButtonStateFlags(0);
	}
	std::ostream& operator << (std::ostream& os, const vr_controller_info& CI)
	{
		const char* controller_type_strings[] = { "none", "controller", "tracker" };
		const char* input_type_strings[] = { "none", "trigger", "pad", "strick" };
		const char* axis_type_strings[] = { "none", "trigger", "pad_x", "pad_y", "strick_x", "stick_y" };
		os << "type = " << controller_type_strings[CI.type] << std::endl;
		os << static_cast<const vr_trackable_info&>(CI);
		if (CI.nr_inputs == 0)
			os << "no inputs provided" << std::endl;
		else {
			os << "input types = [";
			for (int i = 0; i < CI.nr_inputs; ++i) {
				if (i > 0)
					os << ",";
				os << input_type_strings[CI.input_type[i]];
			}
			os << "]" << std::endl;
			os << "axis types = [";
			for (int ai = 0; ai < CI.nr_axes; ++ai) {
				if (ai > 0)
					os << ",";
				os << axis_type_strings[CI.axis_type[ai]];
			}
			os << "]" << std::endl;
		}
		os << "button support = " << get_state_flag_string(CI.supported_buttons) << std::endl;
		return os;
	}
	vr_kit_info::vr_kit_info()
	{
	}

	std::ostream& operator << (std::ostream& os, const vr_kit_info& VI)
	{
		os << "VR KIT\n->HMD\n" << VI.hmd;
		for (int ci = 0; ci < max_nr_controllers; ++ci)
			os << "->CONTROLLER[" << ci << "]\n" << VI.controller[ci];
		return os;
	}

	/// construct with default values
	vr_tracking_reference_info::vr_tracking_reference_info()
	{
		mode = "undef";
		z_near = 0.1f;
		z_far = 4.0f;
		frustum[0] = -0.1f;
		frustum[1] = 0.1f;
		frustum[2] = -0.1f;
		frustum[3] = 0.1f;
	}

	std::ostream& operator << (std::ostream& os, const vr_tracking_reference_info& TI)
	{
		os << static_cast<const vr_trackable_info&>(TI);
		os << "tracking mode = " << TI.mode << std::endl;
		os << "z range = [" << TI.z_near << ", " << TI.z_far << "]" << std::endl;
		os << "frustum = [" << TI.frustum[0] << ", " << TI.frustum[1] << ", "
			<< TI.frustum[2] << ", " << TI.frustum[3] << "]" << std::endl;
		return os;
	}

	vr_tracking_system_info::vr_tracking_system_info()
	{
	}
	std::ostream& operator << (std::ostream& os, const vr_tracking_system_info& TSI)
	{
		os << "tracking system name = " << TSI.name << std::endl;
		for (const auto& rr : TSI.references) {
			os << "tracking reference " << rr.first << " {" << std::endl;
			os << rr.second << "}" << std::endl;
		}
		return os;
	}
}