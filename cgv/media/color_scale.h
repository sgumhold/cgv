#pragma once

#include <vector>
#include <string>

#include <cgv/math/fvec.h>

#include "color.h"
#include "color_scheme.h"

#include "lib_begin.h"

namespace cgv {
namespace media {



enum class ColorScaleTransform {
	kLinear = 0,
	kPow,
	kLog
};

class color_scale {
public:
	virtual ~color_scale() {}

	virtual bool is_discrete() const = 0;

	virtual bool is_opaque() const {
		return true;
	}

	virtual cgv::rgba get_mapped_value(float value) const {
		return { get_mapped_color(value), get_mapped_opacity(value) };
	}

	virtual cgv::rgb get_mapped_color(float value) const {
		return cgv::rgb(unknown_color_);
	}
	
	virtual float get_mapped_opacity(float value) const {
		return 1.0f;
	}

	virtual cgv::rgba get_indexed_value(size_t index) const {
		return unknown_color_;
	}

	virtual size_t get_indexed_color_count() const {
		return 0;
	}

	// Todo: can we simplify this into one method template?
	// Todo: Attention: this will use the mappinmg options to get the quantized values, which may not be the preferred or expected outcome!
	virtual std::vector<cgv::rgba> quantize(size_t count) const {
		std::vector<cgv::rgba> colors;

		if(is_discrete()) {
			for(size_t i = 0; i < get_indexed_color_count(); ++i)
				colors.push_back(get_indexed_value(i));
		} else {
			colors.reserve(count);
			cgv::math::sequence_transform(std::back_inserter(colors), [this](float value) { return get_mapped_value(value); }, count, domain_[0], domain_[1]);
		}

		return colors;
	};

	virtual std::vector<cgv::rgb> quantize_color(size_t count) const {
		std::vector<cgv::rgb> colors;

		if(is_discrete()) {
			for(size_t i = 0; i < get_indexed_color_count(); ++i)
				colors.push_back(get_indexed_value(i));
		} else {
			colors.reserve(count);
			cgv::math::sequence_transform(std::back_inserter(colors), [this](float value) { return get_mapped_color(value); }, count, domain_[0], domain_[1]);
		}

		return colors;
	};

	virtual std::vector<float> quantize_opacity(size_t count) const {
		std::vector<float> opacities;

		if(is_discrete()) {
			for(size_t i = 0; i < get_indexed_color_count(); ++i)
				opacities.push_back(get_indexed_value(i).alpha());
		} else {
			opacities.reserve(count);
			cgv::math::sequence_transform(std::back_inserter(opacities), [this](float value) { return get_mapped_opacity(value); }, count, domain_[0], domain_[1]);
		}

		return opacities;
	};

	virtual void set_domain(cgv::vec2 domain) {
		domain_ = domain;
		modified();
	}

	cgv::vec2 get_domain() const {
		return domain_;
	}

	virtual void set_transform(ColorScaleTransform transform) {
		transform_ = transform;
		modified();
	}

	ColorScaleTransform get_transform() const {
		return transform_;
	}

	virtual void set_exponent(float exponent) {
		exponent_ = exponent;
		modified();
	}

	float get_exponent() const {
		return exponent_;
	}

	virtual void set_base(float base) {
		base_ = base;
		modified();
	}

	float get_base() const {
		return base_;
	}

	virtual void set_clamped(bool clamped) {
		is_clamped_ = clamped;
		modified();
	}

	bool is_clamped() const {
		return is_clamped_;
	}

	virtual void set_reversed(bool reverse) {
		is_reversed_ = reverse;
		modified();
	}

	bool is_reversed() const {
		return is_reversed_;
	}

	virtual void set_unknown_color(cgv::rgba color) {
		unknown_color_ = color;
		modified();
	}

	cgv::rgba get_unknown_color() const {
		return unknown_color_;
	}

	void modified() {
		++temp_modified_counter_;
	};

protected:
	int64_t temp_modified_counter_ = 0;

	cgv::vec2 domain_ = { 0.0f, 1.0f };
	ColorScaleTransform transform_ = ColorScaleTransform::kLinear;
	bool is_clamped_ = true;
	bool is_reversed_ = false;
	float exponent_ = 1.0f;
	float base_ = 10.0f;
	cgv::rgba unknown_color_ = { 0.0f, 0.0f, 0.0f, 1.0f };
};

class continuous_color_scale : public color_scale {
public:
	continuous_color_scale() {};

	continuous_color_scale(const continuous_color_scheme& scheme) : scheme_(scheme) {};

	bool is_discrete() const override {
		return false;
	}

	bool is_opaque() const override {
		return true;
	}

	cgv::rgb get_mapped_color(float value) const override {
		if(is_clamped_)
			value = cgv::math::clamp(value, domain_[0], domain_[1]);
		else if(value < domain_[0] || value > domain_[0])
			return unknown_color_;

		float t = (value - domain_[0]) / (domain_[1] - domain_[0]);

		if(is_reversed_)
			t = 1.0f - t;

		return scheme_.interpolate(t);
	}

	void set_scheme(const continuous_color_scheme& scheme) {
		scheme_ = scheme;
	}

private:
	continuous_color_scheme scheme_;
};

class discrete_color_scale : public color_scale {
public:
	discrete_color_scale() : colors_({ { 0.0f } }) {}

	discrete_color_scale(const discrete_color_scheme& scheme, size_t size) {
		set_scheme(scheme, size);
	}

	bool is_discrete() const override {
		return true;
	}
	
	bool is_opaque() const override {
		return true;
	}

	cgv::rgb get_mapped_color(float value) const override {
		if(colors_.empty())
			return unknown_color_;

		// Todo: Move to base class without performance hit?
		if(is_clamped_)
			value = cgv::math::clamp(value, domain_[0], domain_[1]);
		else if(value < domain_[0] || value > domain_[0])
			return unknown_color_;

		float t = (value - domain_[0]) / (domain_[1] - domain_[0]);

		if(is_reversed_)
			t = 1.0f - t;

		if(t <= 0.0f)
			return colors_.front();
		else if(t >= 1.0f)
			return colors_.back();
		else
			return colors_[std::min(static_cast<size_t>(t * static_cast<float>(colors_.size())), colors_.size() - 1)];

		return cgv::math::interpolate_linear(colors_, t);
	}

	cgv::rgba get_indexed_value(size_t index) const override {
		if(index < colors_.size())
			return { colors_[index], 1.0f };
		return unknown_color_;
	}

	size_t get_indexed_color_count() const override {
		return colors_.size();
	}

	void set_scheme(const discrete_color_scheme& scheme, size_t size) {
		colors_ = scheme.get_colors(size);
	}

private:
	std::vector<cgv::rgb> colors_;
};




















/// <summary>
/// perform a gamma mapping from [0,1] to [0,1] with optional accountance of window zero position in case of bipolar color scales
/// </summary>
/// <tparam name="T">value type</param>
/// <param name="v">to be mapped value</param>
/// <param name="gamma">gamma parameter</param>
/// <param name="is_bipolar">whether bipolar gamma mapping should be use</param>
/// <param name="window_zero_position">zero window position for bipolar gamma mapping</param>
/// <returns></returns>
template<typename T>
T color_scale_gamma_mapping(T v, T gamma, bool is_bipolar, T window_zero_position) {
	if(is_bipolar) {
		T amplitude = std::max(window_zero_position, T(1) - window_zero_position);
		if(v < window_zero_position)
			return window_zero_position - std::pow((window_zero_position - v) / amplitude, gamma) * amplitude;
		else
			return std::pow((v - window_zero_position) / amplitude, gamma) * amplitude + window_zero_position;
	} else
		return std::pow(v, gamma);
}

/// <summary>
/// for the use of bipolar color maps this function can be used to adjust the value such that
/// the window position is mapped to the center of the bipolar color map. In a shader program
/// using color_scale.glsl this adjustment can be enabled by setting the uniform flag
/// map_window_zero_position_to_color_scale_center to true and specifying window_zero_position
/// with the float typed uniform of this name
/// </summary>
/// <tparam name="T">value type</param>
/// <param name="value">to be adjusted value</param>
/// <param name="window_zero_position">window position in the range [0,1] to which the 
/// attribute value zero is mapped</param>
/// <returns></returns>
template<typename T>
T adjust_zero_position(T v, T window_zero_position) {
	if(window_zero_position <= T(0.5))
		return T(1) - T(0.5) * (T(1) - v) / (T(1) - window_zero_position);
	else
		return T(0.5) * v / window_zero_position;
}

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
