#pragma once

#include "capture_format.h"

#include <vector>

#include "lib_begin.h"

namespace capture {

/// structure to specify a capability of a scalar device property
template <typename T>
struct scalar_capability
{
	/// default constructor initializes capability structure to an unsupported property
	scalar_capability() : is_supported(false), has_automatic_mode(false), has_manual_mode(false), default_value(0), min_value(0), max_value(0), granularity(0) {}
	/// whether a property is supported
	bool is_supported;
	/// whether the device has an automatic adjustment for a property
	bool has_automatic_mode;
	/// whether the device supports manual adjustment for a property
	bool has_manual_mode;
	/// default value of property
	T default_value;
	/// minimal value of property
	T min_value;
	/// maximal value of property
	T max_value;
	/// if granularity is > 0, valid property values are {min_value, min_value+granularity, min_value+2*granularity, ..., max_value}
	T granularity;
	/// if not empty, valid values are explicitly enumerated in this vector
	std::vector<T> valid_values;
	/// optionally the values can have names
	std::vector<std::string> valid_value_names;
};

/// structure to specify a capability of a device property that can assume different named enumerates
struct enum_capability
{
	/// to define a 
	struct enum_info
	{
		/// value of enum
		int value;
		/// name of enum
		std::string name;
		/// construct enumerate info
		enum_info(int _value = 0, const std::string& _name = "") : value(_value), name(_name) {}
	};
	/// default constructor initializes capability structure to an unsupported property
	enum_capability() : is_supported(false), has_automatic_mode(false), has_manual_mode(false), default_enum_index(0) {}
	/// whether a property is supported
	bool is_supported;
	/// whether the device has an automatic adjustment for a property
	bool has_automatic_mode;
	/// whether the device supports manual adjustment for a property
	bool has_manual_mode;
	/// index of default enum of property
	int default_enum_index;
	/// values and names of enums
	std::vector<enum_info> enums;
};

/// structure to store supported image sizes
struct image_size_capability
{
	/// construct with independent_dimensions defaulting to false
	image_size_capability() : independent_dimensions(false) {}
	/// whether width and height can be adjusted independently
	bool independent_dimensions;
	/// supported width values
	scalar_capability<int> width;
	/// supported height values
	scalar_capability<int> heigth;
};

/// structure to store supported image formats
struct image_format_capability : public image_size_capability
{
	/// supported pixel formats
	scalar_capability<PixelFormat> pixel_format;
};

struct image_stream_format_capability : public image_format_capability
{
	/// supported frame rates
	scalar_capability<float> fps;
	/// supported maximum data rate in bytes per second which can be used to compute the maximum supported frame rate for each image format
	float max_data_rate;
	/// return the maximum frame rate for a given image format
	float get_max_fps(const image_format& format) const { return float(max_data_rate / format.get_image_size()); }
};

/// structure to store capabilities of capture device
struct device_capabilities
{
	bool has_accelerometer;
	bool has_time_stamp_support;
	bool has_streaming_support;
	bool has_near_field_switch;
	bool has_backlight_compensation;
	bool has_live_view_support;
	bool has_bulb_shooting_support;
	bool has_triggered_shooting_support;
	bool has_sequence_shooting_support;

	InputStreams supported_input_streams;

	scalar_capability<float> pan;
	scalar_capability<float> tilt;
	scalar_capability<float> roll;
	scalar_capability<float> zoom;

	enum_capability ae_mode;
	enum_capability focus_mode;
	scalar_capability<float> focus;

	scalar_capability<float> aperture;
	scalar_capability<float> exposure;
	scalar_capability<float> gain;
	scalar_capability<float> white_balance;
	enum_capability power_line_frequency;

	scalar_capability<float> brightness;
	scalar_capability<float> contrast;
	scalar_capability<float> hue;
	scalar_capability<float> sharpness;
	scalar_capability<float> gamma;

	enum_capability quality;

	image_stream_format_capability color_image_formats;
	image_stream_format_capability infrared_image_formats;
	image_stream_format_capability depth_image_formats;
};

}

#include <cgv/config/lib_end.h>