#pragma once

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <cgv/data/object_registry.h>
#include <cgv/math/interpolate.h>

#include "color.h"

#include "lib_begin.h"

namespace cgv {
namespace media {

/// @brief Return a cgv::rgb color unpacked from an 32-bit integer where the lower 24 bits are interpreted as three 8-bit rgb channels.
static cgv::rgb to_rgb(int32_t hex) {
	return {
		((hex >> 16) & 0xFF) / 255.0f,
		((hex >> 8) & 0xFF) / 255.0f,
		(hex & 0xFF) / 255.0f
	};
}

/// @brief Return a vector of colors from an array of packed 32-bit integers converted using to_rgb.
template<int32_t N>
static std::vector<cgv::rgb> to_colors(const std::array<int32_t, N>& scheme) {
	std::vector<cgv::rgb> colors;
	colors.reserve(scheme.size());
	for(int32_t hex : scheme)
		colors.push_back(to_rgb(hex));
	return colors;
}

/// @brief Return a vector of colors converted from a char array encoded as hexadecimal. Every six consecutive chars are interpreted as a single rgb color.
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

/// @brief Return a vector of color vectors converted from an array of char arrays. See also to_colors(const char* data).
template<uint32_t N>
static std::vector<std::vector<cgv::rgb>> to_colors(const std::array<const char*, N>& data) {
	std::vector<std::vector<cgv::rgb>> colors;
	colors.reserve(N);
	for(const char* entry : data)
		colors.push_back(to_colors(entry));
	return colors;
}

enum class ColorSchemeType {
	kUndefined,
	kSequential,
	kDiverging,
	kCyclical,
	kCategorical
};

struct color_scheme {
	ColorSchemeType type = ColorSchemeType::kUndefined;
};

class continuous_color_scheme : public color_scheme {
public:
	using interpolator_type = cgv::math::interpolator<cgv::rgb, float>;

	continuous_color_scheme() : continuous_color_scheme(cgv::math::identity_interpolator<cgv::rgb, float>()) {}

	continuous_color_scheme(const interpolator_type& interpolator, ColorSchemeType type = ColorSchemeType::kUndefined) {
		this->type = type;
		interpolator_ = interpolator.clone();
	}

	static continuous_color_scheme linear(const std::vector<cgv::rgb>& colors, ColorSchemeType type = ColorSchemeType::kUndefined) {
		return { cgv::math::uniform_linear_interpolator<cgv::rgb, float>(colors), type };
	}

	static continuous_color_scheme linear(const std::vector<std::pair<float, cgv::rgb>>& colors, ColorSchemeType type = ColorSchemeType::kUndefined) {
		return { cgv::math::linear_interpolator<cgv::rgb, float>(colors), type };
	}

	static continuous_color_scheme smooth(const std::vector<cgv::rgb>& colors, ColorSchemeType type = ColorSchemeType::kUndefined) {
		return { cgv::math::uniform_smooth_interpolator<cgv::rgb, float>(colors), type };
	}

	static continuous_color_scheme function(std::function<cgv::rgb(float)> fn, ColorSchemeType type = ColorSchemeType::kUndefined) {
		return { cgv::math::function_ref_interpolator<cgv::rgb, float>(fn), type };
	}

	std::shared_ptr<const interpolator_type> get_interpolator() const {
		return interpolator_;
	}

	cgv::rgb interpolate(float t) const {
		return interpolator_->at(t);
	}

	std::vector<cgv::rgb> quantize(size_t n) const {
		return interpolator_->quantize(n);
	}

private:
	std::shared_ptr<const interpolator_type> interpolator_;
};

class discrete_color_scheme : public color_scheme {
public:
	discrete_color_scheme() {}

	discrete_color_scheme(const std::vector<cgv::rgb>& colors, ColorSchemeType type = ColorSchemeType::kUndefined) : discrete_color_scheme(std::vector<std::vector<cgv::rgb>>{ colors }, type) {}
	
	discrete_color_scheme(const std::vector<std::vector<cgv::rgb>>& variants, ColorSchemeType type = ColorSchemeType::kUndefined) {
		this->type = type;
		set_variants(variants);
	}

	const std::vector<cgv::rgb> get_colors(size_t n) const {
		if(variants_.empty())
			return {};

		// Return the variant with the exact requested size if available.
		auto it = variants_.find(n);
		if(it != variants_.end())
			return it->second;

		// Otherwise find the next largest variant.
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
	void set_variants(const std::vector<std::vector<cgv::rgb>>& variants) {
		variants_.clear();
		for(const auto& variant : variants)
			variants_.insert({ variant.size(), variant });
	}

	std::map<size_t, std::vector<cgv::rgb>> variants_;
};

using continuous_color_scheme_registry = cgv::data::object_registry<continuous_color_scheme>;
using discrete_color_scheme_registry = cgv::data::object_registry<discrete_color_scheme>;

extern CGV_API continuous_color_scheme_registry& get_global_continuous_color_scheme_registry();
extern CGV_API discrete_color_scheme_registry& get_global_discrete_color_scheme_registry();

template<typename RegistryT, typename ColorSchemeT>
class color_scheme_registry_helper {
public:
	color_scheme_registry_helper(RegistryT& registry, const std::set<ColorSchemeType>& types) : registry_(registry), supported_types_(types) {}

	void load(const std::string& name, const ColorSchemeT& scheme) {
		loaded_count_ += registry_.add(name, scheme) ? 1 : 0;
	}

	bool can_load(ColorSchemeType type) const {
		return supported_types_.empty() || supported_types_.find(type) != supported_types_.end();
	}

	size_t loaded_count() const {
		return loaded_count_;
	}

private:
	size_t loaded_count_ = 0;
	RegistryT& registry_;
	std::set<ColorSchemeType> supported_types_;
};

using continuous_color_scheme_registry_helper = color_scheme_registry_helper<continuous_color_scheme_registry, continuous_color_scheme>;
using discrete_color_scheme_registry_helper = color_scheme_registry_helper<discrete_color_scheme_registry, discrete_color_scheme>;

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
