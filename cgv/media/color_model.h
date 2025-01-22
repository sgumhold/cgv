#pragma once

#include "lib_begin.h" //@<

namespace cgv { //@<
	namespace media { //@<

/// @>enum with the different color models, currently only LUM,RGB,HLS,XYZ are supported
enum /*CGV_API*/ ColorModel { LUM, RGB, HLS, XYZ, YUV, YIQ, Luv, Lab };

/// @>enum with the different alpha models
enum /*CGV_API*/ AlphaModel { NO_ALPHA, OPACITY, TRANSPARENCY, EXTINCTION };

	} //@<
} //@<

#include <cgv/config/lib_end.h> //@<