#pragma once

#include "color.h"
#include <vector>
#include <string>

#include "lib_begin.h"

namespace cgv {
	namespace media {

/// <summary>
/// enum to index one of the fixed or named color scales
/// </summary>
enum ColorScale {
	CS_RED,   /// black to red
	CS_GREEN, /// black to green
	CS_BLUE,  /// black to blue
	CS_GRAY,  /// black to white
	CS_TEMPERATURE,   /// not perceptually optimized temperature color scale
	CS_HUE,           /// not perceptually optimized hue color scale
	CS_HUE_LUMINANCE, /// not perceptually optimized hue color scale where also luminance is varied from 0.25 to 0.75
	CS_NAMED          /// represents any of the named color scales
};

/// <summary>
/// compute an rgb color according to the selected color scale. In case of an index larger or equal
/// to CS_NAMED, the i-th named color scale is sampled
/// </summary>
/// <param name="value">value in [0,1] used to sample the color scale</param>
/// <param name="cs">color scale index</param>
/// <param name="polarity">0 ... query all named, 1 ... query unipolar only, 2 ... query bipolar only</param>
/// <returns>rgb color sampled from queried color scale</returns>
extern CGV_API color<float,RGB> color_scale(double value, ColorScale cs = CS_TEMPERATURE, int polarity = 0);

/// <summary>
/// perform a gamma mapping from [0,1] to [0,1] with optional accountance of window zero position in case of bipolar color scales
/// </summary>
/// <param name="v">to be mapped value</param>
/// <param name="gamma">gamma parameter</param>
/// <param name="is_bipolar">whether bipolar gamma mapping should be use</param>
/// <param name="window_zero_position">zero window position for bipolar gamma mapping</param>
/// <returns></returns>
extern CGV_API double color_scale_gamma_mapping(double v, double gamma, bool is_bipolar = false, double window_zero_position = 0.5);

/// <summary>
/// for the use of bipolar color maps this function can be used to adjust the value such that
/// the window position is mapped to the center of the bipolar color map. In a shader program
/// using color_scale.glsl this adjustment can be enabled by setting the uniform flag
/// map_window_zero_position_to_color_scale_center to true and specifying window_zero_position
/// with the float typed uniform of this name
/// </summary>
/// <param name="value">to be adjusted value</param>
/// <param name="window_zero_position">window position in the range [0,1] to which the 
/// attribute value zero is mapped</param>
/// <returns></returns>
extern CGV_API double adjust_zero_position(double value, double window_zero_position);

/// <summary>
/// register color samples as named color scale
/// </summary>
/// <param name="name">name of new color scale</param>
/// <param name="samples">vector of equidistant color samples</param>
/// <param name="is_bipolar">whether color scale is bipolar, where in case of even number of samples, interpolation is adapted to skip central interval producing a hard jump</param>
extern CGV_API void register_named_color_scale(const ::std::string& name, const ::std::vector<color<float, RGB>>& samples, bool is_bipolar = false);

/// <summary>
/// get name of color scale
/// </summary>
/// <param name="cs">color scale enum</param>
/// <returns>name</returns>
extern CGV_API std::string get_color_scale_name(ColorScale cs);

/// <param name="cs">color scale name</param>
/// <returns>name</returns>
/// 

/// <summary>
/// find color scale enum from name
/// </summary>
/// <param name="name">name of color scale</param>
/// <param name="cs">reference where to place resulting color scale enum</param>
/// <returns>where name is a valid color scale name</returns>
extern CGV_API bool find_color_scale(const std::string& name, ColorScale& cs);

/// <summary>
/// Return a timestamp that is increased every time a new color scale is registered in order to support checking whether 
/// a new call to query_color_scale_names() would give a different vector of names
/// </summary>
/// <returns>timestamp</returns>
extern CGV_API size_t get_named_color_scale_timestamp();

/// <summary>
/// return an enum definition string of the form "enums='red,green,...' that can be used for gui creation of 
/// </summary>
/// <param name="include_fixed">whether to include predefined color scales</param>
/// <param name="include_named">whether to include named color scales</param>
/// <param name="polarity">0 ... query all named, 1 ... query unipolar only, 2 ... query bipolar only</param>
/// <returns></returns>
extern CGV_API const std::string& get_color_scale_enum_definition(bool include_fixed = true, bool include_named = true, int polarity = 0);

/// <summary>
/// Query names of all registered color scales.
/// The vector of names is cached internally such that multiple calles without a new 
/// color scale registration event will not need any computation time.
/// </summary>
/// <param name="polarity">0 ... query all, 1 ... query unipolar only, 2 ... query bipolar only</param>
/// <returns>vector with color scale names</returns>
extern CGV_API const std::vector<std::string>& query_color_scale_names(int polarity = 0);

/// <summary>
/// return const reference to vector of registered color samples of a named color scale
/// </summary>
/// <param name="name">name of queried color scale</param>
/// <param name="is_bipolar_ptr">optional pointer to place flag whether queried color scale is bipolar</param>
/// <returns>named color scale without any re-sampling</returns>
extern CGV_API const std::vector<color<float, RGB>>& query_named_color_scale(const std::string& name, bool* is_bipolar_ptr = 0);

/// <summary>
/// computes a sampling of a named color scale referenced by name
/// </summary>
/// <param name="name">name a to be sampled color scale</param>
/// <param name="nr_samples">number of samples or 0 if function can propose best sampling</param>
/// <param name="exact">whether nr_samples parameter needs to be exactly fulfilled or if it is just a guideline</param>
/// <returns>vector of equidistantly sampled colors of the named color scale or empty vector if name is not found</returns>
extern CGV_API std::vector<color<float, RGB>> sample_named_color_scale(const std::string& name, size_t nr_samples = 0, bool exact = false);

/// <summary>
/// for given value in [0,1] compute a sample from a sampled color scale
/// </summary>
/// <param name="value">value in [0,1] which is clamped to [0,1] if it is outside of this range</param>
/// <param name="samples">color samples with first sample corresponding to value==0 and last to value==1</param>
/// <returns></returns>
extern CGV_API color<float, RGB> sample_sampled_color_scale(float value, const std::vector<color<float, RGB>>& samples, bool is_bipolar = false);


	}
}

#include <cgv/config/lib_end.h>