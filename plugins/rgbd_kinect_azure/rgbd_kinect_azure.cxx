#include <iostream>
#include <map>
#include <regex>
#include "rgbd_kinect_azure.h"
#include <cgv/utils/convert.h>
#include <cgv/os/system_devices.h>
#include <cgv/math/pose.h>
#include <k4a/k4atypes.h>

using namespace std;
using namespace std::chrono;

void dummy(void* buffer,void* context){}


//constexpr emulator_parameters

constexpr rgbd::camera_intrinsics azure_color_intrinsics = { 3.765539746314990e+02, 3.758362623002023e+02,6.971111133663950e+02,3.753833895962808e+02, 0.0, 768, 1366 };

namespace rgbd {
	// store mapping from device index to serial of all attached devices
	std::map<int, std::string> serial_map;
	// store mapping from serial to audio device id
	std::map<std::string, std::string> audio_device_id_map;
	// put i-th serial in result and return whether device is attached to
	bool query_serial(int i, std::string& result)
	{
		if (serial_map.find(i) != serial_map.end()) {
			result = serial_map[i];
			return true;
		}
		k4a::device device;
		try {
			device = k4a::device::open(i);
			result = device.get_serialnum();
			device.close();
			return false;
		}
		catch (runtime_error e) {
			cerr << "rgbd_kinect_azure_driver: " << e.what() << '\n';
		}
		return false;
	}
	std::string query_audio_device_id(const std::string& serial)
	{
		// first check if audio_device_id_map knows result
		auto iter = audio_device_id_map.find(serial);
		if (iter != audio_device_id_map.end())
			return iter->second;
		// otherwise rebuild the audio_device_id_map
		audio_device_id_map.clear();
		// first construct map from container id to audio device id
		std::map<cgv::utils::guid, std::string> container_to_audio_device_id_map;
		std::vector<std::pair<std::string, cgv::utils::guid>> wasapi_devices;
		cgv::os::enumerate_system_devices(wasapi_devices, "SWD");
		for (const auto& SD : wasapi_devices) {
			const std::regex wasapiRegex("(SWD\\\\MMDEVAPI\\\\)(.*)");
			std::cmatch match;
			if (!std::regex_match(SD.first.c_str(), match, wasapiRegex))
				continue;
			container_to_audio_device_id_map[SD.second] = match[2];
		}

		// next iterate azure devices and build map from azure serial number to audio device id by matching container ids
		std::vector<std::pair<std::string, cgv::utils::guid>> azure_devices;
		cgv::os::enumerate_system_devices(azure_devices, "USB");
		for (const auto& AD : azure_devices) {
			const std::regex vidPidRegex("(USB\\\\VID_([0-9A-F]{4})&PID_([0-9A-F]{4})\\\\(.*))");
			std::cmatch match;
			if (!std::regex_match(AD.first.c_str(), match, vidPidRegex))
				continue;
			// validate video id
			if (uint16_t(std::stoul(match[2], nullptr, 16)) != uint16_t(0x045E))
				continue;
			// validate depth id
			if (uint16_t(std::stoul(match[3], nullptr, 16)) != uint16_t(0x097C))
				continue;
			// find corresponding audio device and insert into map
			auto jter = container_to_audio_device_id_map.find(AD.second);
			if (jter != container_to_audio_device_id_map.end())
				audio_device_id_map[match[4]] = jter->second;
		}
		// finally query new map and return audio device id or empty string if audio device could not be matched
		iter = audio_device_id_map.find(serial);
		if (iter != audio_device_id_map.end())
			return iter->second;
		return "";
	}
	/// create a detached kinect device object
	rgbd_kinect_azure::rgbd_kinect_azure()
	{
		device_serial = "";
		device_started = false;
		has_new_color_frame = has_new_depth_frame = has_new_ir_frame = false;
		has_new_IMU_data = false;
		imu_enabled = false;
	}
	rgbd_kinect_azure::~rgbd_kinect_azure()
	{
		stop_device();
		if (capture_thread)
			capture_thread->join();
	}
	std::string rgbd_kinect_azure::get_audio_device() const
	{
		if (is_attached())
			return query_audio_device_id(device_serial);
		else
			return "";
	}
	/// attach to the kinect device of the given serial
	bool rgbd_kinect_azure::attach(const std::string& serial)
	{
		if (is_attached()) {
			if (serial == device_serial) {
				return true;
			}
			if (!detach()) {
				cerr	<< "rgbd_kinect_azure::attach: failed detaching the already attached device! [serial:" 
						<< device_serial << "]\n";
				return false;
			}
		}
		color_frames.resize(frame_cache_size);
		depth_frames.resize(frame_cache_size);
		ir_frames.resize(frame_cache_size);
		current_read_color_frame = current_write_color_frame = current_read_depth_frame = current_write_depth_frame = current_read_ir_frame = current_write_ir_frame = 0;
		has_new_color_frame = has_new_depth_frame = has_new_ir_frame = false;
		//find device with given serial
		uint32_t device_count = k4a::device::get_installed_count();
		for (uint32_t i = 0; i < device_count;++i) {
			std::string serial_i;
			if (query_serial(i, serial_i)) {
				if (serial_i == serial) {
					std::cerr << "rgbd_kinect_azure::attach(" << serial << ") failed as physical device is already attached to other device." << std::endl;
					return false;
				}
				continue;
			}
			if (serial_i == serial) {
				device = k4a::device::open(i);
				this->device_serial = serial;
				serial_map[i] = serial;
				return true;
			}
		}
		return false;
	}

	/// return whether device object is attached to a kinect device
	bool rgbd_kinect_azure::is_attached() const
	{
		return (device_serial.size() > 0) && device;
	}
	/// detach from serial (done automatically in constructor)
	bool rgbd_kinect_azure::detach()
	{
		if (is_running()) {
			stop_device();
		}
		if (is_attached()) {
			device.close();
			for (auto sme : serial_map) {
				if (sme.second == device_serial) {
					serial_map.erase(sme.first);
					break;
				}
			}
			device_serial = "";
			return true;
		}
		return false;
	}
	/// check whether rgbd device has inertia measurement unit
	bool rgbd_kinect_azure::has_IMU() const
	{
		return true;
	}
	/// return additional information on inertia measurement unit, copy from rgbd_kinect.cxx
	const IMU_info& rgbd_kinect_azure::get_IMU_info() const
	{
		static IMU_info info;
		info.has_angular_acceleration = true;
		info.has_linear_acceleration = true;
		info.has_time_stamp_support = true;
		return info;
	}
	/// query the current measurement of the acceleration sensors within the given time_out in milliseconds; return whether a measurement has been retrieved
	bool rgbd_kinect_azure::put_IMU_measurement(IMU_measurement& m, unsigned time_out) const
	{
		if (!imu_enabled)
			return false;
		//capture_lock.lock();
		k4a_imu_sample_t imu_sample;
		if (k4a_device_get_imu_sample(device.handle(), &imu_sample, time_out) != K4A_WAIT_RESULT_SUCCEEDED)
			return false;
		m.angular_velocity[0] = imu_sample.gyro_sample.v[0];
		m.angular_velocity[1] = imu_sample.gyro_sample.v[1];
		m.angular_velocity[2] = imu_sample.gyro_sample.v[2];
		m.linear_acceleration[0] = imu_sample.acc_sample.v[0];
		m.linear_acceleration[1] = imu_sample.acc_sample.v[1];
		m.linear_acceleration[2] = imu_sample.acc_sample.v[2];
		m.time_stamp = imu_sample.acc_timestamp_usec;
		m.angular_time_stamp = imu_sample.gyro_timestamp_usec;
		return true;
	}
	/// check whether the device supports the given combination of input streams
	bool rgbd_kinect_azure::check_input_stream_configuration(InputStreams is) const
	{
		return false;
	}
	/// query the stream formats available for a given stream configuration
	void rgbd_kinect_azure::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
		const static float fps[] = { 5.0f,15.0f,30.0f };
		constexpr int fps_size = sizeof(fps) / sizeof(int);
		if ((is & IS_COLOR) != 0) {
			
			for (int i = 0; i < fps_size; ++i) {
				stream_formats.push_back(stream_format(1280, 720, PF_BGR, fps[i], 32));
				stream_formats.push_back(stream_format(1920, 1080, PF_BGR, fps[i], 32));
				stream_formats.push_back(stream_format(2560, 1440, PF_BGR, fps[i], 32));
				stream_formats.push_back(stream_format(2048, 1536, PF_BGR, fps[i], 32));
				if (fps[i] < 30) {
					stream_formats.push_back(stream_format(3840, 2160, PF_BGR, fps[i], 32));
				}
			}
		}
		if ((is & IS_INFRARED) != 0) {
			for (int i = 0; i < fps_size; ++i) {
				stream_formats.push_back(stream_format(1024, 1024, PF_I, fps[i], 16));

				stream_formats.push_back(stream_format(320, 288, PF_I, fps[i], 16));
				stream_formats.push_back(stream_format(512, 512, PF_I, fps[i], 16));
				stream_formats.push_back(stream_format(640, 576, PF_I, fps[i], 16));
			}
		}
		if ((is & IS_DEPTH) != 0) {
			for (int i = 0; i < fps_size; ++i) {
				//binned Formats
				stream_formats.push_back(stream_format(320, 288, PF_DEPTH, fps[i], 16));
				stream_formats.push_back(stream_format(512, 512, PF_DEPTH, fps[i], 16));
				//unbinned Formats
				stream_formats.push_back(stream_format(640,576, PF_DEPTH, fps[i], 16));
				if (fps[i] < 30) {
					stream_formats.push_back(stream_format(1024, 1024, PF_DEPTH, fps[i], 16));
				}
			}
		}
	}
	/// start the camera
	bool rgbd_kinect_azure::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{
		if (is & IS_COLOR) {
			stream_formats.push_back(stream_format(2048, 1536, PF_BGR, 30, 32));
		}
		if (is & IS_DEPTH) {
			stream_formats.push_back(stream_format(512, 512, PF_DEPTH, 30, 16));
		}
		if (is & IS_INFRARED) {
			stream_formats.push_back(stream_format(512, 512, PF_I, 30, 16));
		}
		if (is_running())
			return true;
		return start_device(stream_formats);
	}
	/// check whether a multi-device role is supported
	bool rgbd_kinect_azure::is_supported(MultiDeviceRole mdr) const
	{
		return true;
	}
	/// configure device for a multi-device role and return whether this was successful (do this before starting)
	bool rgbd_kinect_azure::configure_role(MultiDeviceRole mdr)
	{
		multi_device_role = mdr;
		return true;
	}
	/// whether device supports external synchronization
	bool rgbd_kinect_azure::is_sync_supported() const
	{
		return true;
	}
	/// return whether syncronization input jack is connected
	bool rgbd_kinect_azure::is_sync_in_connected() const
	{
		return device.is_sync_in_connected();
	}
	/// return whether syncronization output jack is connected
	bool rgbd_kinect_azure::is_sync_out_connected() const
	{
		return device.is_sync_out_connected();
	}
	const std::vector<color_parameter_info>& rgbd_kinect_azure::get_supported_color_control_parameter_infos() const
	{
		if (ccp_infos.empty()) {
			bool supports_auto;
			int32_t min_value;
			int32_t max_value;
			int32_t step_value;
			int32_t default_value;
			k4a_color_control_mode_t default_mode;
			if (k4a_device_get_color_control_capabilities(device.handle(), K4A_COLOR_CONTROL_EXPOSURE_TIME_ABSOLUTE, &supports_auto, &min_value, &max_value, &step_value, &default_value, &default_mode) == K4A_RESULT_SUCCEEDED)
				ccp_infos.push_back({ "exposure", CCP_EXPOSURE, supports_auto, default_mode == K4A_COLOR_CONTROL_MODE_AUTO, min_value, max_value, step_value, default_value });
			if (k4a_device_get_color_control_capabilities(device.handle(), K4A_COLOR_CONTROL_BRIGHTNESS, &supports_auto, &min_value, &max_value, &step_value, &default_value, &default_mode) == K4A_RESULT_SUCCEEDED)
				ccp_infos.push_back({ "brightness", CCP_BRIGHTNESS, supports_auto, default_mode == K4A_COLOR_CONTROL_MODE_AUTO, min_value, max_value, step_value, default_value });
			if (k4a_device_get_color_control_capabilities(device.handle(), K4A_COLOR_CONTROL_CONTRAST, &supports_auto, &min_value, &max_value, &step_value, &default_value, &default_mode) == K4A_RESULT_SUCCEEDED)
				ccp_infos.push_back({ "contrast", CCP_CONTRAST, supports_auto, default_mode == K4A_COLOR_CONTROL_MODE_AUTO, min_value, max_value, step_value, default_value });
			if (k4a_device_get_color_control_capabilities(device.handle(), K4A_COLOR_CONTROL_SATURATION, &supports_auto, &min_value, &max_value, &step_value, &default_value, &default_mode) == K4A_RESULT_SUCCEEDED)
				ccp_infos.push_back({ "saturation", CCP_SATURATION, supports_auto, default_mode == K4A_COLOR_CONTROL_MODE_AUTO, min_value, max_value, step_value, default_value });
			if (k4a_device_get_color_control_capabilities(device.handle(), K4A_COLOR_CONTROL_SHARPNESS, &supports_auto, &min_value, &max_value, &step_value, &default_value, &default_mode) == K4A_RESULT_SUCCEEDED)
				ccp_infos.push_back({ "sharpness", CCP_SHARPNESS, supports_auto, default_mode == K4A_COLOR_CONTROL_MODE_AUTO, min_value, max_value, step_value, default_value });
			if (k4a_device_get_color_control_capabilities(device.handle(), K4A_COLOR_CONTROL_WHITEBALANCE, &supports_auto, &min_value, &max_value, &step_value, &default_value, &default_mode) == K4A_RESULT_SUCCEEDED)
				ccp_infos.push_back({ "whitebalance[Kelvin]", CCP_WHITEBALANCE, supports_auto, default_mode == K4A_COLOR_CONTROL_MODE_AUTO, min_value, max_value, step_value, default_value });
			if (k4a_device_get_color_control_capabilities(device.handle(), K4A_COLOR_CONTROL_BACKLIGHT_COMPENSATION, &supports_auto, &min_value, &max_value, &step_value, &default_value, &default_mode) == K4A_RESULT_SUCCEEDED)
				ccp_infos.push_back({ "backlight_compensation(off,on)", CCP_BACKLIGHT_COMPENSATION, supports_auto, default_mode == K4A_COLOR_CONTROL_MODE_AUTO, min_value, max_value, step_value, default_value });
			if (k4a_device_get_color_control_capabilities(device.handle(), K4A_COLOR_CONTROL_GAIN, &supports_auto, &min_value, &max_value, &step_value, &default_value, &default_mode) == K4A_RESULT_SUCCEEDED)
				ccp_infos.push_back({ "gain", CCP_GAIN, supports_auto, default_mode == K4A_COLOR_CONTROL_MODE_AUTO, min_value, max_value, step_value, default_value });
			if (k4a_device_get_color_control_capabilities(device.handle(), K4A_COLOR_CONTROL_POWERLINE_FREQUENCY, &supports_auto, &min_value, &max_value, &step_value, &default_value, &default_mode) == K4A_RESULT_SUCCEEDED)
				ccp_infos.push_back({ "powerline_frequency(off,50Hz,60Hz)", CCP_POWERLINE_FREQUENCY, supports_auto, default_mode == K4A_COLOR_CONTROL_MODE_AUTO, min_value, max_value, step_value, default_value });
		}
		return ccp_infos;
		/*
		static std::vector<color_parameter_info> I;
		if (I.empty()) {
			I.push_back({ "exposure", CCP_EXPOSURE, true, -11, 1, 1, -5 });
			I.push_back({ "brightness", CCP_BRIGHTNESS, false, 0, 255, 1, 128 });
			I.push_back({ "contrast", CCP_CONTRAST, false, 0, 255, 1, 128 });
			I.push_back({ "saturation", CCP_SATURATION, false, 0, 255, 1, 128 });
			I.push_back({ "sharpness", CCP_SHARPNESS, false, 0, 255, 1, 128 });
			I.push_back({ "whitebalance[Kelvin]", CCP_WHITEBALANCE, true, 0, 10000, 10, 5500 });
			I.push_back({ "backlight_compensation(off,on)", CCP_BACKLIGHT_COMPENSATION, false, 0, 1, 1, 0 });
			I.push_back({ "gain", CCP_GAIN, false, 0, 255, 1, 128 });
			I.push_back({ "powerline_frequency(off,50Hz,60Hz)", CCP_POWERLINE_FREQUENCY, false, 0, 2, 1, 0 });
		}
		return I;
		*/
	}
	k4a_color_control_command_t ccp_to_ccc(ColorControlParameter ccp)
	{
		static k4a_color_control_command_t cccs[] = {
			K4A_COLOR_CONTROL_EXPOSURE_TIME_ABSOLUTE,
			K4A_COLOR_CONTROL_BRIGHTNESS,
			K4A_COLOR_CONTROL_CONTRAST,
			K4A_COLOR_CONTROL_SATURATION,
			K4A_COLOR_CONTROL_SHARPNESS,
			K4A_COLOR_CONTROL_WHITEBALANCE,
			K4A_COLOR_CONTROL_BACKLIGHT_COMPENSATION,
			K4A_COLOR_CONTROL_GAIN,
			K4A_COLOR_CONTROL_POWERLINE_FREQUENCY
		};
		assert(ccp >= CCP_EXPOSURE && ccp <= CCP_POWERLINE_FREQUENCY);
		return cccs[ccp];
	}
	/// query color control value and whether its adjustment is in automatic mode
	std::pair<int32_t, bool> rgbd_kinect_azure::get_color_control_parameter(ColorControlParameter ccp) const
	{
		k4a_color_control_mode_t mode;
		int32_t value;
		device.get_color_control(ccp_to_ccc(ccp), &mode, &value);
		//if (ccp == CCP_EXPOSURE) {
		//
		//}
		return { value, mode == K4A_COLOR_CONTROL_MODE_AUTO };
	}
	/// set a color control value and automatic mode and return whether successful
	bool rgbd_kinect_azure::set_color_control_parameter(ColorControlParameter ccp, int32_t value, bool automatic_mode)
	{
		try {
			device.set_color_control(ccp_to_ccc(ccp), automatic_mode ? K4A_COLOR_CONTROL_MODE_AUTO : K4A_COLOR_CONTROL_MODE_MANUAL, value);
		}
		catch (k4a::error e)
		{
			std::cerr << "ERROR in set_color_control_parameter: " << e.what() << std::endl;
			return false;
		}
		return true;
	}

	/// start the rgbd device with given stream formats 
	bool rgbd_kinect_azure::start_device(const std::vector<stream_format>& stream_formats)
	{
		if (is_running())
			return true;

		//reset error information
		if (capture_thread_device_error && !recover_from_errors()){
			std::cerr << "rgbd_kinect_azure::start_device: could not recover from errors\n";
			return false;
		}

		k4a_device_configuration_t cfg = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
		cfg.color_resolution = K4A_COLOR_RESOLUTION_OFF;
		cfg.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
		cfg.depth_mode = K4A_DEPTH_MODE_OFF;
		switch (multi_device_role) {
		case MDR_LEADER: cfg.wired_sync_mode = K4A_WIRED_SYNC_MODE_MASTER; break;
		case MDR_FOLLOWER: cfg.wired_sync_mode = K4A_WIRED_SYNC_MODE_SUBORDINATE; break;
		default : cfg.wired_sync_mode = K4A_WIRED_SYNC_MODE_STANDALONE; break;
		}
		cfg.depth_delay_off_color_usec = 0;
		cfg.subordinate_delay_off_master_usec = 0;
		cfg.disable_streaming_indicator = false;
		float fps = 0;

		int color_stream_ix=-1, depth_stream_ix=-1, ir_stream_ix=-1;

		for (int i = 0; i < stream_formats.size();++i) {
			auto &format = stream_formats[i];
			if (fps == 0) {
				if (format.fps == 5) cfg.camera_fps = K4A_FRAMES_PER_SECOND_5;
				else if (format.fps == 15) cfg.camera_fps = K4A_FRAMES_PER_SECOND_15;
				else if (format.fps == 30) cfg.camera_fps = K4A_FRAMES_PER_SECOND_30;
				fps = format.fps;
			}
			else if (fps != format.fps) {
				cerr << "rgbd_kinect_azure::start_device: missmatching fps in selected formats\n";
				return false;
			}
			if (format.pixel_format == PF_BGR) {
				color_stream_ix = i;
			}
			else if (format.pixel_format == PF_DEPTH) {
				depth_stream_ix = i;
			}
			else if (format.pixel_format == PF_I) {
				ir_stream_ix = i;
			}
		}

		if (color_stream_ix != -1) {
			auto &format = stream_formats[color_stream_ix];
			if (format.height == 720) cfg.color_resolution = K4A_COLOR_RESOLUTION_720P;
			else if (format.height == 1080) cfg.color_resolution = K4A_COLOR_RESOLUTION_1080P;
			else if (format.height == 1440) cfg.color_resolution = K4A_COLOR_RESOLUTION_1440P;
			else if (format.height == 1536) cfg.color_resolution = K4A_COLOR_RESOLUTION_1536P;
			else if (format.height == 2160) cfg.color_resolution = K4A_COLOR_RESOLUTION_2160P;
			else if (format.height == 3072) cfg.color_resolution = K4A_COLOR_RESOLUTION_3072P;
			color_format = format;
		}
		if (depth_stream_ix != -1) {
			auto &format = stream_formats[depth_stream_ix];
			if (format.height == 288) cfg.depth_mode = K4A_DEPTH_MODE_NFOV_2X2BINNED;
			else if (format.height == 576) cfg.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
			else if (format.height == 512) cfg.depth_mode = K4A_DEPTH_MODE_WFOV_2X2BINNED;
			else if (format.height == 1024) cfg.depth_mode = K4A_DEPTH_MODE_WFOV_UNBINNED;
			depth_format = format;
		}

		if (ir_stream_ix != -1) {
			auto &format = stream_formats[ir_stream_ix];
			if (depth_stream_ix != -1) {
				if (stream_formats[depth_stream_ix].height != format.height && stream_formats[depth_stream_ix].width != format.width) {
					cerr << "rgbd_kinect_azure::start_device: selected ir and depth format need matching resolutions\n";
					return false;
				}
				ir_format = format;
			}
			else if (format.height == 1024 && format.width == 1024){
				cfg.depth_mode = K4A_DEPTH_MODE_PASSIVE_IR;
				ir_format = format;
			}
			else {
				cerr << "rgbd_kinect_azure::start_device: Without a depth stream the only supported ir resolution is 1024x1024.\n";
				return false;
			}
		}

		cfg.synchronized_images_only = (color_stream_ix != -1 && depth_stream_ix != -1);
		
		camera_calibration = device.get_calibration(cfg.depth_mode, cfg.color_resolution);
		camera_transform = k4a::transformation(camera_calibration);
		intrinsics = &camera_calibration.depth_camera_calibration.intrinsics.parameters;
		camera_calibration_t = &camera_calibration.depth_camera_calibration;
		intrinsics_t = &camera_calibration.depth_camera_calibration.intrinsics;

		try { device.start_cameras(&cfg); }
		catch (runtime_error e) {
			cerr << e.what() << '\n';
			return false;
		}
		try { 
			device.start_imu(); 
			imu_enabled = true; }
		catch (runtime_error e){
			cerr << "failed to start IMU\n" << e.what() << '\n';
			imu_enabled = false;
		}
		device_started = true;
		
		int is =
			((color_stream_ix != -1) ? IS_COLOR : IS_NONE) |
			((depth_stream_ix != -1) ? IS_DEPTH : IS_NONE) |
			((ir_stream_ix != -1) ? IS_INFRARED : IS_NONE);
				
		capture_thread = make_unique<thread>(&rgbd_kinect_azure::capture,this,is);
		return true;
	}
	
	/// query the calibration information and return whether this was successful
	bool rgbd_kinect_azure::query_calibration(rgbd_calibration& calib)
	{
		if (!is_running())
			return false;
		const auto extract_camera_calib = [this](const auto& C, auto& cam) {
			const auto& P = C.intrinsics.parameters.param;
			cam.w = C.resolution_width;
			cam.h = C.resolution_height;
			cam.max_radius_for_projection = C.metric_radius;
			cam.s = cgv::math::fvec<double, 2>(P.fx, P.fy);
			cam.c = cgv::math::fvec<double, 2>(P.cx, P.cy);
			cam.dc = cgv::math::fvec<double, 2>(P.codx, P.cody);
			cam.skew = 0.0;
			cam.k[0] = P.k1;
			cam.k[1] = P.k2;
			cam.k[2] = P.k3;
			cam.k[3] = P.k4;
			cam.k[4] = P.k5;
			cam.k[5] = P.k6;
			cam.p[0] = P.p1;
			cam.p[1] = P.p2;
			cam.pose = pose_construct(
				cgv::math::fmat<float, 3, 3>(3, 3, &C.extrinsics.rotation[0]),
				cgv::math::fvec<float, 3>(3, &C.extrinsics.translation[0]));

		};
		calib.depth_scale = 0.001;
		extract_camera_calib(camera_calibration.depth_camera_calibration, calib.depth);
		extract_camera_calib(camera_calibration.color_camera_calibration, calib.color);
		return true;
	}

	/// stop the camera
	bool rgbd_kinect_azure::stop_device()
	{
		if (!is_running())
			return true;
		device_started = false;
		imu_enabled = false;
		//terminate capture thread
		if (capture_thread) {
			capture_thread->join();
		}
		capture_thread = nullptr;
		//stop device
		device.stop_cameras();
		device.stop_imu();
		return true;
	}

	/// return whether device has been started
	bool rgbd_kinect_azure::is_running() const
	{
		return device_started.load(std::memory_order_relaxed);
	}

	/// query a frame of the given input stream
	bool rgbd_kinect_azure::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{
		if (!is_attached()) {
			cerr << "rgbd_kinect::get_frame called on device that has not been attached" << endl;
			return false;
		}

		if (!is_running()) {
			cerr << "rgbd_kinect::get_frame called on device that is not running" << endl;
			return false;
		}

		if (is == IS_COLOR && has_new_color_frame) {
			capture_lock.lock();
			frame = move(*color_frames[current_read_color_frame]);
			current_read_color_frame = (current_read_color_frame + 1) % frame_cache_size;
			has_new_color_frame = current_read_color_frame != current_write_color_frame;
			capture_lock.unlock();
			return true;
		} else if (is == IS_DEPTH && has_new_depth_frame) {
			capture_lock.lock();
			frame = move(*depth_frames[current_read_depth_frame]);
			current_read_depth_frame = (current_read_depth_frame + 1) % frame_cache_size;
			has_new_depth_frame = current_read_depth_frame != current_write_depth_frame;
			capture_lock.unlock();
			return true;
		} else if (is == IS_INFRARED && has_new_ir_frame) {
			capture_lock.lock();
			frame = move(*ir_frames[current_read_ir_frame]);
			current_read_ir_frame = (current_read_ir_frame + 1) % frame_cache_size;
			has_new_ir_frame = current_read_ir_frame != current_write_ir_frame;
			capture_lock.unlock();
			return true;
		}
		check_errors();
		return false;
	}
	/// map a color frame to the image coordinates of the depth image
	void rgbd_kinect_azure::map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
		frame_type& warped_color_frame) const
	{
		if (!is_attached()) {
			cerr << "rgbd_kinect::map_color_to_depth called on device that has not been attached" << endl;
			return;
		}
		// prepare warped frame
		static_cast<frame_size&>(warped_color_frame) = depth_frame;
		warped_color_frame.pixel_format = color_frame.pixel_format;
		warped_color_frame.nr_bits_per_pixel = color_frame.nr_bits_per_pixel;
		warped_color_frame.compute_buffer_size();
		warped_color_frame.frame_data.resize(warped_color_frame.buffer_size);
		unsigned bytes_per_pixel = color_frame.nr_bits_per_pixel / 8;

		vector<char> col_buffer = color_frame.frame_data;
		vector<char> dep_buffer = depth_frame.frame_data;

		k4a::image color_img = k4a::image::create_from_buffer(K4A_IMAGE_FORMAT_COLOR_BGRA32, color_frame.width, color_frame.height,
			color_frame.width * 4, reinterpret_cast<uint8_t*>(col_buffer.data()),
			color_frame.frame_data.size(),dummy,dummy);
		k4a::image depth_img = k4a::image::create_from_buffer(K4A_IMAGE_FORMAT_DEPTH16, depth_frame.width, depth_frame.height,
			depth_frame.width * 2, reinterpret_cast<uint8_t*>(dep_buffer.data()),
			depth_frame.frame_data.size(), dummy, dummy);
		k4a::image transformed_color_img = camera_transform.color_image_to_depth_camera(depth_img, color_img);

		memcpy(warped_color_frame.frame_data.data(), transformed_color_img.get_buffer(), transformed_color_img.get_size());
	}
	
	/// map a depth value together with pixel indices to a 3D point with coordinates in meters; point_ptr needs to provide space for 3 floats
	bool rgbd_kinect_azure::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
	{
		// call function from sdk
		/*k4a_float2_t p;
		k4a_float3_t ray;
		int valid;

		p.xy.y = (float)y;
		p.xy.x = (float)x;

		k4a_calibration_2d_to_3d(
			&camera_calibration, &p, depth, K4A_CALIBRATION_TYPE_DEPTH, K4A_CALIBRATION_TYPE_DEPTH, &ray, &valid);

		if (valid)
		{
			point_ptr[0] = -1.f * 0.001 * ray.xyz.x;
			point_ptr[1] = 0.001 * ray.xyz.y;
			point_ptr[2] = 0.001 * ray.xyz.z;
		}
		else
		{
			point_ptr[0] = nanf("");
			point_ptr[1] = nanf("");
			point_ptr[2] = nanf("");
		}
		return true;*/
		// 
		auto& p = intrinsics->param;

		double fx_d = 1.0 / p.fx;
		double fy_d = 1.0 / p.fy;
		double cx_d = p.cx;
		double cy_d = p.cy;
		// set 0.001 for current vr_rgbd
		double d = 0.001 * depth * 1.000;
		double x_d = (x - cx_d) * fx_d;
		double y_d = (y - cy_d) * fy_d;

		float uv[2], xy[2];
		uv[0] = float(x_d);
		uv[1] = float(y_d);
		bool valid = true;
		
		if (transformation_unproject_internal(camera_calibration_t, uv, xy, valid)) {
			point_ptr[0] = float(-xy[0] * d);
			point_ptr[1] = float(xy[1] * d);
			point_ptr[2] = float(d);
			return true;
		}
		return false;
	}

	bool rgbd_kinect_azure::transformation_iterative_unproject(const float *uv, float *xy, bool valid, unsigned int max_passes) const
	{
		float Jinv[2 * 2];
		float best_xy[2] = {0.f, 0.f};
		float best_err = FLT_MAX;

		for (unsigned int pass = 0; pass < max_passes; pass++) {
			float p[2];
			float J[2 * 2];
			if (!transformation_project_internal(camera_calibration_t, xy, p, valid, J))
			{
				return K4A_RESULT_FAILED;
			}
			if (valid)
			{
				return K4A_RESULT_SUCCEEDED;
			}

			float err_x = uv[0] - p[0];
			float err_y = uv[1] - p[1];
			float err = err_x * err_x + err_y * err_y;
			if (err >= best_err) {
				xy[0] = best_xy[0];
				xy[1] = best_xy[1];
				break;
			}

			best_err = err;
			best_xy[0] = xy[0];
			best_xy[1] = xy[1];
			invert_2x2(J, Jinv);
			if (pass + 1 == max_passes || best_err < 1e-22f) {
				break;
			}

			float dx = Jinv[0] * err_x + Jinv[1] * err_y;
			float dy = Jinv[2] * err_x + Jinv[3] * err_y;

			xy[0] += dx;
			xy[1] += dy;
		}
		if (best_err > 1e-6f)
		{
			valid = true;
		}
		return K4A_RESULT_SUCCEEDED;
	}

	bool rgbd_kinect_azure::transformation_project_internal(const k4a_calibration_camera_t* camera_calibration_tt,
															const float xy[2], float point2d[2], bool valid,
															float J_xy[2 * 2]) const
	{
		auto* p = &intrinsics->param;
		float max_radius_for_projection = camera_calibration_tt->metric_radius;
		valid = false;

		float xp = xy[0] - p->codx;
		float yp = xy[1] - p->cody;

		float xp2 = xp * xp;
		float yp2 = yp * yp;
		float xyp = xp * yp;
		float rs = xp2 + yp2;
		float rm = max_radius_for_projection * max_radius_for_projection;
		if (rs > rm) {
			valid = true;
			return K4A_RESULT_SUCCEEDED;
		}
		float rss = rs * rs;
		float rsc = rss * rs;
		float a = 1.f + p->k1 * rs + p->k2 * rss + p->k3 * rsc;
		float b = 1.f + p->k4 * rs + p->k5 * rss + p->k6 * rsc;
		float bi;
		if (b != 0.f) {
			bi = 1.f / b;
		}
		else {
			bi = 1.f;
		}
		float d = a * bi;

		float xp_d = xp * d;
		float yp_d = yp * d;

		float rs_2xp2 = rs + 2.f * xp2;
		float rs_2yp2 = rs + 2.f * yp2;

		if (intrinsics_t->type == K4A_CALIBRATION_LENS_DISTORTION_MODEL_RATIONAL_6KT) {
			xp_d += rs_2xp2 * p->p2 + xyp * p->p1;
			yp_d += rs_2yp2 * p->p1 + xyp * p->p2;
		}
		else {
			// the only difference from Rational6ktCameraModel is 2 multiplier for the tangential coefficient term
			// xyp*p1 and xyp*p2
			xp_d += rs_2xp2 * p->p2 + 2.f * xyp * p->p1;
			yp_d += rs_2yp2 * p->p1 + 2.f * xyp * p->p2;
		}

		float xp_d_cx = xp_d + p->codx;
		float yp_d_cy = yp_d + p->cody;

		point2d[0] = xp_d_cx * p->fx + p->cx;
		point2d[1] = yp_d_cy * p->fy + p->cy;

		if (J_xy == 0) {
			return K4A_RESULT_SUCCEEDED;
		}

		// compute Jacobian matrix
		float dudrs = p->k1 + 2.f * p->k2 * rs + 3.f * p->k3 * rss;
		// compute d(b)/d(r^2)
		float dvdrs = p->k4 + 2.f * p->k5 * rs + 3.f * p->k6 * rss;
		float bis = bi * bi;
		float dddrs = (dudrs * b - a * dvdrs) * bis;

		float dddrs_2 = dddrs * 2.f;
		float xp_dddrs_2 = xp * dddrs_2;
		float yp_xp_dddrs_2 = yp * xp_dddrs_2;
		// compute d(u)/d(xp)
		if (intrinsics_t->type == K4A_CALIBRATION_LENS_DISTORTION_MODEL_RATIONAL_6KT) {
			J_xy[0] = p->fx * (d + xp * xp_dddrs_2 + 6.f * xp * p->p2 + yp * p->p1);
			J_xy[1] = p->fx * (yp_xp_dddrs_2 + 2.f * yp * p->p2 + xp * p->p1);
			J_xy[2] = p->fy * (yp_xp_dddrs_2 + 2.f * xp * p->p1 + yp * p->p2);
			J_xy[3] = p->fy * (d + yp * yp * dddrs_2 + 6.f * yp * p->p1 + xp * p->p2);
		}
		else {
			J_xy[0] = p->fx * (d + xp * xp_dddrs_2 + 6.f * xp * p->p2 + 2.f * yp * p->p1);
			J_xy[1] = p->fx * (yp_xp_dddrs_2 + 2.f * yp * p->p2 + 2.f * xp * p->p1);
			J_xy[2] = p->fy * (yp_xp_dddrs_2 + 2.f * xp * p->p1 + 2.f * yp * p->p2);
			J_xy[3] = p->fy * (d + yp * yp * dddrs_2 + 6.f * yp * p->p1 + 2.f * xp * p->p2);
		}
		return K4A_RESULT_SUCCEEDED;
	}

	void rgbd_kinect_azure::invert_2x2(const float J[2 * 2], float Jinv[2 * 2]) const
	{
		float detJ = J[0] * J[3] - J[1] * J[2];
		float inv_detJ = 1.f / detJ;

		Jinv[0] = inv_detJ * J[3];
		Jinv[3] = inv_detJ * J[0];
		Jinv[1] = -inv_detJ * J[1];
		Jinv[2] = -inv_detJ * J[2];
	}

	bool rgbd_kinect_azure::transformation_unproject_internal(const k4a_calibration_camera_t* camera_calibration,
															  const float uv[2], float xy[2], bool valid) const
	{
		auto& p = camera_calibration->intrinsics.parameters.param;
		double xp_d = uv[0] - p.codx;
		double yp_d = uv[1] - p.cody;

		double r2 = xp_d * xp_d + yp_d * yp_d;
		double r4 = r2 * r2;
		double r6 = r2 * r4;
		double r8 = r4 * r4;
		double a = 1 + p.k1 * r2 + p.k2 * r4 + p.k3 * r6;
		double b = 1 + p.k4 * r2 + p.k5 * r4 + p.k6 * r6;
		double ai;
		if (a != 0.f) {
			ai = 1.f / a;
		}
		else {
			ai = 1.f;
		}
		float di = float(ai * b);
		// solve the radial and tangential distortion
		double x_u = xp_d * di;
		double y_u = yp_d * di;

		// approximate correction for tangential params
		float two_xy = float(2* x_u * y_u);
		float xx = float(x_u * x_u);
		float yy = float(y_u * y_u);

		x_u -= (yy + 3.f * xx) * p.p2 + two_xy * p.p1;
		y_u -= (xx + 3.f * yy) * p.p1 + two_xy * p.p2;

		x_u += p.codx;
		y_u += p.cody;

		xy[0] = float(x_u);
		xy[1] = float(y_u);
		return transformation_iterative_unproject(uv, xy, valid, 20);
	}

	bool rgbd_kinect_azure::get_emulator_configuration(emulator_parameters& parameters) const
	{
		parameters.depth_scale = 1.0;
		parameters.intrinsics = azure_color_intrinsics;
		return true;
	}

	void rgbd_kinect_azure::check_errors()
	{
		if (capture_thread_has_new_messages.load(std::memory_order_acquire)) {
			if (capture_thread_device_error) {
				capture_thread_broken = true;
				std::cerr << "rgbd_kinect_azure driver: exception occured in capture thread! \n" << capture_thread_device_error->what() << '\n';
				stop_device();
			}
		}
	}

	bool rgbd_kinect_azure::recover_from_errors()
	{
		assert(!is_running());
		capture_thread_has_new_messages.store(false, std::memory_order_relaxed);
		capture_thread_device_error = nullptr;
		//try recover
		std::string old_dev_serial = device_serial;
		detach();
		return attach(old_dev_serial);
	}

	auto extract_frame(const k4a::image& col, const rgbd::stream_format& color_format)
	{
		auto col_frame = make_unique<frame_type>();
		static_cast<frame_format&>(*col_frame) = color_format;
		col_frame->time = col.get_system_timestamp().count() * 0.000000001;
		col_frame->system_time_stamp = col.get_system_timestamp().count();
		col_frame->device_time_stamp = col.get_device_timestamp().count() * 1000;
		col_frame->frame_data.resize(col.get_size());
		col_frame->compute_buffer_size();
		memcpy(col_frame->frame_data.data(), col.get_buffer(), col.get_size());
		return col_frame;
	}

	void rgbd_kinect_azure::capture(int is)
	{
		while (is_running()) {
			try {
				k4a::capture cap;				
				device.get_capture(&cap);
				//device.get_color_control()
				unique_ptr<frame_type> col_frame, dep_frame, ired_frame;
				if (is & IS_COLOR)
					col_frame = extract_frame(cap.get_color_image(), color_format);
				if (is & IS_DEPTH)
					dep_frame = extract_frame(cap.get_depth_image(), depth_format);
				if (is & IS_INFRARED)
					ired_frame = extract_frame(cap.get_ir_image(), ir_format);
				/*
				//read IMU
				k4a_imu_sample_t imu_sample;
				if (imu_enabled) {
					switch (k4a_device_get_imu_sample(device.handle(), &imu_sample, -1))
					{
					case K4A_WAIT_RESULT_SUCCEEDED:
						break;
					case K4A_WAIT_RESULT_TIMEOUT:
						break;
					case K4A_WAIT_RESULT_FAILED:
						cerr << "Failed to read a imu sample\n";
						imu_enabled = false;
					}
				}
				*/

				capture_lock.lock();
				if (is & IS_COLOR) {
					color_frames[current_write_color_frame] = move(col_frame);
					current_write_color_frame = (current_write_color_frame + 1) % frame_cache_size;
					has_new_color_frame = true;
				}
				if (is & IS_DEPTH) {
					depth_frames[current_write_depth_frame] = move(dep_frame);
					current_write_depth_frame = (current_write_depth_frame + 1) % frame_cache_size;
					has_new_depth_frame = true;
				}
				if (is & IS_INFRARED) {
					ir_frames[current_write_ir_frame] = move(ired_frame);
					current_write_ir_frame = (current_write_ir_frame + 1) % frame_cache_size;
					has_new_ir_frame = true;
				}
				/*
				if (imu_enabled) {
					imu_data.angular_acceleration[0] = imu_sample.gyro_sample.v[0];
					imu_data.angular_acceleration[1] = imu_sample.gyro_sample.v[1];
					imu_data.angular_acceleration[2] = imu_sample.gyro_sample.v[2];

					imu_data.linear_acceleration[0] = imu_sample.acc_sample.v[0];
					imu_data.linear_acceleration[1] = imu_sample.acc_sample.v[1];
					imu_data.linear_acceleration[2] = imu_sample.acc_sample.v[2];
					imu_data.time_stamp = imu_sample.acc_timestamp_usec;
					has_new_IMU_data = true;
				}
				*/
				capture_lock.unlock();
			}
			catch (k4a::error err) {
				capture_thread_device_error = std::make_unique<k4a::error>(err);
				capture_thread_has_new_messages.store(true, std::memory_order_release);
				return; //most of the time the device is unusable after an error
			}
		}
	}
	// undistort
	void rgbd_kinect_azure::compute_xy_range(const k4a_calibration_t* calibration,
		const k4a_calibration_type_t camera,
		const int width,
		const int height,
		float& x_min,
		float& x_max,
		float& y_min,
		float& y_max)
	{
		// Step outward from the centre point until we find the bounds of valid projection
		const float step_u = 0.25f;
		const float step_v = 0.25f;
		const float min_u = 0;
		const float min_v = 0;
		const float max_u = (float)width - 1;
		const float max_v = (float)height - 1;
		const float center_u = 0.5f * width;
		const float center_v = 0.5f * height;

		int valid;
		k4a_float2_t p;
		k4a_float3_t ray;

		// search x_min
		for (float uv[2] = { center_u, center_v }; uv[0] >= min_u; uv[0] -= step_u)
		{
			p.xy.x = uv[0];
			p.xy.y = uv[1];
			k4a_calibration_2d_to_3d(calibration, &p, 1.f, camera, camera, &ray, &valid);

			if (!valid)
			{
				break;
			}
			x_min = ray.xyz.x;
		}

		// search x_max
		for (float uv[2] = { center_u, center_v }; uv[0] <= max_u; uv[0] += step_u)
		{
			p.xy.x = uv[0];
			p.xy.y = uv[1];
			k4a_calibration_2d_to_3d(calibration, &p, 1.f, camera, camera, &ray, &valid);

			if (!valid)
			{
				break;
			}
			x_max = ray.xyz.x;
		}

		// search y_min
		for (float uv[2] = { center_u, center_v }; uv[1] >= min_v; uv[1] -= step_v)
		{
			p.xy.x = uv[0];
			p.xy.y = uv[1];
			k4a_calibration_2d_to_3d(calibration, &p, 1.f, camera, camera, &ray, &valid);

			if (!valid)
			{
				break;
			}
			y_min = ray.xyz.y;
		}

		// search y_max
		for (float uv[2] = { center_u, center_v }; uv[1] <= max_v; uv[1] += step_v)
		{
			p.xy.x = uv[0];
			p.xy.y = uv[1];
			k4a_calibration_2d_to_3d(calibration, &p, 1.f, camera, camera, &ray, &valid);

			if (!valid)
			{
				break;
			}
			y_max = ray.xyz.y;
		}
	}

	pinhole_t rgbd_kinect_azure::create_pinhole_from_xy_range(const k4a_calibration_t* calibration, const k4a_calibration_type_t camera)
	{
		int width = calibration->depth_camera_calibration.resolution_width;
		int height = calibration->depth_camera_calibration.resolution_height;
		if (camera == K4A_CALIBRATION_TYPE_COLOR)
		{
			width = calibration->color_camera_calibration.resolution_width;
			height = calibration->color_camera_calibration.resolution_height;
		}

		float x_min = 0, x_max = 0, y_min = 0, y_max = 0;
		compute_xy_range(calibration, camera, width, height, x_min, x_max, y_min, y_max);

		pinhole_t pinhole;

		float fx = 1.f / (x_max - x_min);
		float fy = 1.f / (y_max - y_min);
		float px = -x_min * fx;
		float py = -y_min * fy;

		pinhole.fx = fx * width;
		pinhole.fy = fy * height;
		pinhole.px = px * width;
		pinhole.py = py * height;
		pinhole.width = width;
		pinhole.height = height;

		return pinhole;
	}

	void rgbd_kinect_azure::create_undistortion_lut(const k4a_calibration_t* calibration,
		const k4a_calibration_type_t camera,
		const pinhole_t* pinhole,
		k4a_image_t lut,
		interpolation_t type)
	{
		coordinate_t* lut_data = (coordinate_t*)(void*)k4a_image_get_buffer(lut);

		k4a_float3_t ray;
		ray.xyz.z = 1.f;

		int src_width = calibration->depth_camera_calibration.resolution_width;
		int src_height = calibration->depth_camera_calibration.resolution_height;
		if (camera == K4A_CALIBRATION_TYPE_COLOR)
		{
			src_width = calibration->color_camera_calibration.resolution_width;
			src_height = calibration->color_camera_calibration.resolution_height;
		}

		for (int y = 0, idx = 0; y < pinhole->height; y++)
		{
			ray.xyz.y = ((float)y - pinhole->py) / pinhole->fy;

			for (int x = 0; x < pinhole->width; x++, idx++)
			{
				ray.xyz.x = ((float)x - pinhole->px) / pinhole->fx;

				k4a_float2_t distorted;
				int valid;
				k4a_calibration_3d_to_2d(calibration, &ray, camera, camera, &distorted, &valid);

				coordinate_t src;
				if (type == INTERPOLATION_NEARESTNEIGHBOR)
				{
					// Remapping via nearest neighbor interpolation
					src.x = (int)floorf(distorted.xy.x + 0.5f);
					src.y = (int)floorf(distorted.xy.y + 0.5f);
				}
				else if (type == INTERPOLATION_BILINEAR || type == INTERPOLATION_BILINEAR_DEPTH)
				{
					// Remapping via bilinear interpolation
					src.x = (int)floorf(distorted.xy.x);
					src.y = (int)floorf(distorted.xy.y);
				}
				else
				{
					printf("Unexpected interpolation type!\n");
					exit(-1);
				}

				if (valid && src.x >= 0 && src.x < src_width && src.y >= 0 && src.y < src_height)
				{
					lut_data[idx] = src;

					if (type == INTERPOLATION_BILINEAR || type == INTERPOLATION_BILINEAR_DEPTH)
					{
						// Compute the floating point weights, using the distance from projected point src to the
						// image coordinate of the upper left neighbor
						float w_x = distorted.xy.x - src.x;
						float w_y = distorted.xy.y - src.y;
						float w0 = (1.f - w_x) * (1.f - w_y);
						float w1 = w_x * (1.f - w_y);
						float w2 = (1.f - w_x) * w_y;
						float w3 = w_x * w_y;

						// Fill into lut
						lut_data[idx].weight[0] = w0;
						lut_data[idx].weight[1] = w1;
						lut_data[idx].weight[2] = w2;
						lut_data[idx].weight[3] = w3;
					}
				}
				else
				{
					lut_data[idx].x = INVALID;
					lut_data[idx].y = INVALID;
				}
			}
		}
	}

	void rgbd_kinect_azure::remap(const k4a_image_t src, const k4a_image_t lut, k4a_image_t dst, interpolation_t type)
	{
		int src_width = k4a_image_get_width_pixels(src);
		int dst_width = k4a_image_get_width_pixels(dst);
		int dst_height = k4a_image_get_height_pixels(dst);

		uint16_t* src_data = (uint16_t*)(void*)k4a_image_get_buffer(src);
		uint16_t* dst_data = (uint16_t*)(void*)k4a_image_get_buffer(dst);
		coordinate_t* lut_data = (coordinate_t*)(void*)k4a_image_get_buffer(lut);

		memset(dst_data, 0, (size_t)dst_width * (size_t)dst_height * sizeof(uint16_t));

		for (int i = 0; i < dst_width * dst_height; i++)
		{
			if (lut_data[i].x != INVALID && lut_data[i].y != INVALID)
			{
				if (type == INTERPOLATION_NEARESTNEIGHBOR)
				{
					dst_data[i] = src_data[lut_data[i].y * src_width + lut_data[i].x];
				}
				else if (type == INTERPOLATION_BILINEAR || type == INTERPOLATION_BILINEAR_DEPTH)
				{
					const uint16_t neighbors[4]{ src_data[lut_data[i].y * src_width + lut_data[i].x],
												 src_data[lut_data[i].y * src_width + lut_data[i].x + 1],
												 src_data[(lut_data[i].y + 1) * src_width + lut_data[i].x],
												 src_data[(lut_data[i].y + 1) * src_width + lut_data[i].x + 1] };

					if (type == INTERPOLATION_BILINEAR_DEPTH)
					{
						// If the image contains invalid data, e.g. depth image contains value 0, ignore the bilinear
						// interpolation for current target pixel if one of the neighbors contains invalid data to avoid
						// introduce noise on the edge. If the image is color or ir images, user should use
						// INTERPOLATION_BILINEAR
						if (neighbors[0] == 0 || neighbors[1] == 0 || neighbors[2] == 0 || neighbors[3] == 0)
						{
							continue;
						}

						// Ignore interpolation at large depth discontinuity without disrupting slanted surface
						// Skip interpolation threshold is estimated based on the following logic:
						// - angle between two pixels is: theta = 0.234375 degree (120 degree / 512) in binning resolution
						// mode
						// - distance between two pixels at same depth approximately is: A ~= sin(theta) * depth
						// - distance between two pixels at highly slanted surface (e.g. alpha = 85 degree) is: B = A /
						// cos(alpha)
						// - skip_interpolation_ratio ~= sin(theta) / cos(alpha)
						// We use B as the threshold that to skip interpolation if the depth difference in the triangle is
						// larger than B. This is a conservative threshold to estimate largest distance on a highly slanted
						// surface at given depth, in reality, given distortion, distance, resolution difference, B can be
						// smaller
						const float skip_interpolation_ratio = 0.04693441759f;
						float depth_min = std::min(std::min(neighbors[0], neighbors[1]),
							std::min(neighbors[2], neighbors[3]));
						float depth_max = std::max(std::max(neighbors[0], neighbors[1]),
							std::max(neighbors[2], neighbors[3]));
						float depth_delta = depth_max - depth_min;
						float skip_interpolation_threshold = skip_interpolation_ratio * depth_min;
						if (depth_delta > skip_interpolation_threshold)
						{
							continue;
						}
					}

					dst_data[i] = (uint16_t)(neighbors[0] * lut_data[i].weight[0] + neighbors[1] * lut_data[i].weight[1] +
						neighbors[2] * lut_data[i].weight[2] + neighbors[3] * lut_data[i].weight[3] +
						0.5f);
				}
				else
				{
					printf("Unexpected interpolation type!\n");
					exit(-1);
				}
			}
		}
	}

	/// construct driver
	rgbd_kinect_azure_driver::rgbd_kinect_azure_driver()
	{
	}
	/// destructor
	rgbd_kinect_azure_driver::~rgbd_kinect_azure_driver()
	{
	}
	/// return the number of kinect devices found by driver
	unsigned rgbd_kinect_azure_driver::get_nr_devices()
	{
		return k4a::device::get_installed_count();
	}
	/// return the serial of the i-th kinect devices
	std::string rgbd_kinect_azure_driver::get_serial(int i)
	{
		std::string serial;
		query_serial(i, serial);
		return serial;
	}
	/// in case the rgbd device has a microphone or microphone array, return the id of the corresponding sound device (wsapi id under windows)
	std::string rgbd_kinect_azure_driver::get_audio_device_id(int i)
	{
		std::string serial;
		query_serial(i, serial);
		return query_audio_device_id(serial);
	}
	/// create a kinect device
	rgbd_device* rgbd_kinect_azure_driver::create_rgbd_device()
	{
		return new rgbd_kinect_azure();
	}
}
#include "lib_begin.h"

extern CGV_API rgbd::driver_registration<rgbd::rgbd_kinect_azure_driver> rgbd_kinect_azure_driver_registration("kinect_azure_driver");