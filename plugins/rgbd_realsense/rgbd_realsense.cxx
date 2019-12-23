#include <iostream>
#include "rgbd_realsense.h"

using namespace std;

namespace rgbd {

	rs2::context& get_rs2_context() {
		static rs2::context ctx;
		return ctx;
	}

	rgbd_realsense::rgbd_realsense() {
		
	}

	rgbd_realsense::~rgbd_realsense() {
		
	}

	bool rgbd_realsense::attach(const std::string& serial)
	{
		if (is_attached()) {
			detach();
		}
		rs2::context& ctx = get_rs2_context();
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
		if (!is_attached()) {
			std::cerr << "rgbd_realsense::query_stream_formats:  device is not attached!\n";
			return;
		}
		auto sensors = dev.query_sensors();
		vector<rs2::stream_profile> stream_profiles;
		for (auto sensor : sensors) {
			for (auto profile : sensor.get_stream_profiles()) {
				stream_profiles.push_back(profile);
			}
		}

		for (rs2::stream_profile profile : stream_profiles) {
			if (((is & IS_COLOR) != 0) && 
					(profile.stream_type() == RS2_STREAM_COLOR) && profile.is<rs2::video_stream_profile>()) {
				//add color stream formats
				rs2::video_stream_profile video_stream = static_cast<rs2::video_stream_profile>(profile);
				stream_format stream;
				stream.fps = profile.fps();
				bool valid_format = false;
				switch (profile.format()) {
				case RS2_FORMAT_RGBA8:
					stream.nr_bits_per_pixel = 32;
					stream.pixel_format = PF_RGBA;
					valid_format = true;
					break;
				case RS2_FORMAT_BGRA8:
					stream.nr_bits_per_pixel = 32;
					stream.pixel_format = PF_BGRA;
					valid_format = true;
					break;
				case RS2_FORMAT_BGR8:
					stream.nr_bits_per_pixel = 24;
					stream.pixel_format = PF_BGR;
					valid_format = true;
					break;
				}
				if (valid_format) {
					stream.height = video_stream.height();
					stream.width = video_stream.width();
					stream_formats.push_back(stream);
				}
				continue;
			}
			if (((is & IS_DEPTH) != 0) &&
				(profile.stream_type() == RS2_STREAM_DEPTH) && profile.is<rs2::video_stream_profile>()) {
				//add ir stream formats
				rs2::video_stream_profile video_stream = static_cast<rs2::video_stream_profile>(profile);
				stream_format stream;
				stream.fps = profile.fps();
				switch (profile.format()) {
				case RS2_FORMAT_Z16:
					stream.nr_bits_per_pixel = 16;
					stream.pixel_format = PF_DEPTH;
					stream.height = video_stream.height();
					stream.width = video_stream.width();
					stream_formats.push_back(stream);
					break;
				}
				continue;
			}
			if (((is & IS_INFRARED) != 0) &&
				(profile.stream_type() == RS2_STREAM_INFRARED) && profile.is<rs2::video_stream_profile>()) {
				//add ir stream formats
				rs2::video_stream_profile video_stream = static_cast<rs2::video_stream_profile>(profile);
				stream_format stream;
				stream.fps = profile.fps();
				switch (profile.format()) {
				case RS2_FORMAT_Y8:
					stream.nr_bits_per_pixel = 8;
					stream.pixel_format = PF_DEPTH;
					stream.height = video_stream.height();
					stream.width = video_stream.width();
					stream_formats.push_back(stream);
					break;
				}
				continue;
			}
		}
	}


	rgbd_realsense_driver::rgbd_realsense_driver() {
	}

	rgbd_realsense_driver::~rgbd_realsense_driver() {
	}

	unsigned rgbd_realsense_driver::get_nr_devices() {
		rs2::context& ctx = get_rs2_context();
		auto list = ctx.query_devices();
		return list.size();
	}

	std::string rgbd_realsense_driver::get_serial(int i) {
		rs2::context& ctx = get_rs2_context();
		auto list = ctx.query_devices();
		int list_size = list.size();
		if (i >= list_size) return string();
		if (list[i].supports(RS2_CAMERA_INFO_SERIAL_NUMBER)) {
			return list[i].get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
		}
		return string();
	}

	rgbd_device* rgbd_realsense_driver::create_rgbd_device() {
		return new rgbd_realsense();
	}
}
#include "lib_begin.h"

extern CGV_API rgbd::driver_registration<rgbd::rgbd_realsense_driver> rgbd_realsense_driver_registration("realsense_driver");