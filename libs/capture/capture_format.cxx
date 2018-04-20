#include "capture_format.h"

namespace capture {

struct pixel_format_info
{
	PixelFormat pf;
	const std::string name;
	unsigned size;
	const char* cgv_format;
};

const pixel_format_info& ref_pixel_format_info(PixelFormat pf)
{
	static pixel_format_info infos[] = {
			{ PF_INFRARED8, "infrared 8", 1, "uint8:[I]" },
			{ PF_INFRARED16, "infrared 16", 2, "uint16:[I]" },
			{ PF_LONG_INFRARED8, "long infrared 8", 1, "uint8:[L]" },
			{ PF_LONG_INFRARED16, "long infrared 16", 2, "uint16:[L]" },

			{ PF_COLOR, "color", 4, "[R,G,B,A]" },
			{ PF_COLOR_RAW, "color raw", 4, "uint8:[R,G,B,A]" },
			{ PF_COLOR_BAYER, "color Bayer", 4, "uint8:[R,G,B,A]" },
			{ PF_COLOR_YUV, "color YUV", 3, "uint8:[Y,U,V]" },
			{ PF_COLOR_YUV2, "color YUV2", 3, "uint8:[Y,U,Y,V]" },
			{ PF_COLOR_RGB24, "color RGB24", 3, "uint8:[R,G,B]" },
			{ PF_COLOR_BGR24, "color BGR24", 3, "uint8:[B,G,R]" },
			{ PF_COLOR_RGB32, "color RGB32", 4, "uint8:[R,G,B,A]" },
			{ PF_COLOR_BGR32, "color BGR32", 4, "uint8:[B,G,R,A]" },

			{ PF_DEPTH, "depth", 2, "[D]" },
			{ PF_DEPTH_RAW, "depth_raw", 2, "uint16:[D]" },
			{ PF_DEPTH8, "depth8", 1, "uint8:[D]" },
			{ PF_DEPTH12, "depth12", 2, "uint12:[D]" },
			{ PF_DEPTH16, "depth16", 2, "uint16:[D]" },

			{ PF_RGBD, "rgbd", 4, "[R,G,B,D]" },
			{ PF_RGBD32, "rgbd32", 4, "uint8:[R,G,B,D]" }
	};
	return infos[pf];
};

/// convert pixel format to string
const std::string& to_string(PixelFormat pf)
{
	return ref_pixel_format_info(pf).name;
}

/// return pixel size in bytes
unsigned get_pixel_size(PixelFormat pf)
{
	return ref_pixel_format_info(pf).size;
}

}
