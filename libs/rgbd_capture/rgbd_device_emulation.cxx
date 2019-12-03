#include "rgbd_device_emulation.h"
#include "rgbd_input.h"

#include <cgv/utils/file.h>
#include <iostream>
#include <fstream>

using namespace std;

namespace rgbd {

	rgbd_emulation::rgbd_emulation(const std::string& fn)
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
			color_stream.pixel_format = PixelFormat::PF_BGRA;
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
		return true;
	}
	void rgbd_emulation::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{

	}
	bool rgbd_emulation::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{
		return true;
	}
	bool rgbd_emulation::start_device(const std::vector<stream_format>& stream_formats)
	{
		return true;
	}
	bool rgbd_emulation::is_running() const
	{
		return true; 
	}
	bool rgbd_emulation::stop_device()
	{
		return true;
	}
	unsigned rgbd_emulation::get_width(InputStreams) const
	{
		return 640;
	}
	unsigned rgbd_emulation::get_height(InputStreams) const
	{
		return 480;
	}
	bool rgbd_emulation::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{	
		string fn = compose_file_name(file_name, frame, frame.frame_index);
		if (!cgv::utils::file::exists(fn)) {
			idx = 0;
			fn = compose_file_name(file_name, frame, 0);
		}
//		if (!rgbd_input::read_frame(fn, data_ptr, get_frame_size(ff)))
//			return false;
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