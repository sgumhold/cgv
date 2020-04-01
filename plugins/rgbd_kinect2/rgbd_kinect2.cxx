#include <iostream>
#include <chrono>
#include "rgbd_kinect2.h"
#include <Windows.h>
#include <Kinect.h>
#include <Ole2.h>
#include <cgv/utils/convert.h>

using namespace std;
using namespace chrono;


namespace rgbd {

	rgbd_kinect2::rgbd_kinect2() : infrared_reader(nullptr),color_reader(nullptr),depth_reader(nullptr)
	{
		camera = nullptr;
	}

	/// attach to the kinect device of the given serial
	bool rgbd_kinect2::attach(const std::string& serial)
	{
		if (serial.compare("default-kinect2") != 0) {
			return false;
		}
		if (is_attached()) {
			detach();
		}

		if (FAILED(GetDefaultKinectSensor(&camera))) {
			cerr << "rgbd_kinect2::attach: failed to get default kinect sensor\n";
			return false;
		}
		return true;
	}

	/// return whether device object is attached to a kinect device
	bool rgbd_kinect2::is_attached() const
	{
		return camera != 0;
	}
	/// detach from serial
	bool rgbd_kinect2::detach()
	{
		camera->Release();
		camera = nullptr;
		return true;
	}

	/// check whether the device supports the given combination of input streams
	bool rgbd_kinect2::check_input_stream_configuration(InputStreams is) const
	{
		return true;
	}

	/// query the stream formats available for a given stream configuration
	void rgbd_kinect2::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
		if ((is & IS_COLOR) != 0) {
			stream_formats.push_back(stream_format(1920, 1080, PF_BGRA, 30, 32));
		}
		if ((is & IS_INFRARED) != 0) {
			stream_formats.push_back(stream_format(512, 424, PF_I, 30, 16));
		}
		if ((is & IS_DEPTH) != 0) {
			stream_formats.push_back(stream_format(512, 424, PF_DEPTH, 30, 16));
		}
	}
	/// start the camera
	bool rgbd_kinect2::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{
		if (is & IS_COLOR) {
			stream_formats.push_back(stream_format(1920, 1080, PF_BGRA, 30, 32));
		}
		if (is & IS_INFRARED) {
			stream_formats.push_back(stream_format(512, 424, PF_I, 30, 16));
		}
		if (is & IS_DEPTH) {
			stream_formats.push_back(stream_format(512, 424, PF_DEPTH, 30, 16));
		}
		start_device(stream_formats);
		return true;
	}
	/// start the rgbd device with given stream formats 
	bool rgbd_kinect2::start_device(const std::vector<stream_format>& stream_formats)
	{
		if (!is_attached()) {
			cerr << "rgbd_kinect2: tried to start an unattached device!\n";
		}
		if (is_running())
			return true;
		
		
		if (FAILED(camera->Open())) {
			cerr << "rgbd_kinect2::start_device: failed to open kinect sensor\n";
		}


		IDepthFrameSource* depth_frame_source;
		IColorFrameSource* color_frame_source;
		IInfraredFrameSource* infrared_frame_source;
		if (FAILED(camera->get_ColorFrameSource(&color_frame_source))) {
			cerr << "rgbd_kinect2::start_device: failed to create frame reader\n";
		}
		camera->get_DepthFrameSource(&depth_frame_source);
		camera->get_InfraredFrameSource(&infrared_frame_source);
		color_frame_source->OpenReader(&color_reader);
		depth_frame_source->OpenReader(&depth_reader);
		infrared_frame_source->OpenReader(&infrared_reader);
		color_frame_source->Release();
		depth_frame_source->Release();
		infrared_frame_source->Release();

		for (auto& format : stream_formats) {
			switch (format.pixel_format) {
			case PF_BGRA:
				color_format = format;
				break;
			case PF_DEPTH:
				depth_format = format;
				break;
			case PF_I:
				ir_format = format;
				break;
			}
		}

		return true;
	}
	/// stop the camera
	bool rgbd_kinect2::stop_device()
	{
		if (is_running()) {
			color_reader->Release();
			color_reader = nullptr;
			depth_reader->Release();
			depth_reader = nullptr;
			infrared_reader->Release();
			infrared_reader = nullptr;
			camera->Close();
			return true;
		}
		return false;
	}

	/// return whether device has been started
	bool rgbd_kinect2::is_running() const
	{
		return color_reader != 0;
	}

		/// query a frame of the given input stream
	bool rgbd_kinect2::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{
		if (!is_attached()) {
			cerr << "rgbd_kinect2::get_frame called on device that has not been attached" << endl;
			return false;
		}

		if (!is_running()) {
			cerr << "rgbd_kinect2::get_frame called on device that is not running" << endl;
			return false;
		}

		double time = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();

		if (is == IS_COLOR) {
			IColorFrame* kinect_frame;
			if (SUCCEEDED(color_reader->AcquireLatestFrame(&kinect_frame))) {
				static_cast<frame_format&>(frame) = color_format;
				frame.time = time;
				//frame.compute_buffer_size();
				if (frame.frame_data.size() != frame.buffer_size) {
					frame.frame_data.resize(frame.buffer_size);
				}
				HRESULT hr = kinect_frame->CopyConvertedFrameDataToArray(frame.buffer_size, reinterpret_cast<BYTE*>(frame.frame_data.data()), ColorImageFormat_Bgra);
				if (FAILED(hr)) {
					cerr << "rgbd_kinect2::get_frame failed to copy data into frame buffer\n";
					return false;
				}
				
				kinect_frame->Release();
				return true;
			}
		}
		else if (is == IS_DEPTH) {
			IDepthFrame* kinect_frame;
			if (SUCCEEDED(depth_reader->AcquireLatestFrame(&kinect_frame))) {
				static_cast<frame_format&>(frame) = depth_format;
				frame.time = time;
				frame.compute_buffer_size();
				if (frame.frame_data.size() != frame.buffer_size) {
					frame.frame_data.resize(frame.buffer_size);
				}
				HRESULT hr = kinect_frame->CopyFrameDataToArray(frame.buffer_size >> 1, reinterpret_cast<UINT16*>(frame.frame_data.data()));
				if (FAILED(hr)) {
					cerr << "rgbd_kinect2::get_frame failed to copy data into frame buffer\n";
					return false;
				}

				kinect_frame->Release();
				return true;
			}
		}
		else if (is == IS_INFRARED) {
			IInfraredFrame* kinect_frame;
			if (SUCCEEDED(infrared_reader->AcquireLatestFrame(&kinect_frame))) {
				static_cast<frame_format&>(frame) = ir_format;
				frame.time = time;
				frame.compute_buffer_size();
				if (frame.frame_data.size() != frame.buffer_size) {
					frame.frame_data.resize(frame.buffer_size);
				}
				
				HRESULT hr = kinect_frame->CopyFrameDataToArray(frame.buffer_size >> 1, reinterpret_cast<UINT16*>(frame.frame_data.data()));
				if (FAILED(hr)) {
					cerr << "rgbd_kinect2::get_frame failed to copy data into frame buffer\n";
					return false;
				}
				kinect_frame->Release();
				return true;
			}
		}


		return false;
	}
	/// map a color frame to the image coordinates of the depth image
	void rgbd_kinect2::map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
		frame_type& warped_color_frame) const
	{		
		ICoordinateMapper* mapper;
		camera->get_CoordinateMapper(&mapper);
		vector<ColorSpacePoint> depth2color(depth_frame.width*depth_frame.height, ColorSpacePoint());     // Maps depth pixels to rgb pixels

		mapper->MapDepthFrameToColorSpace(
			depth_frame.width*depth_frame.height, reinterpret_cast<const UINT16*>(depth_frame.frame_data.data()),        // Depth frame data and size of depth frame
			depth_frame.width*depth_frame.height, depth2color.data()); // Output ColorSpacePoint array and size

		mapper->Release();
		
		static_cast<frame_size&>(warped_color_frame) = depth_frame;
		warped_color_frame.pixel_format = color_frame.pixel_format;
		warped_color_frame.nr_bits_per_pixel = color_frame.nr_bits_per_pixel;
		warped_color_frame.compute_buffer_size();
		warped_color_frame.frame_data.resize(warped_color_frame.buffer_size);
		unsigned bytes_per_pixel = color_frame.nr_bits_per_pixel / 8;

		char* dest = &warped_color_frame.frame_data.front();
		for (size_t y = 0; y < depth_frame.height; ++y) {
			for (size_t x = 0; x < depth_frame.width; ++x) {
				ColorSpacePoint cp = depth2color[y*depth_frame.width + x];
				
				int color_y = floor(cp.Y);
				int color_x = floor(cp.X);
				
				static char zeros[8] = { 0,0,0,0,0,0,0,0 };
				const char* src = zeros;

				if (color_x >= 0 && color_x < color_frame.width && color_y >= 0 && color_y < color_frame.height)
					src = &color_frame.frame_data.front() + bytes_per_pixel * (color_x + color_frame.width*color_y);
				std::copy(src, src + bytes_per_pixel, dest);
				dest += bytes_per_pixel;
			}
		}
		
	}
	
	/// map a depth value together with pixel indices to a 3D point with coordinates in meters; point_ptr needs to provide space for 3 floats
	bool rgbd_kinect2::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
	{
		//intrinisic matrix
		//1042.56333152221 	0					0
		//0 				1039.41616865874	0
		//964.559360889174	547.127316078404	1
		static const double fx_d = 1.0 / 1042.56333152221;
		static const double fy_d = 1.0 / 1039.41616865874;
		static const double cx_d = 964.559360889174;
		static const double cy_d = 547.127316078404;

		//double d = 0.001 * depth;
		double d = depth;
		point_ptr[0] = float((x - cx_d) * d * fx_d);
		point_ptr[1] = float((y - cy_d) * d * fy_d);
		point_ptr[2] = float(d);
		return true;
	}

	/// construct driver
	rgbd_kinect2_driver::rgbd_kinect2_driver()
	{
	}
	/// destructor
	rgbd_kinect2_driver::~rgbd_kinect2_driver()
	{
	}
	/// return the number of kinect devices found by driver
	unsigned rgbd_kinect2_driver::get_nr_devices()
	{
		IKinectSensor* sensor;

		if (FAILED(GetDefaultKinectSensor(&sensor))) {
			return 0;
		}
		sensor->Release();
		return 1;
	}

	/// return the serial of the i-th kinect devices
	std::string rgbd_kinect2_driver::get_serial(int i)
	{
		int n = get_nr_devices();
		if (i >= n)
			return "";

		IKinectSensor* sensor;

		if (FAILED(GetDefaultKinectSensor(&sensor))) {
			return "";
		}
		
		sensor->Release();
	
		return "default-kinect2";
	}

	/// create a kinect device
	rgbd_device* rgbd_kinect2_driver::create_rgbd_device()
	{
		return new rgbd_kinect2();
	}
}
#include "lib_begin.h"

extern CGV_API rgbd::driver_registration<rgbd::rgbd_kinect2_driver> rgbd_kinect2_driver_registration("kinect2_driver");