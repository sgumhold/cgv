#pragma once

#include "capture_format.h"

#include "lib_begin.h"

namespace capture {

/// structure to store current device status
struct device_status
{
	float pan;
	float tilt;
	float roll;
	float zoom;

	bool near_field_mode;

	bool auto_exposure;
	bool auto_white_balance;
	bool backlight_compensation;
	bool live_view;

	int ae_mode_index;
	int focus_mode_index;
	float focus;

	float aperture;
	float exposure;
	float gain;
	float white_balance;
	int   power_line_frequency_index;

	float brightness;
	float contrast;
	float hue;
	float sharpness;
	float gamma;

	int quality_index;

	InputStreams running_input_streams;

	image_stream_format color_format;
	image_stream_format depth_format;
	image_stream_format infrared_format;
};

}

#include <cgv/config/lib_end.h>