#pragma once

#include <rgbd_input.h>
#include <artec/sdk/base/TArrayRef.h>
#include "lib_begin.h"

namespace rgbd {

	class CGV_API rgbd_spider : public rgbd_device
	{
	public:
		/// create a detached realsense device object
		rgbd_spider();
		~rgbd_spider();
		
		/// attach to the realsense device of the given serial, the expected serial is the same as returned by rgbd_realsense_driver::get_serial
		bool attach(const std::string& serial);
		/// return whether device object is attached to a realsense device
		bool is_attached() const;
		/// detaches the object from the realsense device
		bool detach();
		/// check whether the device supports the given combination of input streams
		bool check_input_stream_configuration(InputStreams is) const;
		void query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const;
		bool start_device(InputStreams is, std::vector<stream_format>& stream_formats);
		/// start the rgbd device with given stream formats 
		bool start_device(const std::vector<stream_format>& stream_formats);
		/// stop the camera
		bool stop_device();
		/// return whether device has been started
		bool is_running() const;
		/// query a frame of the given input stream
		bool get_frame(InputStreams is, frame_type& frame, int timeOut);
		/// map a color frame to the image coordinates of the depth image
		void map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
			frame_type& warped_color_frame) const;
		/// map a depth value together with pixel indices to a 3D point with coordinates in meters; point_ptr needs to provide space for 3 floats
		bool map_depth_to_point(int x, int y, int depth, float* point_ptr) const;

	protected:
		std::string serial;
		artec::sdk::base::TRef<artec::sdk::capturing::IScanner> scanner;
		/// selected stream formats for color,depth and infrared
		stream_format color_stream, depth_stream, ir_stream;
		double last_color_frame_time, last_depth_frame_time, last_ir_frame_time;
	};

	class CGV_API rgbd_spider_driver : public rgbd_driver
	{
	public:
		/// construct CLNUI driver
		rgbd_spider_driver();
		/// destructor
		~rgbd_spider_driver();
		/// return the number of realsense devices found by driver
		unsigned get_nr_devices();
		/// return the serial of the i-th realsense devices
		std::string get_serial(int i);
		/// create a kinect device
		rgbd_device* create_rgbd_device();
	};

}
#include <cgv/config/lib_end.h>