#include "rgbd_device_emulation.h"
#include "rgbd_input.h"

#include <cgv/utils/file.h>
#include <iostream>
#include <fstream>

using namespace std;

namespace rgbd {

	rgbd_emulation::rgbd_emulation(const std::string& fn):device_is_running(false)
	{
		file_name = fn;
		flags = idx = 0;

		last_frame_time = chrono::high_resolution_clock::now();

		std::cout << "rgbd_emulation filename:" << fn << '\n';

		has_color_stream = false;
		has_depth_stream = false;
		has_ir_stream = false;

		const string suffix = "0000000000";

		//find first file of the color frames
		string fn_colorstream = fn + suffix + ".bgr32"; //currently only bgr32 exists
		ifstream file_colorstream = std::ifstream(fn_colorstream.c_str());


		//find first file of streams
		void* file = cgv::utils::file::find_first(fn+'*');
		
		if (file == nullptr) {
			cerr << "rgbd_emulation::rgbd_emulation: no frame files found for the prefix:" << fn << endl;
		}
		size_t file_count = 0; 
		while(file != nullptr) {
				string file_name = cgv::utils::file::find_name(file);
				string file_ext = cgv::utils::file::get_extension(file_name);
				size_t file_size = cgv::utils::file::find_size(file);

				if (file_ext.compare("bgr32") == 0 && !has_color_stream) {
					if (file_size == 1228800) {
						color_stream.width = 640;
						color_stream.height = 480;
					}
					else if (file_size == 4915200) {
						color_stream.width = 1280;
						color_stream.height = 960;
					}
					color_stream.fps = 30;
					color_stream.pixel_format = PixelFormat::PF_BGR;
					color_stream.nr_bits_per_pixel = 32;
					has_color_stream = true;
				}
				else if (file_ext.compare("dep16") == 0 && !has_depth_stream) {
					if (file_size == 614400) {
						depth_stream.width = 640;
						depth_stream.height = 480;
					}
					else if (file_size == 153600) {
						depth_stream.width = 320;
						depth_stream.height = 240;
					}
					else if (file_size == 9600) {
						depth_stream.width = 80;
						depth_stream.height = 60;
					}
					depth_stream.fps = 30;
					depth_stream.pixel_format = PixelFormat::PF_DEPTH;
					depth_stream.nr_bits_per_pixel = 16;
					has_depth_stream = true;
				}
			++file_count;
			file = cgv::utils::file::find_next(file);
		}
		number_of_frames = file_count;
	}

	bool rgbd_emulation::attach(const std::string& fn)
	{
		file_name = fn;
		flags = idx = 0;
		return true;
	}
	bool rgbd_emulation::is_attached() const
	{
		return !file_name.empty();
	}
	
	bool rgbd_emulation::detach()
	{
		file_name = "";
		return true;
	}
	
	bool rgbd_emulation::set_pitch(float y)
	{
		return true;
	}
	
	bool rgbd_emulation::has_IMU() const
	{
		return true;
	}
	const IMU_info& rgbd_emulation::get_IMU_info() const
	{
		static IMU_info info;
		info.has_angular_acceleration = true;
		info.has_linear_acceleration = true;
		info.has_time_stamp_support = false;
		return info;
	}
	bool rgbd_emulation::put_IMU_measurement(IMU_measurement& m, unsigned time_out) const
	{
		m.angular_acceleration[0] = m.angular_acceleration[1] = m.angular_acceleration[2] = 0.0f;
		m.linear_acceleration[0] = m.linear_acceleration[1] = m.linear_acceleration[2] = 0.0f;
		m.time_stamp = 0;
		return true;
	}
	
	bool rgbd_emulation::check_input_stream_configuration(InputStreams is) const
	{
		unsigned streams_avaiable = IS_NONE, streams_in = is;

		if (has_color_stream) {
			streams_avaiable |= IS_COLOR;
		}
		if (has_depth_stream) {
			streams_avaiable |= IS_DEPTH;
		}
		return (~(~streams_in | streams_avaiable)) == 0;
	}
	void rgbd_emulation::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
		if (has_color_stream && (is & IS_COLOR)) {
			stream_formats.push_back(color_stream);
		}
		if (has_depth_stream && (is & IS_DEPTH)) {
			stream_formats.push_back(depth_stream);
		}
	}
	bool rgbd_emulation::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{
		if (is_running())
			return true;

		if (!check_input_stream_configuration(is)) {
			cerr << "invalid input streams configuration!\n";
			return false;
		}
		stream_formats.resize(0);
		query_stream_formats(is, stream_formats);

		device_is_running = true;
		return true;
	}
	bool rgbd_emulation::start_device(const std::vector<stream_format>& stream_formats)
	{
		if (is_running())
			return true;
		device_is_running = true;
		return true;
	}
	bool rgbd_emulation::is_running() const
	{
		return device_is_running;
	}
	bool rgbd_emulation::stop_device()
	{
		device_is_running = false;
		return true;
	}
	unsigned rgbd_emulation::get_width(InputStreams is) const
	{
		if (!check_input_stream_configuration(is)) {
			cerr << "invalid input stream configuration";
			return 0;
		}
		unsigned w = 0;
		if (is & IS_COLOR) {
			w = static_cast<unsigned>(color_stream.width);
		}

		if (is & IS_DEPTH) {
			unsigned dw = static_cast<unsigned>(depth_stream.width);
			w = (w > dw) ? w : dw;
		}

		return w;
	}
	unsigned rgbd_emulation::get_height(InputStreams is) const
	{
		if (!check_input_stream_configuration(is)) {
			cerr << "invalid input stream configuration";
			return 0;
		}

		if (is & IS_COLOR) {
			return static_cast<unsigned>(color_stream.height);
		}

		if (is & IS_DEPTH) {
			return static_cast<unsigned>(depth_stream.height);
		}
	}
	bool rgbd_emulation::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{	
		if (!is_attached()) {
			cerr << "rgbd_emulation::get_frame called on device that has not been attached" << endl;
			return false;
		}

		if (!is_running()) {
			cerr << "rgbd_emulation::get_frame called on device that is not running" << endl;
			return false;
		}

		//get the stream information
		stream_format* stream = nullptr;
		
		switch (is) {
		case IS_COLOR:
			stream = &color_stream;
			break;
		case IS_DEPTH:
			stream = &depth_stream;
			break;
		}

		if (stream == nullptr) {
			cerr << "rgbd_emulation::get_frame: unsupported stream\n";
			return false;
		}

		auto inv_fps = chrono::duration_cast<chrono::high_resolution_clock::duration>(chrono::duration<double>( 1. / stream->fps ));
		auto current_frame_time = chrono::high_resolution_clock::now();
	
		//limit fps
		if (current_frame_time < last_frame_time+inv_fps) {
			return false;
		}
		last_frame_time = current_frame_time;

		//check index
		if (idx >= number_of_frames) idx = 0;
		frame.frame_index = idx;

		//copy frame_format data from stream
		static_cast<frame_format&>(frame) = *stream;

		string fn = compose_file_name(file_name, frame , idx);
		while (!cgv::utils::file::exists(fn)) {
			fn = compose_file_name(file_name, frame, idx);
			++idx;
		}

		frame.frame_index = idx;
		frame.time = idx / stream->fps;
		

		//read file
		string data;
		if (!cgv::utils::file::read(fn, data, false)) {
			cerr << "rgbd_emulation: file not found: " << fn << '\n';
			return false;
		}

		frame.buffer_size = data.size();
		if (frame.frame_data.size() < frame.buffer_size) {
			frame.frame_data.resize(frame.buffer_size);
		}
		memcpy(&frame.frame_data.front(), &data.front(), data.size());
		idx++;
		return true;
	}
	void rgbd_emulation::map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
		frame_type& warped_color_frame) const
	{
		std::cerr << "map_color_to_depth() not implemented in rgbd_emulation" << std::endl;
	}

	bool rgbd_emulation::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
	{
		std::cerr << "map_depth_to_point() not implemented in rgbd_emulation" << std::endl;
		return false;
	}

}