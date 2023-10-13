#pragma once
#include <string>

#include "lib_begin.h"

namespace holo_disp {
	//! lightweight class to represent calibration of holographic display
	struct CGV_API holographic_display_calibration
	{
		std::string serial = "LKG-APW92011";
		double screen_width = 3840.0;
		double screen_height = 2160.0;
		double DPI = 283.0;
		double pitch = 50.0693473815918;
		double slope = -7.521984100341797;
		double center = 0.07635253667831421;
		bool flip_x = false;
		bool flip_y = false;
		bool read(const std::string& fn = "visual.json");
	};
}

#include <cgv/config/lib_end.h>