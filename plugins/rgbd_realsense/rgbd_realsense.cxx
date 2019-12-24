#include <iostream>
#include "rgbd_realsense.h"

using namespace std;

namespace rgbd {


	rgbd_realsense::rgbd_realsense() {
		ctx = make_shared<rs2::context>();
		dev = rs2::device();
		pipe = rs2::pipeline();
		cfg = rs2::config();
	}

	rgbd_realsense::~rgbd_realsense() {
		
	}

	bool rgbd_realsense::attach(const std::string& serial)
	{
		if (is_attached()) {
			detach();
		}
		
		auto list = ctx->query_devices();
		for (int i = 0; i < list.size(); ++i) {
			if (string(list[i].get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)).compare(serial) == 0) {
				dev = list[i];
				cfg = rs2::config();
				cfg.enable_device(serial);
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
		cfg.enable_device("");
		return true;
	}

	bool rgbd_realsense::check_input_stream_configuration(InputStreams is) const
	{
		return false;
	}

	bool rgbd_realsense::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{
		if (!is_attached()) {
			cerr << "rgbd_realsense::start_device : tried to start unattached device\n";
			return false;
		}
		if (is_running()) {
			return true;
		}
		
		pipe = rs2::pipeline(*ctx);
		if (is && IS_COLOR) {
			cfg.enable_stream(RS2_STREAM_COLOR, 0, 640, 480, RS2_FORMAT_BGR8, 30);
			stream_formats.push_back(color_stream = stream_format(640, 480,PF_BGR, 30, 24));
		}
		if (is && IS_DEPTH) {
			cfg.enable_stream(RS2_STREAM_DEPTH, 0, 640, 480, RS2_FORMAT_Z16, 30);
			stream_formats.push_back(depth_stream = stream_format(640, 480, PF_DEPTH, 30, 16));
		}
		if (is && IS_INFRARED) {
			cfg.enable_stream(RS2_STREAM_INFRARED, 0, 640, 480, RS2_FORMAT_Y8, 30);
			stream_formats.push_back(ir_stream = stream_format(640, 480, PF_I, 30, 8));
		}
		if (cfg.can_resolve(pipe)) {
			pipe.start(cfg);
			return true;
		}
		return false;
	}

	bool rgbd_realsense::start_device(const std::vector<stream_format>& stream_formats)
	{
		return false;
	}

	bool rgbd_realsense::stop_device()
	{
		if (is_running()) {
			pipe.stop();
			return true;
		}

		return false;
	}

	bool rgbd_realsense::is_running() const
	{
		return ;
	}

	bool rgbd_realsense::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{
		rs2::frameset frames;
		if (pipe.poll_for_frames(&frames))
		{
			if (is & IS_COLOR) {
				rs2::frame depth_frame = frames.first(RS2_STREAM_DEPTH);
				if (frame.frame_data.size() != color_stream.buffer_size) {
					frame.frame_data.resize(color_stream.buffer_size);
				}
				memcpy(frame.frame_data.data(), depth_frame.get_data(), color_stream.buffer_size);
				
			}
			return true;
		}
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
			if ((is & IS_COLOR) && 
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
			if ((is & IS_DEPTH) &&
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
			if ((is & IS_INFRARED) &&
				(profile.stream_type() == RS2_STREAM_INFRARED) && profile.is<rs2::video_stream_profile>()) {
				//add ir stream formats
				rs2::video_stream_profile video_stream = static_cast<rs2::video_stream_profile>(profile);
				stream_format stream;
				stream.fps = profile.fps();
				switch (profile.format()) {
				case RS2_FORMAT_Y8:
					stream.nr_bits_per_pixel = 8;
					stream.pixel_format = PF_I;
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
		rs2::context ctx;
		auto list = ctx.query_devices();
		return list.size();
	}

	std::string rgbd_realsense_driver::get_serial(int i) {
		rs2::context ctx;
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