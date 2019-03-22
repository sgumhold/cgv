#include <iostream>
#include <algorithm>
#include "rgbd_device.h"
#include <cgv/utils/file.h>
#include <cgv/utils/convert.h>

using namespace std;

namespace rgbd {

	const char* get_frame_extension(FrameFormat ff)
	{
		static const char* exts[] = {
			"clr", "rgb24", "rgb32", "dep", "dep8", "dep12", "dep24"
		};
		return exts[ff];
	}

	string compose_file_name(const string& file_name, FrameFormat ff, unsigned idx)
	{
		string fn = file_name;
		fn += cgv::utils::to_string(idx);
		return fn + '.' + get_frame_extension(ff);
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

	unsigned rgbd_device::get_entry_size(FrameFormat ff) const
	{
		unsigned entry_size = 4;
		switch (ff) {
		case FF_COLOR_RAW:   entry_size = 4; break;
		case FF_COLOR_RGB24: entry_size = 3; break;
		case FF_DEPTH_RAW:   entry_size = 2; break;
		case FF_DEPTH_D8:    entry_size = 1; break;
		case FF_DEPTH_D12:   entry_size = 2; break;
		}
		return entry_size;
	}
	unsigned rgbd_device::get_frame_size(FrameFormat ff) const
	{
		return get_width()*get_height()*get_entry_size(ff);
	}
}