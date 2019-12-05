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

		std::cout << "rgbd_emulation filename:" << fn << '\n';

		has_color_stream = false;
		has_depth_stream = false;
		has_ir_stream = false;

		const string suffix = "0000000000";

		//find first file of the color frames
		string fn_colorstream = fn + suffix + ".bgr32"; //currently only bgr32 exists
		ifstream file_colorstream = std::ifstream(fn_colorstream.c_str());
		
		if (file_colorstream.good()) {
			//because the frames are saved as raw data metadata needs to be extracted from the file size and file extension
			size_t begin = file_colorstream.tellg();
			file_colorstream.seekg(0, ios::end);
			size_t end = file_colorstream.tellg();
			size_t fsize = (end - begin);
			if (fsize == 1228800) {
				color_stream.width = 640;
				color_stream.height = 480;
			}
			else if (fsize == 4915200) {
				color_stream.width = 1280;
				color_stream.height = 960;
			}
			color_stream.fps = 30;
			color_stream.pixel_format = PixelFormat::PF_BGR;
			color_stream.nr_bits_per_pixel = 32;
			has_color_stream = true;
		}

		//find first file of the depth frames
		string fn_depthstream = fn + suffix + ".dep16"; //currently only dep16 exists
		ifstream file_depthstream = std::ifstream(fn_depthstream.c_str());
		if (file_depthstream.good()) {
			//because the frames are saved as raw data metadata needs to be extracted from the file size and file extension
			size_t begin = file_depthstream.tellg();
			file_depthstream.seekg(0, ios::end);
			size_t end = file_depthstream.tellg();
			size_t fsize = (end - begin);
			if (fsize == 614400) {
				depth_stream.width = 640;
				depth_stream.height = 480;
			}
			else if (fsize == 153600) {
				depth_stream.width = 320;
				depth_stream.height = 240;
			} else if (fsize == 9600) {
				depth_stream.width = 80;
				depth_stream.height = 60;
			}
			depth_stream.fps = 30;
			depth_stream.pixel_format = PixelFormat::PF_DEPTH;
			depth_stream.nr_bits_per_pixel = 16;
			has_depth_stream = true;
		}
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
		if (!check_input_stream_configuration(is)) {
			cerr << "invalid input streams configuration!\n";
			return false;
		}
		query_stream_formats(is, stream_formats);

		device_is_running = true;
		return true;
	}
	bool rgbd_emulation::start_device(const std::vector<stream_format>& stream_formats)
	{
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
		unsigned w = 0;
		if (is & IS_COLOR) {
			w = static_cast<unsigned>(color_stream.height);
		}

		if (is & IS_DEPTH) {
			unsigned dw = static_cast<unsigned>(depth_stream.height);
			w = (w > dw) ? w : dw;
		}

		return w;
	}
	bool rgbd_emulation::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{	
		//get the stream information
		stream_format stream;
		if (is == IS_COLOR) {
			stream = color_stream;
		}

		if (is == IS_DEPTH) {
			stream = depth_stream;
		}

		//set frame metadata
		//frame.frame_index = idx;
		frame.nr_bits_per_pixel = stream.nr_bits_per_pixel;
		frame.pixel_format = stream.pixel_format;
		frame.width = stream.width;
		frame.height = stream.height;

		string fn = compose_file_name(file_name, frame, idx);
		if (!cgv::utils::file::exists(fn)) {
			idx = 0;
			fn = compose_file_name(file_name, frame, 0);
		}

//		if (!rgbd_input::read_frame(fn, data_ptr, get_frame_size(ff)))
//			return false;
		

		//read file
		string data;
		if (!cgv::utils::file::read(fn, data, false)) {
			cerr << "rgbd_emulation: file not found: " << fn << '\n';
			return false;
		}
		
		frame.frame_data = vector<char>(data.begin(), data.end());
		//frame.frame_index = idx;
		++idx;
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