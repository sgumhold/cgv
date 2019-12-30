#include <iostream>
#include <librealsense2/rs.hpp>
#include "rgbd_realsense.h"

using namespace std;

namespace rgbd {


	rgbd_realsense::rgbd_realsense() {
		ctx = new rs2::context();
		dev = nullptr;
		cfg = nullptr;
		pipe = nullptr;
	}

	rgbd_realsense::~rgbd_realsense() {
		delete ctx;
		delete dev;
		delete cfg;
		delete pipe;
	}

	bool rgbd_realsense::attach(const std::string& serial)
	{
		if (is_attached()) {
			detach();
		}
		
		auto list = ctx->query_devices();
		for (unsigned i = 0; i < list.size(); ++i) {
			if (string(list[i].get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)).compare(serial) == 0) {
				dev = new rs2::device(list[i]);
				this->serial = serial;
				return true;
			}
		}
		return false;
	}

	bool rgbd_realsense::is_attached() const
	{
		return dev != nullptr;
	}

	bool rgbd_realsense::detach()
	{
		delete dev;
		dev = nullptr;
		delete cfg;
		cfg = nullptr;
		serial = "";
		return true;
	}

	bool rgbd_realsense::check_input_stream_configuration(InputStreams is) const
	{
		return true;
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

		pipe = new rs2::pipeline(*ctx);
		rs2::config cfg;
		cfg.enable_device(serial);
		
		if (is && IS_COLOR) {
			cfg.enable_stream(RS2_STREAM_COLOR, -1, 640, 480, RS2_FORMAT_BGR8, 30);
			stream_formats.push_back(color_stream = stream_format(640, 480,PF_BGR, 30, 24));
		}
		if (is && IS_DEPTH) {
			cfg.enable_stream(RS2_STREAM_DEPTH, -1, 640, 480, RS2_FORMAT_Z16, 30);
			stream_formats.push_back(depth_stream = stream_format(640, 480, PF_DEPTH, 30, 16));
		}
		if (is && IS_INFRARED) {
			cfg.enable_stream(RS2_STREAM_INFRARED, -1, 640, 480, RS2_FORMAT_Y8, 30);
			stream_formats.push_back(ir_stream = stream_format(640, 480, PF_I, 30, 8));
		}
		if (cfg.can_resolve(*pipe)) {
			got_color_frame = got_depth_frame = got_ir_frame = false;
			auto profile = pipe->start(cfg);
			return true;
		}
		
		return false;
	}

	bool rgbd_realsense::start_device(const std::vector<stream_format>& stream_formats)
	{
		if (!is_attached()) {
			cerr << "rgbd_realsense::start_device : tried to start unattached device\n";
			return false;
		}
		if (is_running()) {
			return true;
		}

		pipe = new rs2::pipeline(*ctx);
		rs2::config cfg;
		cfg.enable_device(serial);

		for (auto format : stream_formats) {
			rs2_stream rs2_stream_type = RS2_STREAM_ANY;
			rs2_format rs2_pixel_format = RS2_FORMAT_ANY;
			switch (format.pixel_format) {
			case PF_RGB: {
				if (format.nr_bits_per_pixel == 24) {
					rs2_pixel_format = RS2_FORMAT_RGB8;
					rs2_stream_type = RS2_STREAM_COLOR;
				}
				break;
			}
			case PF_BGR: {
				if (format.nr_bits_per_pixel == 24) {
					rs2_pixel_format = RS2_FORMAT_BGR8;
					rs2_stream_type = RS2_STREAM_COLOR;
				}
				break;
			}
			case PF_BGRA:
				rs2_pixel_format = RS2_FORMAT_BGRA8;
				rs2_stream_type = RS2_STREAM_COLOR;
				break;
			case PF_RGBA:
				rs2_pixel_format = RS2_FORMAT_RGBA8;
				rs2_stream_type = RS2_STREAM_COLOR;
				break;
			case PF_DEPTH:
				rs2_pixel_format = RS2_FORMAT_Z16;
				rs2_stream_type = RS2_STREAM_DEPTH;
				break;
			case PF_I:
				rs2_pixel_format = RS2_FORMAT_Y8;
				rs2_stream_type = RS2_STREAM_INFRARED;
				break;
			}

			if (rs2_pixel_format == RS2_FORMAT_ANY) {
				cerr << "rgbd_realsense::start_device : stream_formats contains an unsupported stream format\n";
				continue;
			}
			switch (rs2_stream_type) {
			case RS2_STREAM_COLOR:
				color_stream = format;
				break;
			case RS2_STREAM_DEPTH:
				depth_stream = format;
				break;
			case RS2_STREAM_INFRARED:
				ir_stream = format;
				break;
			}
			cfg.enable_stream(rs2_stream_type, -1, format.width, format.height, rs2_pixel_format, format.fps);
		}

		if (cfg.can_resolve(*pipe)) {
			got_color_frame = got_depth_frame = got_ir_frame = false;
			auto profile = pipe->start(cfg);
			return true;
		}
		return false;
	}

	bool rgbd_realsense::stop_device()
	{
		if (is_running()) {
			pipe->stop();
			delete pipe;
			pipe = nullptr;
			return true;
		}

		return false;
	}

	bool rgbd_realsense::is_running() const
	{
		return pipe != nullptr;
	}

	bool rgbd_realsense::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{
		if (pipe->poll_for_frames(&frame_cache)) {
			got_color_frame = got_depth_frame = got_ir_frame = false;
		}
		if (frame_cache.size() > 0)
		{
			stream_format* stream = nullptr;
			rs2_stream rs_stream = RS2_STREAM_ANY;

			if (is == IS_COLOR && !got_color_frame) {
				stream = &color_stream;
				rs_stream = RS2_STREAM_COLOR;
				got_color_frame = true;
			} else if (is == IS_DEPTH && !got_depth_frame) {
				stream = &depth_stream;
				rs_stream = RS2_STREAM_DEPTH;
				got_depth_frame = true;
			} else if (is == IS_INFRARED && !got_ir_frame) {
				stream = &ir_stream;
				rs_stream = RS2_STREAM_INFRARED;
				got_ir_frame = true;
			}

			if (!stream) {
				return false;
			}

			rs2::frame next_frame = frame_cache.first(rs_stream);
				static_cast<frame_format&>(frame) = *stream;
				if (frame.frame_data.size() != stream->buffer_size) {
					frame.frame_data.resize(stream->buffer_size);
				}
				memcpy(frame.frame_data.data(), next_frame.get_data(), stream->buffer_size);
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
		auto sensors = dev->query_sensors();
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
				stream.fps = (float) profile.fps();
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
				stream.fps = (float) profile.fps();
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
				stream.fps = (float) profile.fps();
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