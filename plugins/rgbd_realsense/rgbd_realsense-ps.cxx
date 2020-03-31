#include <iostream>
#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>
#include "rgbd_realsense-ps.h"
#include <librealsense2/rs_advanced_mode.hpp>
#include <cgv/utils/file.h>
#include "realsense-presets.h"

using namespace std;

namespace rgbd {


	rgbd_realsense_ps::rgbd_realsense_ps() {
		temp_filter = rs2::temporal_filter(0.25f, 43.f, 3);
		decimation_filter_intensity = 2;
		dec_filter = rs2::decimation_filter(decimation_filter_intensity);
		depth2disparity_transform = rs2::disparity_transform(true);
		disparity2depth_transform = rs2::disparity_transform(false);
		spatial_filter = rs2::spatial_filter(0.5, 20, 2, 0);
		hole_filling_filter = rs2::hole_filling_filter(1);
	}

	rgbd_realsense_ps::~rgbd_realsense_ps() {
	}

	bool rgbd_realsense_ps::attach(const std::string& serial)
	{
		if (is_attached()) {
			detach();
		}
		
		try {
			auto list = ctx->query_devices();
			for (unsigned i = 0; i < list.size(); ++i) {
				if ((string(list[i].get_info(RS2_CAMERA_INFO_SERIAL_NUMBER))+"-postfx").compare(serial) == 0) {
					dev = new rs2::device(list[i]);
					this->serial = serial;
					return true;
				}
			}
		}
		catch (const rs2::camera_disconnected_error& e)
		{
			cerr << e.what() << "\nrgbd_realsense::attach: Camera was disconnected! Please connect it back\n";
		}
		catch (const std::runtime_error& e) {
			cerr << "rgbd_realsense::attach: runtime error=" << e.what() << endl;
		}
		return false;
	}

	template<typename Container>
	stream_format* find_best_format(Container& supported_formats,const rgbd::PixelFormat pf,int fps) {
		sort(supported_formats.begin(), supported_formats.end(), [](const stream_format& a, const stream_format& b) {
			return a.height > b.height && a.width >= b.width; });
		auto it = find_if(supported_formats.begin(), supported_formats.end(), [&](stream_format format) {return format.pixel_format == pf && format.fps == fps; });
		if (it != supported_formats.end()) {
			return &(*it);
		}
		return nullptr;
	}

	bool rgbd_realsense_ps::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
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
		//enable high precision for Intel RealSense D435
		if (strcmp(dev->get_info(RS2_CAMERA_INFO_NAME), "Intel RealSense D435") == 0)
		{
			//advanced mode is required for loading presets
			auto advanced_mode_dev = dev->as<rs400::advanced_mode>();
			if (!advanced_mode_dev.is_enabled()) advanced_mode_dev.toggle_advanced_mode(true);
			advanced_mode_dev.load_json(highprec_preset);
		}


		if (is & IS_COLOR) {
			std::vector<stream_format> supported_formats;
			query_stream_formats(IS_COLOR, supported_formats);
			stream_format* f = find_best_format(supported_formats, PF_BGRA,30);
			if (f) { stream_formats.push_back(color_stream = *f); }
		}
		if (is & IS_DEPTH) {
			std::vector<stream_format> supported_formats;
			query_stream_formats(IS_DEPTH, supported_formats);
			stream_format* f = find_best_format(supported_formats, PF_DEPTH,30);
			if (f) { 
				stream_formats.push_back(depth_stream = *f); 
				if (is & IS_INFRARED) {
					std::vector<stream_format> supported_ir_formats;
					query_stream_formats(IS_INFRARED, supported_ir_formats);
					auto it = find_if(supported_ir_formats.begin(), supported_ir_formats.end(), [&](const stream_format& a) {
						return a.height == f->height && a.width == f->width && a.fps == f->fps; });
					stream_formats.push_back(ir_stream = *it);
				}
			}
		}
		else if (is & IS_INFRARED) {
			std::vector<stream_format> supported_formats;
			query_stream_formats(IS_INFRARED, supported_formats);
			stream_format* f = find_best_format(supported_formats, PF_I,30);
			if (f) { stream_formats.push_back(ir_stream = *f); }
		}
		return this->start_device(stream_formats);
	}

	bool rgbd_realsense_ps::start_device(const std::vector<stream_format>& stream_formats)
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
		cfg.enable_device(serial.substr(0,serial.find("-postfx")));
		int input_streams = IS_NONE;

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
				input_streams = input_streams|IS_COLOR;
				cfg.enable_stream(rs2_stream_type, -1, format.width, format.height, rs2_pixel_format, format.fps);
				break;
			case RS2_STREAM_DEPTH:
				depth_stream = format;
				input_streams = input_streams | IS_DEPTH;
				cfg.enable_stream(rs2_stream_type, -1, format.width*decimation_filter_intensity, format.height*decimation_filter_intensity, rs2_pixel_format, format.fps);
				break;
			case RS2_STREAM_INFRARED:
				ir_stream = format;
				input_streams = input_streams | IS_INFRARED;
				cfg.enable_stream(rs2_stream_type, -1, format.width, format.height, rs2_pixel_format, format.fps);
				break;
			}
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
			
			active_profile = pipe->get_active_profile();
			if ((input_streams & IS_COLOR_AND_DEPTH) == IS_COLOR_AND_DEPTH) {
				rs2_color_stream = active_profile.get_stream(RS2_STREAM_COLOR);
				rs2_depth_stream = active_profile.get_stream(RS2_STREAM_DEPTH);
				extrinsics_to_color_stream = rs2_depth_stream.get_extrinsics_to(rs2_color_stream);
				depth_intrinsics = rs2_depth_stream.as<rs2::video_stream_profile>().get_intrinsics();
				depth_intrinsics.ppx /= decimation_filter_intensity;
				depth_intrinsics.ppy /= decimation_filter_intensity;
				depth_intrinsics.height /= decimation_filter_intensity;
				depth_intrinsics.width /= decimation_filter_intensity;
				depth_intrinsics.fx /= decimation_filter_intensity;
				depth_intrinsics.fy /= decimation_filter_intensity;
				color_intrinsics = rs2_color_stream.as<rs2::video_stream_profile>().get_intrinsics();
			}
			return true;
		}
		cerr << "rgbd_realsense::start_device : can't resolve configuration, make sure to use the same ir stream and depth stream resolutions and framerates\n";
		delete pipe;
		pipe = nullptr;
		return false;
	}

	bool rgbd_realsense_ps::get_frame(InputStreams is, frame_type& frame, int timeOut)
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
						if (next_frame.get_frame_number() == last_color_frame_number) return false;
						stream = &color_stream;
						last_color_frame_number = next_frame.get_frame_number();
					}
				}
				else if (is == IS_DEPTH) {
					if (next_frame = frame_cache.first(RS2_STREAM_DEPTH)) {
						if (next_frame.get_frame_number() == last_depth_frame_number) return false;
						stream = &depth_stream;
						last_depth_frame_number = next_frame.get_frame_number();
						//post processing pipeline: decimation -> depth2disparity -> spatial -> temporal -> disparity2depth
						next_frame = dec_filter.process(next_frame);
						next_frame = depth2disparity_transform.process(next_frame);
						next_frame = spatial_filter.process(next_frame);
						next_frame = temp_filter.process(next_frame);
						next_frame = disparity2depth_transform.process(next_frame);
						//next_frame = hole_filling_filter.process(next_frame);
					}
				}
				else if (is == IS_INFRARED) {
					if (next_frame = frame_cache.first(RS2_STREAM_INFRARED)) {
						if (next_frame.get_frame_number() == last_ir_frame_number) return false;
						stream = &ir_stream;
						last_ir_frame_number = next_frame.get_frame_number();
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
			frame.time = next_frame.get_timestamp(); //time in milliseconds
			stream->compute_buffer_size();
			if (frame.frame_data.size() != stream->buffer_size) {
				frame.frame_data.resize(stream->buffer_size);
			}
			memcpy(frame.frame_data.data(), next_frame.get_data(), stream->buffer_size);
			return true;
		}
		return false;
	}


	void rgbd_realsense_ps::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
		if (!is_attached()) {
			std::cerr << "rgbd_realsense::query_stream_formats:  device is not attached!\n";
			return;
		}
		//find stream formats by querying the sensors
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
					stream.pixel_format = PF_CONFIDENCE; //using PF_CONFIDENCE here because it is the only color stream pixel format supporting 16 bit grayscale images
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
				//add depth stream formats
				rs2::video_stream_profile video_stream = static_cast<rs2::video_stream_profile>(profile);
				stream_format stream;
				stream.fps = (float) profile.fps();
				switch (profile.format()) {
				case RS2_FORMAT_Z16:
					stream.nr_bits_per_pixel = 16;
					stream.pixel_format = PF_DEPTH;
					stream.height = video_stream.height()/decimation_filter_intensity;
					stream.width = video_stream.width()/decimation_filter_intensity;
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


	rgbd_realsense_ps_driver::rgbd_realsense_ps_driver() {
		
	}

	rgbd_realsense_ps_driver::~rgbd_realsense_ps_driver() {
		
	}

	unsigned rgbd_realsense_ps_driver::get_nr_devices() {
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

	std::string rgbd_realsense_ps_driver::get_serial(int i) {
		try {
			rs2::context ctx;
			auto list = ctx.query_devices();
			int list_size = list.size();
			if (i >= list_size) return string();
			if (list[i].supports(RS2_CAMERA_INFO_SERIAL_NUMBER)) {
				return string(list[i].get_info(RS2_CAMERA_INFO_SERIAL_NUMBER))+"-postfx";
			}
			return string();
		}
		catch (runtime_error err) {
			cerr << "get_serial: runtime error=" << err.what() << endl;
		}
		return string();
	}

	rgbd_device* rgbd_realsense_ps_driver::create_rgbd_device() {
		return new rgbd_realsense_ps();
	}
}
#include "lib_begin.h"

extern CGV_API rgbd::driver_registration<rgbd::rgbd_realsense_ps_driver> rgbd_realsense_ps_driver_registration("realsense_ps_driver");
