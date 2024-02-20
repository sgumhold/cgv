#pragma once

#include <string>

#include "lib_begin.h"

namespace cgv {
namespace g2d {

struct CGV_API shaders {
	static const std::string arrow;
	static const std::string background;
	static const std::string circle;
	static const std::string ring;
	static const std::string quadratic_spline;
	static const std::string cubic_spline;
	static const std::string ellipse;
	static const std::string grid;
	static const std::string line;
	static const std::string polygon;
	static const std::string quad;
	static const std::string rectangle;
};

}
}

#include <cgv/config/lib_end.h>
