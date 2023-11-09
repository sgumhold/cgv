#include "frame.h"
#include <cassert>
#include <iostream>
#include <cgv/utils/convert.h>
#include <cgv/utils/file.h>

using namespace std;

namespace rgbd {
	string get_frame_extension(const frame_format& ff)
	{
		static const char* exts[] = {
			"ir", "rgb", "bgr", "rgba", "bgra", "byr", "dep", "d_p", "p_tri"
		};
		return string(exts[ff.pixel_format]) + to_string(ff.nr_bits_per_pixel);
	}

	string compose_file_name(const string& file_name, const frame_format& ff, unsigned idx)
	{
		string fn = file_name;

		stringstream ss;
		ss << setfill('0') << setw(10) << idx;

		fn += ss.str();
		return fn + '.' + get_frame_extension(ff);
	}
	/// return number of bytes per pixel (ceil(nr_bits_per_pixel/8))
	unsigned frame_format::get_nr_bytes_per_pixel() const
	{
		return nr_bits_per_pixel / 8 + ((nr_bits_per_pixel & 7) == 0 ? 0 : 1);
	}
	/// standard computation of the buffer size member
	void frame_format::compute_buffer_size()
	{
		buffer_size = width * height * get_nr_bytes_per_pixel();
	}
	/// check whether frame data is allocated
	bool frame_type::is_allocated() const
	{
		return !frame_data.empty();
	}

	/// write to file
	bool frame_type::write(const string& fn) const
	{
		assert(buffer_size == frame_data.size());
		return
			cgv::utils::file::write(fn, reinterpret_cast<const char*>(this), sizeof(frame_format), false) &&
			cgv::utils::file::append(fn, &frame_data.front(), frame_data.size(), false);
	}

	/// read from file
	bool frame_type::read(const string& fn)
	{
		if (!cgv::utils::file::read(fn,
			reinterpret_cast<char*>(static_cast<frame_format*>(this)),
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
	ostream& operator << (ostream& os, const frame_size& fs)
	{
		return os << fs.width << "x" << fs.height;
	}
	ostream& operator << (ostream& os, const frame_format& ff)
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
	ostream& operator << (ostream& os, const stream_format& sf)
	{
		return os << static_cast<const frame_format&>(sf) << ":" << sf.fps;
	}

}