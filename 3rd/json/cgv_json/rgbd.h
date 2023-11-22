#pragma once

// implementation of to_json and from_json helper functions for rgbd calibration
// please ensure cgv/libs is an include path

#include "math.h"
#include <rgbd_capture/rgbd_device.h>
#include "../nlohmann/json.hpp"

namespace rgbd {
	void to_json(nlohmann::json& j, const rgbd_calibration& c) {
		j["depth_scale"] = c.depth_scale;
		j["depth"] = c.depth;
		j["color"] = c.color;
	}
	void from_json(const nlohmann::json& j, rgbd_calibration& c) {
		j.at("depth_scale").get_to(c.depth_scale);
		j.at("depth").get_to(c.depth);
		j.at("color").get_to(c.color);
	}
}
