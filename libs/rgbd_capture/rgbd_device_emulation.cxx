#include "rgbd_device_emulation.h"
#include "rgbd_input.h"

#include <cgv/utils/file.h>

using namespace std;

namespace rgbd {

	rgbd_emulation::rgbd_emulation(const std::string& fn)
	{
		file_name = fn;
		flags = idx = 0;
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
	bool rgbd_emulation::start_device(InputStreams is)
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
	bool rgbd_emulation::get_frame(FrameFormat ff, void* data_ptr, int timeOut)
	{	
		unsigned flag = 2;
		if (ff < FF_DEPTH_RAW)
			flag = 1;
		if (flags & flag) {
			++idx;
			flags = flag;
		}
		else
			flags |= flag;

		string fn = compose_file_name(file_name, ff, idx);
		if (!cgv::utils::file::exists(fn)) {
			idx = 0;
			flags = 0;
			fn = compose_file_name(file_name, ff, 0);
		}
		if (!rgbd_input::read_frame(fn, data_ptr, get_frame_size(ff)))
			return false;
		return true;
	}
	void rgbd_emulation::map_depth_to_color_pixel(FrameFormat depth_ff, const void* depth_data_ptr, void* color_pixel_data_ptr) const
	{

	}
	void rgbd_emulation::map_color_to_depth(FrameFormat depth_ff, const void* depth_data_ptr, FrameFormat color_ff, void* color_data_ptr) const
	{

	}
	bool rgbd_emulation::map_pixel_to_point(int x, int y, unsigned depth, FrameFormat depth_ff, float point[3])
	{
		return false;
	}


}