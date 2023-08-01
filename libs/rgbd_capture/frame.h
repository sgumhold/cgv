#pragma once

#include <string>
#include <vector>

#include "lib_begin.h"

namespace rgbd {
	/// frame size in pixels
	struct frame_size
	{
		/// width of frame in pixel
		int width;
		/// height of frame in pixel 
		int height;
	};

	/// format of individual pixels
	enum PixelFormat {
		PF_I, // infrared

		/* TODO: add color formats in other color spaces like YUV */

		PF_RGB,   // 24 or 32 bit rgb format with byte alignment
		PF_BGR,   // 24 or 24 bit bgr format with byte alignment
		PF_RGBA,  // 32 bit rgba format
		PF_BGRA,  // 32 bit brga format
		PF_BAYER, // 8 bit per pixel, raw bayer pattern values

		PF_DEPTH,
		PF_DEPTH_AND_PLAYER,
		PF_POINTS_AND_TRIANGLES,
		PF_CONFIDENCE
	};

	/// format of a frame
	struct CGV_API frame_format : public frame_size
	{
		/// format of pixels
		PixelFormat pixel_format;
		// total number of bits per pixel
		unsigned nr_bits_per_pixel;
		/// return number of bytes per pixel (ceil(nr_bits_per_pixel/8))
		unsigned get_nr_bytes_per_pixel() const;
		/// buffer size; returns width*height*get_nr_bytes_per_pixel()
		unsigned buffer_size;
		/// standard computation of the buffer size member
		void compute_buffer_size();
	};

	/// struct to store single frame
	struct frame_info : public frame_format
	{
		///
		unsigned frame_index;
		/// 
		double time;
		///
		long long system_time_stamp;
		///
		long long device_time_stamp;
	};
	/// struct to store single frame
	struct CGV_API frame_type : public frame_info
	{
		/// vector with RAW frame data 
		std::vector<char> frame_data;
		/// check whether frame data is allocated
		bool is_allocated() const;
		/// write to file
		bool write(const std::string& fn) const;
		/// read from file
		bool read(const std::string& fn);
	};

	/// steam format adds frames per second
	struct CGV_API stream_format : public frame_format
	{
		stream_format(int w = 640, int h = 480, PixelFormat pf = PF_RGB, float fps = 30, unsigned _nr_bits = 32, unsigned _buffer_size = -1);
		bool operator == (const stream_format& sf) const;
		float fps;
	};

	extern CGV_API std::ostream& operator << (std::ostream& os, const frame_size& fs);
	extern CGV_API std::ostream& operator << (std::ostream& os, const frame_format& ff);
	extern CGV_API std::ostream& operator << (std::ostream& os, const stream_format& sf);
}

#include <cgv/config/lib_end.h>