/* This file is part of the cgv framework realsense driver
*  for use ensure that a diretory containing realsense2.dll is in the PATH variable else the plugin fails to load
*  e.g. C:\Program Files (x86)\Intel RealSense SDK 2.0\bin\x86
*  also make sure REALSENSE_DIR is set
*  e.g. C:\Program Files (x86)\Intel RealSense SDK 2.0
*/
#pragma once

#include <rgbd_input.h>
#include "rgbd_realsense.h"
#include "lib_begin.h"

namespace rgbd {
	/// realsense driver with post processing
	class CGV_API rgbd_realsense_ps : public rgbd_realsense
	{
	public:
		/// create a detached realsense device object
		rgbd_realsense_ps();
		~rgbd_realsense_ps();
		
		/// attach to the realsense device of the given serial, the expected serial is the same as returned by rgbd_realsense_driver::get_serial
		bool attach(const std::string& serial);
		/// check whether the device supports the given combination of input streams
		void query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const;
		bool start_device(InputStreams is, std::vector<stream_format>& stream_formats);
		/// start the rgbd device with given stream formats 
		bool start_device(const std::vector<stream_format>& stream_formats);
		/// query a frame of the given input stream
		bool get_frame(InputStreams is, frame_type& frame, int timeOut);
	private:
		int decimation_filter_intensity;
		rs2::temporal_filter temp_filter;
		rs2::decimation_filter dec_filter;
		rs2::disparity_transform depth2disparity_transform;
		rs2::disparity_transform disparity2depth_transform;
		rs2::spatial_filter spatial_filter;
		rs2::hole_filling_filter hole_filling_filter;
	};

	class CGV_API rgbd_realsense_ps_driver : public rgbd_driver
	{
	public:
		/// construct realsense driver
		rgbd_realsense_ps_driver();
		/// destructor
		~rgbd_realsense_ps_driver();
		/// return the number of realsense devices found by driver
		unsigned get_nr_devices();
		/// return the serial of the i-th realsense devices
		std::string get_serial(int i);
		/// create a realsense device
		rgbd_device* create_rgbd_device();
	};

}
#include <cgv/config/lib_end.h>