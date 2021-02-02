#include <iostream>
#include "rgbd_kinect_azure.h"
#include <cgv/utils/convert.h>


using namespace std;
using namespace std::chrono;

void dummy(void* buffer,void* context){}


//constexpr emulator_parameters

constexpr rgbd::camera_intrinsics azure_color_intrinsics = { 3.765539746314990e+02, 3.758362623002023e+02,6.971111133663950e+02,3.753833895962808e+02, 0.0, 768, 1366 };

namespace rgbd {
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
		//find device with given serial
		for (int i = 0; i < k4a::device::get_installed_count();++i) {
			string dev_serial;
			try {
				device = k4a::device::open(i);
				dev_serial = device.get_serialnum();

				if (dev_serial == serial) {
					this->device_serial = serial;
					return true;
				}
				device.close();
			}
			catch (runtime_error e) {
				cerr << "rgbd_kinect_azure_driver: " << e.what() << '\n';
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
		if (has_new_IMU_data) {
			capture_lock.lock();
			memcpy(&m, &imu_data, sizeof(imu_data));
			has_new_IMU_data = false;
			capture_lock.unlock();
			return true;
		}
		return false;
	}
	/// check whether the device supports the given combination of input streams
	bool rgbd_kinect_azure::check_input_stream_configuration(InputStreams is) const
	{
		return false;
	}
	/// query the stream formats available for a given stream configuration
	void rgbd_kinect_azure::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
		const static int fps[] = { 5,15,30 };
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
		cfg.wired_sync_mode = K4A_WIRED_SYNC_MODE_STANDALONE; //set syncronization mode to standalone
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
			frame = move(*color_frame);
			has_new_color_frame = false;
			capture_lock.unlock();
			return true;
		} else if (is == IS_DEPTH && has_new_depth_frame) {
			capture_lock.lock();
			frame = move(*depth_frame);
			has_new_depth_frame = false;
			capture_lock.unlock();
			return true;
		} else if (is == IS_INFRARED && has_new_ir_frame) {
			capture_lock.lock();
			frame = move(*ir_frame);
			has_new_ir_frame = false;
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
		/*static constexpr double fx_d = 1.0 / azure_color_intrinsics.fx;
		static constexpr double fy_d = 1.0 / azure_color_intrinsics.fy;
		static constexpr double cx_d = azure_color_intrinsics.cx;
		static constexpr double cy_d = azure_color_intrinsics.cy;*/
		double fx_d = 1.0 / intrinsics->param.fx;
		double fy_d = 1.0 / intrinsics->param.fy;
		double cx_d = intrinsics->param.cx;
		double cy_d = intrinsics->param.cy;
		// set 0.001 for current vr_rgbd
		double d = 0.001 * depth * 1.100;
		//solve the radial and tangential distortion
		double x_distorted = x * (1 + intrinsics->param.k1 * pow(intrinsics->param.metric_radius, 2.0) + intrinsics->param.k2 * pow(intrinsics->param.metric_radius, 4.0) + intrinsics->param.k3 * pow(intrinsics->param.metric_radius, 6.0));
		double y_distorted = y * (1 + intrinsics->param.k4 * pow(intrinsics->param.metric_radius, 2.0) + intrinsics->param.k5 * pow(intrinsics->param.metric_radius, 4.0) + intrinsics->param.k6 * pow(intrinsics->param.metric_radius, 6.0));
		x_distorted = x_distorted + 2.0 * intrinsics->param.p1 * x * y + intrinsics->param.p2 * (pow(intrinsics->param.metric_radius, 2.0) + 2 * pow(x, 2.0));
		y_distorted = y_distorted + 2.0 * intrinsics->param.p2 * x * y + intrinsics->param.p1 * (pow(intrinsics->param.metric_radius, 2.0) + 2 * pow(y, 2.0));
		point_ptr[0] = -1.f * float((x_distorted - cx_d) * d * fx_d);
		point_ptr[1] = float((y_distorted - cy_d) * d * fy_d);
		point_ptr[2] = float(d);
		return true;
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

	void rgbd_kinect_azure::capture(int is)
	{
		while (is_running()) {
			try {
				k4a::capture cap;
				
				device.get_capture(&cap);
				unique_ptr<frame_type> col_frame, dep_frame, ired_frame;

				if (is & IS_COLOR) {
					k4a::image col = cap.get_color_image();
					col_frame = make_unique<frame_type>();
					static_cast<frame_format&>(*col_frame) = color_format;
					col_frame->time = col.get_device_timestamp().count() * 0.001;
					col_frame->frame_data.resize(col.get_size());
					col_frame->compute_buffer_size();
					memcpy(col_frame->frame_data.data(), col.get_buffer(), col.get_size());
				}

				if (is & IS_DEPTH) {
					k4a::image dep;
					try {
						dep = cap.get_depth_image();
					}
					catch (k4a::error err) {
						std::cout << "Failed to get depth image" << std::endl;
					}
					if (depth_image = dep.handle())
					{
						pinhole = create_pinhole_from_xy_range(&camera_calibration, K4A_CALIBRATION_TYPE_DEPTH);

						if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM,
							pinhole.width,
							pinhole.height,
							pinhole.width * (int)sizeof(coordinate_t),
							&lut)) {
							std::cout << "Failed to create lut" << std::endl;
						};

						create_undistortion_lut(&camera_calibration, K4A_CALIBRATION_TYPE_DEPTH, &pinhole, lut, interpolation_type);

						if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16,
							pinhole.width,
							pinhole.height,
							pinhole.width * (int)sizeof(uint16_t),
							&undistorted))
						{
							std::cout << "Failed to create undistorted" << std::endl;
						}
						remap(depth_image, lut, undistorted, interpolation_type);
						try {
							dep = k4a::image::image(depth_image);
						}
						catch (k4a::error err) {
							std::cout << "Failed to create k4a::image::image(depth_image)" << std::endl;
						}
						dep_frame = make_unique<frame_type>();
						static_cast<frame_format&>(*dep_frame) = depth_format;
						dep_frame->time = dep.get_device_timestamp().count() * 0.001;
						dep_frame->frame_data.resize(dep.get_size());
						dep_frame->compute_buffer_size();
						memcpy(dep_frame->frame_data.data(), dep.get_buffer(), dep.get_size());
					}
				}

				if (is & IS_INFRARED) {
					k4a::image ir = cap.get_ir_image();
					ired_frame = make_unique<frame_type>();
					static_cast<frame_format&>(*ired_frame) = ir_format;
					ired_frame->time = ir.get_device_timestamp().count() * 0.001;
					ired_frame->frame_data.resize(ir.get_size());
					ired_frame->compute_buffer_size();
					memcpy(ired_frame->frame_data.data(), ir.get_buffer(), ir.get_size());
				}


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


				capture_lock.lock();
				if (is & IS_COLOR) {
					color_frame = move(col_frame);
					has_new_color_frame = true;
				}
				if (is & IS_DEPTH) {
					depth_frame = move(dep_frame);
					has_new_depth_frame = true;
				}
				if (is & IS_INFRARED) {
					ir_frame = move(ired_frame);
					has_new_ir_frame = true;
				}

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
		k4a::device device;

		try {
			device = k4a::device::open(i);
			string serial = device.get_serialnum();
			device.close();
			return serial;
		}
		catch (runtime_error e) {
			cerr << "rgbd_kinect_azure_driver: " << e.what() << '\n';
		}
		return string();
	}

	/// create a kinect device
	rgbd_device* rgbd_kinect_azure_driver::create_rgbd_device()
	{
		return new rgbd_kinect_azure();
	}
}
#include "lib_begin.h"

extern CGV_API rgbd::driver_registration<rgbd::rgbd_kinect_azure_driver> rgbd_kinect_azure_driver_registration("kinect_azure_driver");