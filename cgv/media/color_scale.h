#pragma once

#include "color.h"

#include "lib_begin.h"

namespace cgv {
	namespace media {

enum ColorScale {
	CS_TEMPERATURE,
	CS_HUE,
	CS_HUE_LUMINANCE
};

/// compute an rgb color according to the selected color scale
extern CGV_API color<float,RGB> color_scale(double v, ColorScale cs = CS_TEMPERATURE);

	}
}

#include <cgv/config/lib_end.h>