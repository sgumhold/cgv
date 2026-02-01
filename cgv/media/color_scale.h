#pragma once

#include <vector>
#include <string>

#include <cgv/data/time_stamp.h>
#include <cgv/math/fvec.h>
#include <cgv/math/compare_float.h>

#include "color.h"
#include "color_scheme.h"

#include "lib_begin.h"

namespace cgv {
namespace media {

enum class SequentialMappingTransform {
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

	virtual void set_domain(cgv::vec2 domain) {
		domain_ = domain;
		modified();
	}

	cgv::vec2 get_domain() const {
		return domain_;
	}

	virtual void set_clamped(bool clamped) {
		if(is_clamped_ != clamped) {
			is_clamped_ = clamped;
			modified();
		}
	}

	bool is_clamped() const {
		return is_clamped_;
	}

	virtual void set_reversed(bool reverse) {
		if(is_reversed_ != reverse) {
			is_reversed_ = reverse;
			modified();
		}
	}

	bool is_reversed() const {
		return is_reversed_;
	}

	virtual void set_unknown_color(cgv::rgba color) {
		if(unknown_color_ != color) {
			unknown_color_ = color;
			modified();
		}
	}

	cgv::rgba get_unknown_color() const {
		return unknown_color_;
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

	virtual std::vector<cgv::rgba> quantize(size_t count) const = 0;

	cgv::data::time_point get_modified_time() const {
		return time_.get_modified_time();
	}

	// Todo: change name to reflect that this checks for clamping
	bool is_out_of_domain(float value) const {
		return !is_clamped_ && (value < domain_[0] || value > domain_[1]);
	}

protected:
	void modified() {
		time_.modified();
	};

	float map_range_safe(float value, float in_left, float in_right, float out_left, float out_right) const {
		float size = in_right - in_left;
		if(cgv::math::is_zero(size))
			return out_left;
		return out_left + (out_right - out_left) * ((value - in_left) / size);
	}

private:
	cgv::data::time_stamp time_;

	cgv::vec2 domain_ = { 0.0f, 1.0f };
	bool is_clamped_ = true;
	bool is_reversed_ = false;
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

	virtual void set_transform(SequentialMappingTransform transform) {
		mapping_transform_ = transform;
		modified();
	}

	SequentialMappingTransform get_transform() const {
		return mapping_transform_;
	}

	virtual void set_pow_exponent(float exponent) {
		if(pow_exponent_ != exponent) {
			pow_exponent_ = exponent;
			modified();
		}
	}

	float get_pow_exponent() const {
		return pow_exponent_;
	}

	virtual void set_log_base(float base) {
		if(log_base_ != base) {
			log_base_ = base;
			modified();
		}
	}

	float get_log_base() const {
		return log_base_;
	}

	virtual void set_diverging(bool diverging) {
		if(is_diverging_ != diverging) {
			is_diverging_ = diverging;
			modified();
		}
	}

	bool is_diverging() const {
		return is_diverging_;
	}

	virtual void set_midpoint(float midpoint) {
		if(diverging_midpoint_ != midpoint) {
			diverging_midpoint_ = midpoint;
			modified();
		}
	}

	float get_midpoint() const {
		return diverging_midpoint_;
	}

	cgv::rgb get_mapped_color(float value) const override {
		if(is_out_of_domain(value))
			return { get_unknown_color(), 1.0f };
		return scheme_.interpolate(map_value(value));
	}

	std::vector<cgv::rgba> quantize(size_t count) const override {
		std::vector<cgv::rgba> colors;
		colors.reserve(count);
		cgv::vec2 domain = { 0.0f, 1.0f };
		if(is_reversed())
			std::swap(domain[0], domain[1]);
		cgv::math::sequence_transform(std::back_inserter(colors), [this](float t) { return scheme_.get_interpolator()->at(t); }, count, domain[0], domain[1]);
		return colors;
	}

	void set_scheme(const continuous_color_scheme& scheme) {
		scheme_ = scheme;
		modified();
	}

	float map_value(float value) const {
		// Todo: Remove underscore.
		cgv::vec2 domain_ = get_domain();
		if(is_clamped())
			value = cgv::math::clamp(value, domain_[0], domain_[1]);

		float t = 0.0f;

		switch(mapping_transform_) {
		case SequentialMappingTransform::kLinear:
		{
			if(is_diverging_) {
				if(value < diverging_midpoint_)
					t = map_range_safe(value, domain_[0], diverging_midpoint_, 0.0f, 0.5f);
				else
					t = map_range_safe(value, diverging_midpoint_, domain_[1], 0.5f, 1.0f);
			} else {
				t = map_range_safe(value, domain_[0], domain_[1], 0.0f, 1.0f);
			}
			break;
		}
		case SequentialMappingTransform::kPow:
		{
			if(is_diverging_) {
				if(value < diverging_midpoint_) {
					t = map_range_safe(value, domain_[0], diverging_midpoint_, 0.0f, 1.0f);
					t = 0.5f * (1.0f - std::pow(1.0f - t, pow_exponent_));
				} else {
					t = map_range_safe(value, diverging_midpoint_, domain_[1], 0.0f, 1.0f);
					t = 0.5f * std::pow(t, pow_exponent_) + 0.5f;
				}
			} else {
				t = map_range_safe(value, domain_[0], domain_[1], 0.0f, 1.0f);
				t = std::pow(t, pow_exponent_);
			}
			break;
		}
		case SequentialMappingTransform::kLog:
			std::cout << "color_scale log mapping not implemented" << std::endl;
			break;
		}

		return is_reversed() ? 1.0f - t : t;
	}

private:
	continuous_color_scheme scheme_;
	SequentialMappingTransform mapping_transform_ = SequentialMappingTransform::kLinear;
	bool is_diverging_ = false;
	float diverging_midpoint_ = 0.5f;
	float pow_exponent_ = 1.0f;
	float log_base_ = 10.0f;
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
		if(is_out_of_domain(value) || colors_.empty())
			return { get_unknown_color(), 1.0f };

		cgv::vec2 domain = get_domain();
		if(is_clamped())
			value = cgv::math::clamp(value, domain[0], domain[1]);

		float t = map_range_safe(value, domain[0], domain[1], 0.0f, 1.0f);

		if(t <= 0.0f)
			return colors_.front();
		else if(t >= 1.0f)
			return colors_.back();
		else
			return colors_[std::min(static_cast<size_t>(t * static_cast<float>(colors_.size())), colors_.size() - 1)];

		return cgv::math::interpolate_linear(colors_, t);
	}

	cgv::rgba get_indexed_value(size_t index) const override {
		// Todo: use modulo
		if(index < colors_.size())
			return { colors_[index], 1.0f };
		return { get_unknown_color(), 1.0f };
	}

	size_t get_indexed_color_count() const override {
		return colors_.size();
	}

	std::vector<cgv::rgba> quantize(size_t count) const override {
		std::vector<cgv::rgba> colors;
		size_t color_count = get_indexed_color_count();
		for(size_t i = 0; i < color_count; ++i) {
			size_t index = is_reversed() ? color_count - i - 1 : i;
			colors.push_back(colors_[index]);
		}
		return colors;
	}

	void set_scheme(const discrete_color_scheme& scheme, size_t size) {
		colors_ = scheme.get_colors(size);
	}

private:
	std::vector<cgv::rgb> colors_;
};

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
