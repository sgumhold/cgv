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
/// <returns>rgb color sampled from queried color scale</returns>
extern CGV_API color<float,RGB> color_scale(double value, ColorScale cs = CS_TEMPERATURE);

/// <summary>
/// register color samples as named color scale
/// </summary>
/// <param name="name">name of new color scale</param>
/// <param name="samples">vector of equidistant color samples</param>
extern CGV_API void register_named_color_scale(const ::std::string& name, const ::std::vector<color<float, RGB>>& samples);

/// <summary>
/// Return a timestamp that is increased every time a new color scale is registered in order to support checking whether 
/// a new call to query_color_scale_names() would give a different vector of names
/// </summary>
/// <returns>timestamp</returns>
extern CGV_API size_t get_named_color_scale_timestamp();

/// <summary>
/// return an enum definition string of the form "enums='red,green,...' that can be used for gui creation of 
/// </summary>
/// <returns></returns>
extern CGV_API const std::string& get_color_scale_enum_definition(bool include_fixed = true, bool include_named = true);

/// <summary>
/// Query names of all registered color scales.
/// The vector of names is cached internally such that multiple calles without a new 
/// color scale registration event will not need any computation time.
/// </summary>
/// <returns>vector with color scale names</returns>
extern CGV_API const std::vector<std::string>& query_color_scale_names();

/// <summary>
/// return const reference to vector of registered color samples of a named color scale
/// </summary>
/// <param name="name">name of queried color scale</param>
/// <returns>named color scale without any re-sampling</returns>
extern CGV_API const std::vector<color<float, RGB>>& query_named_color_scale(const std::string& name);

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
extern CGV_API color<float, RGB> sample_sampled_color_scale(float value, const std::vector<color<float, RGB>>& samples);


	}
}

#include <cgv/config/lib_end.h>