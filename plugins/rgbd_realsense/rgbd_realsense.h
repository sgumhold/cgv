#pragma once

#include <rgbd_input.h>
#include "lib_begin.h"
#include <librealsense2/rs.hpp>

namespace rgbd {

	class CGV_API rgbd_realsense : public rgbd_device
	{
	public:
		rgbd_realsense();
		~rgbd_realsense();

		//not implemented yet

		
		bool attach(const std::string& serial);
		bool is_attached() const;
		bool detach();
		bool check_input_stream_configuration(InputStreams is) const;
		void query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const;
		bool start_device(InputStreams is, std::vector<stream_format>& stream_formats);
		bool start_device(const std::vector<stream_format>& stream_formats);
		bool stop_device();
		bool is_running() const;
		bool get_frame(InputStreams is, frame_type& frame, int timeOut);
		void map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
			frame_type& warped_color_frame) const;
		bool map_depth_to_point(int x, int y, int depth, float* point_ptr) const;

	protected:
		rs2::device dev;
	};

	class CGV_API rgbd_realsense_driver : public rgbd_driver
	{
	public:
		/// construct CLNUI driver
		rgbd_realsense_driver();
		/// destructor
		~rgbd_realsense_driver();
		/// return the number of realsense devices found by driver
		unsigned get_nr_devices();
		/// return the serial of the i-th realsense devices
		std::string get_serial(int i);
		/// create a kinect device
		rgbd_device* create_rgbd_device();
	protected:
		rs2::context ctx;
	};
}
#include <cgv/config/lib_end.h>