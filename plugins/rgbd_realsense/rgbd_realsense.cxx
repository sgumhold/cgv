#include "rgbd_realsense.h"

using namespace std;

namespace rgbd {

	rgbd_realsense::rgbd_realsense() {
		
	}

	rgbd_realsense::~rgbd_realsense() {
		
	}

	bool rgbd_realsense::attach(const std::string& serial)
	{
		detach();
		rs2::context ctx;
		auto list = ctx.query_devices();
		for (int i = 0; i < list.size(); ++i) {
			if (string(list[i].get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)).compare(serial) == 0) {
				dev = list[i];
				return true;
			}
		}
		return false;
	}

	bool rgbd_realsense::is_attached() const
	{
		return dev;
	}

	bool rgbd_realsense::detach()
	{
		dev = rs2::device();
		return true;
	}

	bool rgbd_realsense::check_input_stream_configuration(InputStreams is) const
	{
		return false;
	}

	bool rgbd_realsense::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{
		return false;
	}

	bool rgbd_realsense::start_device(const std::vector<stream_format>& stream_formats)
	{
		return false;
	}

	bool rgbd_realsense::stop_device()
	{
		return false;
	}

	bool rgbd_realsense::is_running() const
	{
		return false;
	}

	bool rgbd_realsense::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{
		return false;
	}

	void rgbd_realsense::map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame, frame_type& warped_color_frame) const
	{
	}

	bool rgbd_realsense::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
	{
		return false;
	}

	void rgbd_realsense::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
	}


	rgbd_realsense_driver::rgbd_realsense_driver() {
	}

	rgbd_realsense_driver::~rgbd_realsense_driver() {
	}

	unsigned rgbd_realsense_driver::get_nr_devices() {
		auto list = ctx.query_devices();
		return list.size();
	}

	std::string rgbd_realsense_driver::get_serial(int i) {
		auto list = ctx.query_devices();
		int list_size = get_nr_devices();
		if (i >= list_size) return "";
		
		return list[i].get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
	}

	rgbd_device* rgbd_realsense_driver::create_rgbd_device() {
		return new rgbd_realsense();
	}
}
#include "lib_begin.h"

extern CGV_API rgbd::driver_registration<rgbd::rgbd_realsense_driver> rgbd_realsense_driver_registration("realsense_driver");