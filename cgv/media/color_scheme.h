#pragma once

#include <functional>
#include <map>
#include <set>
#include <cstring>
#include <vector>

#include <cgv/data/object_registry.h>
#include <cgv/math/interpolate.h>

#include "color.h"

#include "lib_begin.h"

namespace cgv {
namespace media {

/// @brief Convert a char array to a sequence of rgb colors. Input colors are encoded in hexadecimal with every six consecutive chars interpreted as a single rgb color.
/// 
/// @param data The encoded colors char array.
/// @return A sequence of decoded rgb colors.
static std::vector<cgv::rgb> to_colors(const char* data) {
	std::vector<cgv::rgb> colors;
	if(data) {
		size_t count = std::strlen(data) / 6;
		for(size_t i = 0; i < count; ++i) {
			std::string hex(data + 6 * i, 6);
			cgv::rgb color = { 0.0f };
			if(from_hex(hex, color))
				colors.push_back(color);
		}
	}
	return colors;
}

/// @brief Convert multiple char arrays to a multiple sequences of rgb colors. Input colors are encoded in hexadecimal with every six consecutive chars interpreted as a single rgb color.
/// See to_colors(const char* data).
/// 
/// @tparam N The number of input sequences.
/// @param data The std::array of encoded color char arrays.
/// @return The sequences of decoded rgb colors.
template<size_t N>
static std::vector<std::vector<cgv::rgb>> to_colors(const std::array<const char*, N>& data) {
	std::vector<std::vector<cgv::rgb>> colors;
	colors.reserve(N);
	for(const char* entry : data)
		colors.push_back(to_colors(entry));
	return colors;
}

/// @brief Enum class of possible color scheme types.
enum class ColorSchemeType {
	kUndefined,		// All others.
	kSequential,	// Continuously varying colors from a limited set of hues (typically 1 or 2). Typically used for ordered and quantitative data.
	kDiverging,		// Continuously varying colors with contrasting hues meeting at a neutral midpoint. Typically used when the data range covers a 'zero' point.
	kCyclical,		// Continuously varying colors where the start and end color match. Typically used when the lower and upper bound of the data range should be mapped to the same color.
	kCategorical	// Distinct colors with sharp boundaries. Typically used to represent unordered, discrete data categories.
};

/// @brief Base class for color schemes. Only holds color scheme type.
struct color_scheme {
	/// The type of this color scheme.
	ColorSchemeType type = ColorSchemeType::kUndefined;
};

/// @brief This class represents a continuous color scheme using an interpolator to convert continuous scalar values to colors.
class continuous_color_scheme : public color_scheme {
public:
	/// The used interpolator type.
	using interpolator_type = cgv::math::interpolator<cgv::rgb, float>;

	/// Construct using an interpolator that represents the identity function, basically providing a grayscale color ramp.
	continuous_color_scheme() : continuous_color_scheme(cgv::math::identity_interpolator<cgv::rgb, float>()) {}
	
	/// @brief Construct using the given interpolator and scheme type. The type should describe the color ramp produced by the interpolator if possible.
	/// 
	/// @param interpolator The used interpolator.
	/// @param type The color scheme type.
	continuous_color_scheme(const interpolator_type& interpolator, ColorSchemeType type = ColorSchemeType::kUndefined) {
		this->type = type;
		interpolator_ = interpolator.clone();
	}

	/// @brief Create a continuous_color_scheme using uniform linear interpolation of the given colors.
	/// 
	/// @param colors The list of colors to interpolate.
	/// @param type The color scheme type.
	/// @return The color scheme.
	static continuous_color_scheme linear(const std::vector<cgv::rgb>& colors, ColorSchemeType type = ColorSchemeType::kUndefined) {
		return { cgv::math::uniform_linear_interpolator<cgv::rgb, float>(colors), type };
	}

	/// @brief Create a continuous_color_scheme using linear interpolation of the given control points consisting of position and color.
	/// The positions should be within the range [0,1].
	///
	/// @param colors The list of control points to interpolate.
	/// @param type The color scheme type.
	/// @return The color scheme.
	static continuous_color_scheme linear(const std::vector<std::pair<float, cgv::rgb>>& colors, ColorSchemeType type = ColorSchemeType::kUndefined) {
		return { cgv::math::linear_interpolator<cgv::rgb, float>(colors), type };
	}

	/// @brief Create a continuous_color_scheme using uniform b-spline interpolation of the given colors.
	/// 
	/// @param colors The list of colors to interpolate.
	/// @param type The color scheme type.
	/// @return The color scheme.
	static continuous_color_scheme smooth(const std::vector<cgv::rgb>& colors, ColorSchemeType type = ColorSchemeType::kUndefined) {
		return { cgv::math::uniform_smooth_interpolator<cgv::rgb, float>(colors), type };
	}

	/// @brief Create a continuous_color_scheme using a functor as interpolator.
	/// 
	/// @param colors The function used to map scalars to colors.
	/// @param type The color scheme type.
	/// @return The color scheme.
	static continuous_color_scheme function(std::function<cgv::rgb(float)> fn, ColorSchemeType type = ColorSchemeType::kUndefined) {
		return { cgv::math::function_ref_interpolator<cgv::rgb, float>(fn), type };
	}

	/// @brief Return the interpolator.
	std::shared_ptr<const interpolator_type> get_interpolator() const {
		return interpolator_;
	}

	/// @brief Evaluate the color scheme at position t.
	/// 
	/// @param t The position at which to evaluate the interpolated values.
	/// @return The interpolated color.
	cgv::rgb interpolate(float t) const {
		return interpolator_->at(t);
	}

	/// @brief Evaluate the color scheme at n uniformly-spaced positions within the range [0,1].
	/// 
	/// @param n The number of samples to interpolate.
	/// @return The sequence of interpolated colors.
	std::vector<cgv::rgb> quantize(size_t n) const {
		return interpolator_->quantize(n);
	}

private:
	/// The interpolator used for mapping scalars to colors.
	std::shared_ptr<const interpolator_type> interpolator_;
};

/// @brief This class represents a discrete color scheme using sets of colors to convert discrete scalars to colors.
class discrete_color_scheme : public color_scheme {
public:
	/// Construct an empty scheme.
	discrete_color_scheme() {}

	/// @brief Construct from a set of colors and scheme type. The type should describe the color set produced by the interpolator if possible (typically ColorSchemeType::kCategorical).
	/// 
	/// @param colors The set of used colors.
	/// @param type The color scheme type.
	discrete_color_scheme(const std::vector<cgv::rgb>& colors, ColorSchemeType type = ColorSchemeType::kUndefined) : discrete_color_scheme(std::vector<std::vector<cgv::rgb>>{ colors }, type) {}
	
	/// @brief Construct from a list of color sets and scheme type.
	/// The color sets represent variants of the scheme that should be similar but can express slight variations at different scheme sizes.
	/// 
	/// @param variants The list of sets of color scheme variants.
	/// @param type The color scheme type.
	discrete_color_scheme(const std::vector<std::vector<cgv::rgb>>& variants, ColorSchemeType type = ColorSchemeType::kUndefined) {
		this->type = type;
		variants_.clear();
		for(const auto& variant : variants)
			variants_.insert({ variant.size(), variant });
	}

	/// @brief Return a sequence of colors of the specified count.
	/// Returns the scheme variant with exact size n if it exists.
	/// Otherwise, for schemes of type ColorSchemeType::kCategorical the next largest or largest available variant is returned,
	/// possibly being larger than n. For all other scheme types the next smaller variant is smoothly interpolated at n positions
	/// and the result returned.
	/// 
	/// @param n The number of requested colors.
	/// @return A sequence of n colors.
	const std::vector<cgv::rgb> get_colors(size_t n) const {
		if(variants_.empty())
			return {};

		// Return the variant with the exact requested size if available.
		auto it = variants_.find(n);
		if(it != variants_.end())
			return it->second;

		// Otherwise, find the next largest variant.
		it = variants_.upper_bound(n);

		if(type == ColorSchemeType::kCategorical) {
			// For categorical-type schemes the colors are not interpolated, so return at least n colors or the maximum available amount.
			if(it == variants_.end())
				--it;
			return it->second;
		} else {
			// For all other types the next-smaller variant is interpolated and quantized to n colors.
			if(it != variants_.begin())
				--it;

			if(!it->second.empty())
				return cgv::math::uniform_smooth_interpolator<cgv::rgb, float>(it->second).quantize(n);
			else
				return {};
		}
	}


private:
	/// The available color scheme variants indexed by their size.
	std::map<size_t, std::vector<cgv::rgb>> variants_;
};

/// A type of object_registry that manages continuous_color_schemes.
using continuous_color_scheme_registry = cgv::data::object_registry<continuous_color_scheme>;
/// A type of object_registry that manages discrete_color_schemes.
using discrete_color_scheme_registry = cgv::data::object_registry<discrete_color_scheme>;

/// @brief Return a reference to the global registry for continuous color schemes.
extern CGV_API continuous_color_scheme_registry& get_global_continuous_color_scheme_registry();
/// @brief Return a reference to the global registry for discrete color schemes.
extern CGV_API discrete_color_scheme_registry& get_global_discrete_color_scheme_registry();

/// @brief A helper class to simplify registering color schemes in a registry and counting the number of successful registrations.
/// Can be configured to only accept certain color scheme types.
/// 
/// @tparam RegistryT The registry type. Must accept ColorSchemeT.
/// @tparam ColorSchemeT The color scheme class type.
template<typename RegistryT, typename ColorSchemeT>
class color_scheme_registry_helper {
public:
	/// @brief Construct using a reference to the used registry and supported set of types.
	/// 
	/// @param registry The used color scheme registry.
	/// @param types The supported color scheme types.
	color_scheme_registry_helper(RegistryT& registry, const std::set<ColorSchemeType>& types) : registry_(registry), supported_types_(types) {}

	/// @brief Register a color scheme under the given name.
	/// 
	/// @param name The color scheme name.
	/// @param scheme The color scheme.
	void load(const std::string& name, const ColorSchemeT& scheme) {
		loaded_count_ += registry_.add(name, scheme) ? 1 : 0;
	}

	/// @brief Return whether the registry helper can load the given type of color scheme.
	/// 
	/// @param type The color scheme type to query.
	/// @return True if the scheme is supported, false otherwise.
	bool can_load(ColorSchemeType type) const {
		return supported_types_.empty() || supported_types_.find(type) != supported_types_.end();
	}

	/// @brief Return the count of successfully registered color schemes. 
	size_t loaded_count() const {
		return loaded_count_;
	}

private:
	/// The count of registered color schemes.
	size_t loaded_count_ = 0;
	/// Reference to the used color scheme registry.
	RegistryT& registry_;
	/// The set of supported color scheme types.
	std::set<ColorSchemeType> supported_types_;
};

/// A type of color_scheme_registry_helper that manages a continuous_color_scheme_registry.
using continuous_color_scheme_registry_helper = color_scheme_registry_helper<continuous_color_scheme_registry, continuous_color_scheme>;
/// A type of color_scheme_registry_helper that manages a discrete_color_scheme_registry.
using discrete_color_scheme_registry_helper = color_scheme_registry_helper<discrete_color_scheme_registry, discrete_color_scheme>;

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
