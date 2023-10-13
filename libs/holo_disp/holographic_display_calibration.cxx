#include "holographic_display_calibration.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace holo_disp {
	bool holographic_display_calibration::read(const std::string& fn)
	{
		nlohmann::json j;
		std::ifstream is(fn);
		if (is.fail())
			return false;
		is >> j;
		j.at("serial").get_to(serial);
		j.at("pitch").at("value").get_to(pitch);
		j.at("slope").at("value").get_to(slope);
		j.at("center").at("value").get_to(center);
		j.at("DPI").at("value").get_to(DPI);
		j.at("screenW").at("value").get_to(screen_width);
		j.at("screenH").at("value").get_to(screen_height);
		double fx = 0.0, fy = 0.0;
		j.at("flipImageX").at("value").get_to(fx);
		j.at("flipImageY").at("value").get_to(fy);
		flip_x = fx != 0.0;
		flip_y = fy != 0.0;
		return true;
	}
}