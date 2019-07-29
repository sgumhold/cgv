#include <iostream>
#include <algorithm>
#include "rgbd_device.h"
#include <cgv/utils/file.h>
#include <cgv/utils/convert.h>

using namespace std;

namespace rgbd {

	std::string get_frame_extension(const frame_format& ff)
	{
		static const char* exts[] = {
			"ir", "rgb", "bgr", "rgba", "bgra", "byr", "dep", "d_p"
		};
		return std::string(exts[ff.pixel_format]) + to_string(ff.nr_bits_per_pixel);
	}

	string compose_file_name(const string& file_name, const frame_format& ff, unsigned idx)
	{
		string fn = file_name;
		fn += cgv::utils::to_string(idx);
		return fn + '.' + get_frame_extension(ff);
	}

	stream_format::stream_format(int w, int h, PixelFormat pf, unsigned _nr_bits, unsigned _buffer_size, float _fps)
	{
		width = w;
		height = h;
		pixel_format = pf;
		nr_bits_per_pixel = _nr_bits;
		buffer_size = _buffer_size;
		fps = _fps;
	}

	/// virtual destructor
	rgbd_device::~rgbd_device()
	{
	}

	/// return whether rgbd device has support for view finding actuator
	bool rgbd_device::has_view_finder() const
	{
		return false;
	}
	/// return a view finder info structure
	const view_finder_info& rgbd_device::get_view_finder_info() const
	{
		static view_finder_info info;
		info.degrees_of_freedom = 0;
		info.max_angle[0] = info.max_angle[1] = info.max_angle[2] = 0;
		info.min_angle[0] = info.min_angle[1] = info.min_angle[2] = 0;
		return info;
	}
	/// set the pitch position of the rgbd device in degrees with 0 as middle position, return whether this was successful
	bool rgbd_device::set_pitch(float y)
	{
		return false;
	}
	/// return the pitch position of the rgbd device in degrees with 0 as middle position
	float rgbd_device::get_pitch() const
	{
		return 0;
	}

	/// whether rgbd device has support for a near field depth mode
	bool rgbd_device::has_near_mode() const
	{
		return false;
	}
	/// return whether the near field depth mode is activated
	bool rgbd_device::get_near_mode() const
	{
		return false;
	}
	/// activate or deactivate near field depth mode, return whether this was successful
	bool rgbd_device::set_near_mode(bool on)
	{
		return false;
	}

	bool rgbd_device::has_IMU() const
	{
		return false;
	}
	const IMU_info& rgbd_device::get_IMU_info() const
	{
		static IMU_info info;
		info.has_angular_acceleration = info.has_linear_acceleration = info.has_time_stamp_support = false;
		return info;
	}
	bool rgbd_device::put_IMU_measurement(IMU_measurement& m, unsigned time_out) const
	{
		return false;
	}
}