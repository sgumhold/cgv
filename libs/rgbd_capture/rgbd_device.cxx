#include <iostream>
#include <cassert>
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
	/// standard computation of the buffer size member
	void frame_format::compute_buffer_size()
	{
		buffer_size = width * height * nr_bits_per_pixel / 8;
	}

	/// check whether frame data is allocated
	bool frame_type::is_allocated() const
	{
		return !frame_data.empty();
	}

	/// write to file
	bool frame_type::write(const std::string& fn) const
	{
		assert(buffer_size == frame_data.size());
		return
			cgv::utils::file::write(fn, reinterpret_cast<const char*>(this), sizeof(frame_format), false) &&
			cgv::utils::file::append(fn, &frame_data.front(), frame_data.size(), false);
	}

	/// read from file
	bool frame_type::read(const std::string& fn)
	{
		if (!cgv::utils::file::read(fn,
			reinterpret_cast<char*>(this),
			sizeof(frame_format), false))
			return false;
		frame_data.resize(buffer_size);
		return
			cgv::utils::file::read(fn,
				&frame_data.front(), buffer_size, false,
				sizeof(frame_format));
	}

	stream_format::stream_format(int w, int h, PixelFormat pf, float _fps, unsigned _nr_bits, unsigned _buffer_size)
	{
		width = w;
		height = h;
		pixel_format = pf;
		nr_bits_per_pixel = _nr_bits;
		buffer_size = _buffer_size;
		if (buffer_size == -1)
			compute_buffer_size();
		fps = _fps;
	}
	bool stream_format::operator == (const stream_format& sf) const
	{
		return
			width == sf.width &&
			height == sf.height &&
			pixel_format == sf.pixel_format &&
			nr_bits_per_pixel == sf.nr_bits_per_pixel &&
			fps == sf.fps;
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

	std::ostream& operator << (std::ostream& os, const frame_size& fs)
	{
		return os << fs.width << "x" << fs.height;
	}
	std::ostream& operator << (std::ostream& os, const frame_format& ff)
	{
		os << static_cast<const frame_size&>(ff) << "|";
		switch (ff.pixel_format) {
		case PF_I: os << "INF" << ff.nr_bits_per_pixel; break;
		case PF_DEPTH: os << "DEP" << ff.nr_bits_per_pixel; break;
		case PF_DEPTH_AND_PLAYER: os << "D+P" << ff.nr_bits_per_pixel; break;
		case PF_CONFIDENCE: os << "CNF" << ff.nr_bits_per_pixel; break;
		case PF_RGB: os << ((ff.nr_bits_per_pixel == 24) ? "RGB24" : "RGB32"); break;
		case PF_BGR: os << ((ff.nr_bits_per_pixel == 24) ? "BGR24" : "BGR32"); break;
		case PF_RGBA: os << "RGB32"; break;
		case PF_BGRA: os << "BGR32"; break;
		case PF_BAYER:os << "Bayer"; break;
		}
		return os;
	}
	std::ostream& operator << (std::ostream& os, const stream_format& sf)
	{
		return os << static_cast<const frame_format&>(sf) << ":" << sf.fps;
	}
}