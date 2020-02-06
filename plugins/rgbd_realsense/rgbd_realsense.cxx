#include <iostream>
#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>
#include "rgbd_realsense.h"

using namespace std;

namespace rgbd {


	rgbd_realsense::rgbd_realsense() {
		ctx = new rs2::context();
		dev = nullptr;
		pipe = nullptr;
		depth_scale = 0.001;
	}

	rgbd_realsense::~rgbd_realsense() {
		delete ctx;
		delete dev;
		delete pipe;
	}

	bool rgbd_realsense::attach(const std::string& serial)
	{
		if (is_attached()) {
			detach();
		}
		
		try {
			auto list = ctx->query_devices();
			for (unsigned i = 0; i < list.size(); ++i) {
				if (string(list[i].get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)).compare(serial) == 0) {
					dev = new rs2::device(list[i]);
					this->serial = serial;
					return true;
				}
			}
		}
		catch (const rs2::camera_disconnected_error& e)
		{
			cerr << "rgbd_realsense::attach: Camera was disconnected! Please connect it back" << endl;
		}
		catch (const std::runtime_error& e) {
			cerr << "get_serial: runtime error=" << e.what() << endl;
		}
		return false;
	}

	bool rgbd_realsense::is_attached() const
	{
		return dev != nullptr;
	}

	bool rgbd_realsense::detach()
	{
		if (this->is_attached()) {
			delete dev;
			dev = nullptr;
			serial = "";
			return true;
		}
		return false;
	}
	
	bool rgbd_realsense::check_input_stream_configuration(InputStreams is) const
	{
		static const unsigned streams_avaiable = IS_COLOR | IS_DEPTH | IS_INFRARED, streams_in = is;
		return (~(~streams_in | streams_avaiable)) == 0;
	}

	bool rgbd_realsense::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{
		if (!check_input_stream_configuration(is)) {
			cerr << "rgbd_realsense::start_device : invalid stream configuration\n";
			return false;
		}
		if (!is_attached()) {
			cerr << "rgbd_realsense::start_device : tried to start unattached device\n";
			return false;
		}
		if (is_running()) {
			return true;
		}

		
		if (is && IS_COLOR) {
			stream_formats.push_back(color_stream = stream_format(640, 480,PF_RGBA, 30, 32));
		}
		if (is && IS_DEPTH) {
			stream_formats.push_back(depth_stream = stream_format(640, 480, PF_DEPTH, 30, 16));
		}
		if (is && IS_INFRARED) {
			stream_formats.push_back(ir_stream = stream_format(640, 480, PF_I, 30, 8));
		}
		return this->start_device(stream_formats);
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
		
		for (const auto& format : stream_formats) {
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
			case PF_CONFIDENCE:
				rs2_pixel_format = RS2_FORMAT_Y16;
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
			//get the depth scaling for pointcloud generation
			auto sensors = dev->query_sensors();
			for (auto sensor : sensors) {
				if (sensor.is<rs2::depth_sensor>()) {
					depth_scale = sensor.as<rs2::depth_sensor>().get_depth_scale();
				}
			}
			//start the camera
			auto profile = pipe->start(cfg);
			return true;
		}
		cerr << "rgbd_realsense::start_device : can't resolve configuration, make sure to use the same ir stream and depth stream resolutions and framerates\n";
		delete pipe;
		pipe = nullptr;
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
		if (!is_running()) {
			cerr << "rgbd_realsense::get_frame called on device that is not running" << endl;
			return false;
		}
		if (!check_input_stream_configuration(is)) {
			cerr << "rgbd_realsense::get_frame called with an invalid input stream configuration" << endl;
			return false;
		}
		pipe->poll_for_frames(&frame_cache);

		if (frame_cache.size() > 0)
		{
			stream_format* stream = nullptr;

			rs2::frame next_frame;

			try {
				if (is == IS_COLOR) {
					if (next_frame = frame_cache.first(RS2_STREAM_COLOR)) {
						if (next_frame.get_timestamp() == last_color_frame_time) return false;
						stream = &color_stream;
						last_color_frame_time = next_frame.get_timestamp();
					}
				}
				else if (is == IS_DEPTH) {
					if (next_frame = frame_cache.first(RS2_STREAM_DEPTH)) {
						if (next_frame.get_timestamp() == last_depth_frame_time) return false;
						stream = &depth_stream;
						last_depth_frame_time = next_frame.get_timestamp();
					}
				}
				else if (is == IS_INFRARED) {
					if (next_frame = frame_cache.first(RS2_STREAM_INFRARED)) {
						if (next_frame.get_timestamp() == last_ir_frame_time) return false;
						stream = &ir_stream;
						last_ir_frame_time = next_frame.get_timestamp();
					}
				}
			}
			catch (const rs2::error &e){
				cerr <<  "rgbd_realsense::get_frame: " << e.what();
				switch (is) {
				case IS_COLOR:
					cerr << " color stream was not selected during start. please stop and start the device again with an color stream format\n";
					break;
				case IS_DEPTH:
					cerr << " depth stream was not selected during start. please stop and start the device again with an depth stream format\n";
					break;
				case IS_INFRARED:
					cerr << " infrared stream was not selected during start. please stop and start the device again with an ir stream format\n";
					break;
				}
				return false;
			}
			
			if (!stream) {
				return false;
			}
			
			static_cast<frame_format&>(frame) = *stream;
			frame.time = next_frame.get_timestamp();
			stream->compute_buffer_size();
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
		rs2::pipeline_profile profile = pipe->get_active_profile();
		auto color_stream = profile.get_stream(RS2_STREAM_COLOR);
		auto depth_stream = profile.get_stream(RS2_STREAM_DEPTH);
		rs2_extrinsics e = depth_stream.get_extrinsics_to(color_stream);
		rs2_intrinsics depth_intrinsics = depth_stream.as<rs2::video_stream_profile>().get_intrinsics();
		rs2_intrinsics color_intrinsics = color_stream.as<rs2::video_stream_profile>().get_intrinsics();

		static_cast<frame_size&>(warped_color_frame) = depth_frame;
		warped_color_frame.pixel_format = color_frame.pixel_format;
		warped_color_frame.nr_bits_per_pixel = color_frame.nr_bits_per_pixel;
		warped_color_frame.compute_buffer_size();
		warped_color_frame.frame_data.resize(warped_color_frame.buffer_size);
		unsigned bytes_per_pixel = color_frame.nr_bits_per_pixel / 8;

		float* color_coordinates = new float[depth_frame.height*depth_frame.width * 2];
		for (int y = 0; y < depth_frame.height; ++y) {
			for (int x = 0; x < depth_frame.width; ++x) {
				float depth = reinterpret_cast<const unsigned short*>(depth_frame.frame_data.data())[depth_frame.width*y + x];
				float color_pixel[2];
				float xyz[3];
				float color_xyz[3];
				float depth_pixel[2]{x,y};
				
				rs2_deproject_pixel_to_point(xyz, &depth_intrinsics, depth_pixel, depth_scale*depth);
				rs2_transform_point_to_point(color_xyz, &e, xyz);
				rs2_project_point_to_pixel(color_pixel, &color_intrinsics, color_xyz);

				memcpy(color_coordinates+2*(depth_frame.width*y+x), color_pixel, 2 * sizeof(float));
			}
		}
		
		// loop over each row and column of the color
		unsigned i = 0;
		char* dest = &warped_color_frame.frame_data.front();
		for (int y = 0; y < depth_frame.height; ++y) {
			for (int x = 0; x < depth_frame.width; ++x) {
				// retrieve the depth to color mapping for the current depth pixel
				int color_x = (int) color_coordinates[i++];
				int color_y = (int) color_coordinates[i++];

				static char zeros[8] = { 0,0,0,0,0,0,0,0 };
				const char* src = zeros;
				// make sure the depth pixel maps to a valid point in color space
				if (color_x >= 0 && color_x < color_frame.width && color_y >= 0 && color_y < color_frame.height)
					src = &color_frame.frame_data.front() + bytes_per_pixel * (color_x + color_frame.width*color_y);
				std::copy(src, src + bytes_per_pixel, dest);
				dest += bytes_per_pixel;
			}
		}

		delete[] color_coordinates;
	}

	bool rgbd_realsense::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
	{
		if (depth == 0)
			return false;
		//intrinisc camera matrix
		//1407.89876957815	0					0
		//0					1402.63035792400	0
		//975.564959401091	546.629684502803	1
		
		//rs300 intrinsics
		//1357.265516384099e 		0					0
		//0						1354.74750108174	0
		//963.179434235186		547.678492185045	1
		
		static const double fx_d = 1.0/1407.89876957815;
		static const double fy_d = 1.0/1402.63035792400;
		static const double cx_d = 975.564959401091;
		static const double cy_d = 546.629684502803;
		double d = depth*depth_scale;
		point_ptr[0] = float((x - cx_d) * d * fx_d);
		point_ptr[1] = float((y - cy_d) * d * fy_d);
		point_ptr[2] = float(d);
		return true;
	}

	void rgbd_realsense::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
		if (!is_attached()) {
			std::cerr << "rgbd_realsense::query_stream_formats:  device is not attached!\n";
			return;
		}
		//find stream formats by quering the sensors
		auto sensors = dev->query_sensors();
		vector<rs2::stream_profile> stream_profiles;
		for (auto sensor : sensors) {
			for (auto profile : sensor.get_stream_profiles()) {
				stream_profiles.push_back(profile);
			}
		}

		for (const rs2::stream_profile &profile : stream_profiles) {
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
				case RS2_FORMAT_Y16:
					stream.nr_bits_per_pixel = 16;
					stream.pixel_format = PF_CONFIDENCE;
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
		try {
			rs2::context ctx;
			auto list = ctx.query_devices();
			return list.size();
		}
		catch (runtime_error err) {
			cerr << "get_nr_devices: runtime error=" << err.what() << endl;
			return 0;
		}
	}

	std::string rgbd_realsense_driver::get_serial(int i) {
		try {
			rs2::context ctx;
			auto list = ctx.query_devices();
			int list_size = list.size();
			if (i >= list_size) return string();
			if (list[i].supports(RS2_CAMERA_INFO_SERIAL_NUMBER)) {
				return list[i].get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
			}
			return string();
		}
		catch (runtime_error err) {
			cerr << "get_serial: runtime error=" << err.what() << endl;
		}
		return string();
	}

	rgbd_device* rgbd_realsense_driver::create_rgbd_device() {
		return new rgbd_realsense();
	}
}
#include "lib_begin.h"

extern CGV_API rgbd::driver_registration<rgbd::rgbd_realsense_driver> rgbd_realsense_driver_registration("realsense_driver");
