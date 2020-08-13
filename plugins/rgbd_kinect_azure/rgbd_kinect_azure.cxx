#include <iostream>
#include "rgbd_kinect_azure.h"
#include <cgv/utils/convert.h>


using namespace std;
using namespace std::chrono;

namespace rgbd {
	/// create a detached kinect device object
	rgbd_kinect_azure::rgbd_kinect_azure()
	{
		device_serial = "";
		device_started = false;
		has_new_color_frame = has_new_depth_frame = has_new_ir_frame = false;
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

	/// return whether rgbd device has support for view finding actuator
	bool rgbd_kinect_azure::has_view_finder() const
	{
		return false;
	}
	/// return a view finder info structure, copy from rgbd_kinect.cxx
	const view_finder_info& rgbd_kinect_azure::get_view_finder_info() const
	{
		static view_finder_info info;
		info.degrees_of_freedom = 1;
		info.min_angle[0] = -27.0f;
		info.max_angle[0] =  27.0f;
		info.min_angle[1] = 0.0f;
		info.max_angle[1] = 0.0f;
		info.min_angle[2] = 0.0f;
		info.max_angle[2] = 0.0f;
		return info;
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
		info.has_angular_acceleration = false;
		info.has_linear_acceleration = true;
		info.has_time_stamp_support = false;
		return info;
	}
	/// query the current measurement of the acceleration sensors within the given time_out in milliseconds; return whether a measurement has been retrieved
	bool rgbd_kinect_azure::put_IMU_measurement(IMU_measurement& m, unsigned time_out) const
	{
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
		
		try { device.start_cameras(&cfg); }
		catch (runtime_error e) {
			cerr << e.what() << '\n';
			return false;
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
		//terminate capture thread
		if (capture_thread) {
			capture_thread->join();
		}
		capture_thread = nullptr;
		//stop device
		device.stop_cameras();
		return true;
	}

	/// return whether device has been started
	bool rgbd_kinect_azure::is_running() const
	{
		return device_started;
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
			frame = *color_frame;
			has_new_color_frame = false;
			capture_lock.unlock();
			return true;
		} else if (is == IS_DEPTH && has_new_depth_frame) {
			capture_lock.lock();
			frame = *depth_frame;
			has_new_depth_frame = false;
			capture_lock.unlock();
			return true;
		} else if (is == IS_INFRARED && has_new_ir_frame) {
			capture_lock.lock();
			frame = *ir_frame;
			has_new_ir_frame = false;
			capture_lock.unlock();
			return true;
		}

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
	}
	
	/// map a depth value together with pixel indices to a 3D point with coordinates in meters; point_ptr needs to provide space for 3 floats
	bool rgbd_kinect_azure::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
	{
		//data from rgbd_kinect.cxx
		depth /= 8;
		if (depth == 0)
			return false;

		static const double fx_d = 1.0 / 5.9421434211923247e+02;
		static const double fy_d = 1.0 / 5.9104053696870778e+02;
		static const double cx_d = 3.3930780975300314e+02;
		static const double cy_d = 2.4273913761751615e+02;
		double d = 0.001 * depth;
		point_ptr[0] = float((x - cx_d) * d * fx_d);
		point_ptr[1] = float((y - cy_d) * d * fy_d);
		point_ptr[2] = float(d);
		return true;
	}

	void rgbd_kinect_azure::capture(int is)
	{
		while (is_running()) {
			k4a::capture cap;
			device.get_capture(&cap);
			shared_ptr<frame_type> col_frame, dep_frame, ired_frame;

			if (is & IS_COLOR) {
				k4a::image col = cap.get_color_image();
				col_frame = make_shared<frame_type>();
				static_cast<frame_format&>(*col_frame) = color_format;
				col_frame->time = col.get_device_timestamp().count()*0.001;
				col_frame->frame_data.resize(col.get_size());
				col_frame->compute_buffer_size();
				memcpy(col_frame->frame_data.data(), col.get_buffer(), col.get_size());
			}

			if (is & IS_DEPTH) {
				k4a::image dep = cap.get_depth_image();
				dep_frame = make_shared<frame_type>();
				static_cast<frame_format&>(*dep_frame) = depth_format;
				dep_frame->time = dep.get_device_timestamp().count()*0.001;
				dep_frame->frame_data.resize(dep.get_size());
				dep_frame->compute_buffer_size();
				memcpy(dep_frame->frame_data.data(), dep.get_buffer(), dep.get_size());
			}

			if (is & IS_INFRARED) {
				k4a::image ir = cap.get_ir_image();
				ired_frame = make_shared<frame_type>();
				static_cast<frame_format&>(*ired_frame) = ir_format;
				ired_frame->time = ir.get_device_timestamp().count()*0.001;
				ired_frame->frame_data.resize(ir.get_size());
				ired_frame->compute_buffer_size();
				memcpy(ired_frame->frame_data.data(), ir.get_buffer(), ir.get_size());
			}

			capture_lock.lock();
			if (is & IS_COLOR) {
				color_frame = col_frame;
				has_new_color_frame = true;
			}
			if (is & IS_DEPTH) {
				depth_frame = dep_frame;
				has_new_depth_frame = true;
			}
			if (is & IS_INFRARED) {
				ir_frame = ired_frame;
				has_new_ir_frame = true;
			}
			capture_lock.unlock();
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