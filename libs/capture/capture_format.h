#pragma once

#include <string>

#include "lib_begin.h"

namespace capture {

/// different input streams
enum InputStreams {
	IS_NONE = 0,
	IS_COLOR = 1,
	IS_DEPTH = 2,
	IS_INFRARED = 4,
	IS_PLAYER_INDEX = 8,
	IS_SKELETON = 16,

	IS_COLOR_AND_DEPTH = IS_COLOR | IS_DEPTH,
	IS_DEPTH_AND_PLAYER_INDEX = IS_DEPTH | IS_PLAYER_INDEX,

	IS_ALL = 31
};


/// different pixel formats
enum PixelFormat {
	PF_INFRARED8,
	PF_INFRARED16,
	PF_LONG_INFRARED8,
	PF_LONG_INFRARED16,

	PF_COLOR,
	PF_COLOR_RAW,
	PF_COLOR_BAYER,
	PF_COLOR_YUV,
	PF_COLOR_YUV2,
	PF_COLOR_RGB24,
	PF_COLOR_BGR24,
	PF_COLOR_RGB32,
	PF_COLOR_BGR32,

	PF_DEPTH,
	PF_DEPTH_RAW,
	PF_DEPTH8,
	PF_DEPTH12,
	PF_DEPTH16,

	PF_RGBD,
	PF_RGBD32,
};

/// convert pixel format to string
extern CGV_API const std::string& to_string(PixelFormat pf);

/// return pixel size in bytes
extern CGV_API unsigned get_pixel_size(PixelFormat pf);

/// struct to store the size of an image in pixels
struct image_size
{
	/// width of the image
	int width;
	/// height of the image
	int height;
};

/// struct to store the format of an image
struct image_format : public image_size
{
	/// pixel format of image
	PixelFormat pixel_format;

	/// return the number of bytes needed to store the image
	size_t get_image_size() const { return width*height*get_pixel_size(pixel_format);  }
};

/// struct to store the format of an image stream
struct image_stream_format : public image_format
{
	/// number of frames per second
	float fps;
	/// return the number of bytes per seconded needed to stream uncompressed video data
	size_t get_data_rate() const { return size_t(fps*get_image_size()); }
};

}

#include <cgv/config/lib_end.h>