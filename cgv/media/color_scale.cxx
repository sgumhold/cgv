#include "color_scale.h"

#include <cgv/math/compare_float.h>
#include <cgv/media/ticks.h>

namespace cgv {
namespace media {

void color_scale::set_domain(cgv::vec2 domain) {
	domain_ = domain;
	modified();
}

void color_scale::set_clamped(bool clamped) {
	if(is_clamped_ != clamped) {
		is_clamped_ = clamped;
		modified();
	}
}

void color_scale::set_reversed(bool reverse) {
	if(is_reversed_ != reverse) {
		is_reversed_ = reverse;
		modified();
	}
}

bool color_scale::is_unknown(float value) const {
	const vec2 domain = get_domain();
	return !is_clamped() && (value < domain[0] || value > domain[1]);
}

float color_scale::map_range_safe(float value, float in_left, float in_right, float out_left, float out_right) const {
	float size = in_right - in_left;
	if(cgv::math::is_zero(size))
		return out_left;
	return out_left + (out_right - out_left) * ((value - in_left) / size);
}

void continuous_color_scale::set_domain(cgv::vec2 domain) {
	color_scale::set_domain(domain);
	update_log_invariants();
}

void continuous_color_scale::set_transform(ContinuousMappingTransform transform) {
	mapping_transform_ = transform;
	update_log_invariants();
	modified();
}

void continuous_color_scale::set_pow_exponent(float exponent) {
	if(pow_exponent_ != exponent) {
		pow_exponent_ = exponent;
		modified();
	}
}

void continuous_color_scale::set_log_base(float base) {
	if(log_base_ != base) {
		log_base_ = base;
		update_log_invariants();
		modified();
	}
}

void continuous_color_scale::set_diverging(bool diverging) {
	if(is_diverging_ != diverging) {
		is_diverging_ = diverging;
		modified();
	}
}

void continuous_color_scale::set_midpoint(float midpoint) {
	if(diverging_midpoint_ != midpoint) {
		diverging_midpoint_ = midpoint;
		modified();
	}
}

float continuous_color_scale::normalize_value(float value) const {
	const cgv::vec2 domain = get_domain();
	if(is_clamped())
		value = cgv::math::clamp(value, domain[0], domain[1]);

	float t = 0.0f;

	switch(mapping_transform_) {
	case ContinuousMappingTransform::kLinear:
		if(is_diverging_) {
			if(value < diverging_midpoint_)
				t = map_range_safe(value, domain[0], diverging_midpoint_, 0.0f, 0.5f);
			else
				t = map_range_safe(value, diverging_midpoint_, domain[1], 0.5f, 1.0f);
		} else {
			t = map_range_safe(value, domain[0], domain[1], 0.0f, 1.0f);
		}
		break;
	case ContinuousMappingTransform::kPow:
		if(is_diverging_) {
			if(value < diverging_midpoint_) {
				t = map_range_safe(value, domain[0], diverging_midpoint_, 0.0f, 1.0f);
				t = 0.5f * (1.0f - std::pow(1.0f - t, pow_exponent_));
			} else {
				t = map_range_safe(value, diverging_midpoint_, domain[1], 0.0f, 1.0f);
				t = 0.5f * std::pow(t, pow_exponent_) + 0.5f;
			}
		} else {
			t = map_range_safe(value, domain[0], domain[1], 0.0f, 1.0f);
			t = std::pow(t, pow_exponent_);
		}
		break;
	case ContinuousMappingTransform::kLog:
		t = std::log(log_value_sign_ * value) / log_of_base_;
		if(is_diverging_) {
			if(value < diverging_midpoint_) {
				t = map_range_safe(t, log_of_lower_bound_, log_of_midpoint_, 0.0f, 0.5f);
			} else {
				t = map_range_safe(t, log_of_midpoint_, log_of_upper_bound_, 0.5f, 1.0f);
			}
		} else {
			t = map_range_safe(t, log_of_lower_bound_, log_of_upper_bound_, 0.0f, 1.0f);
		}
		if(std::isnan(t))
			t = 0.0f;
		t *= log_value_sign_;
		break;
	default:
		break;
	}

	return is_reversed() ? 1.0f - t : t;
}

cgv::rgb continuous_color_scale::get_mapped_color(float value) const {
	if(is_unknown(value))
		return { get_unknown_color(), 1.0f };
	return scheme_.interpolate(normalize_value(value));
}

std::vector<cgv::rgba> continuous_color_scale::quantize(size_t count) const {
	std::vector<cgv::rgba> colors;
	colors.reserve(count);
	cgv::vec2 domain = { 0.0f, 1.0f };
	if(is_reversed())
		std::swap(domain[0], domain[1]);
	cgv::math::sequence_transform(std::back_inserter(colors), [this](float t) { return scheme_.get_interpolator()->at(t); }, count, domain[0], domain[1]);
	return colors;
}

std::vector<float> continuous_color_scale::get_ticks(size_t request_count) const {
	const vec2 domain = get_domain();

	if(mapping_transform_ == ContinuousMappingTransform::kLog)
		return compute_ticks_log(domain[0], domain[1], log_base_, request_count);
	else
		return compute_ticks(domain[0], domain[1], request_count);
}

void continuous_color_scale::set_scheme(const continuous_color_scheme& scheme) {
	scheme_ = scheme;
	modified();
}

void continuous_color_scale::update_log_invariants() {
	if(mapping_transform_ == ContinuousMappingTransform::kLog) {
		vec2 domain = get_domain();

		if(domain[0] < 0.0f && domain[1] < 0.0f)
			log_value_sign_ = -1.0f;

		log_of_base_ = std::log(log_base_);
		log_of_midpoint_ = std::log(log_value_sign_ * diverging_midpoint_) / log_of_base_;
		log_of_lower_bound_ = std::log(log_value_sign_ * domain[0]) / log_of_base_;
		log_of_upper_bound_ = std::log(log_value_sign_ * domain[1]) / log_of_base_;
	}
}

float discrete_color_scale::normalize_value(float value) const {
	cgv::vec2 domain = get_domain();
	if(is_clamped())
		value = cgv::math::clamp(value, domain[0], domain[1]);

	return map_range_safe(value, domain[0], domain[1], 0.0f, 1.0f);
}

cgv::rgb discrete_color_scale::get_mapped_color(float value) const {
	if(is_unknown(value) || colors_.empty())
		return { get_unknown_color(), 1.0f };

	float t = normalize_value(value);

	if(t <= 0.0f)
		return colors_.front();
	else if(t >= 1.0f)
		return colors_.back();
	else
		return colors_[std::min(static_cast<size_t>(std::floor(t * static_cast<float>(colors_.size()))), colors_.size() - 1)];
}

cgv::rgba discrete_color_scale::get_indexed_color(size_t index) const {
	index %= colors_.size();
	return { colors_[index], 1.0f };
}

std::vector<cgv::rgba> discrete_color_scale::quantize(size_t count) const {
	std::vector<cgv::rgba> colors;
	size_t color_count = get_indexed_color_count();
	for(size_t i = 0; i < color_count; ++i) {
		size_t index = is_reversed() ? color_count - i - 1 : i;
		colors.push_back(colors_[index]);
	}
	return colors;
}

std::vector<float> discrete_color_scale::get_ticks(size_t request_count) const {
	return {};
}

void discrete_color_scale::set_scheme(const discrete_color_scheme& scheme, size_t size) {
	colors_ = scheme.get_colors(size);
	modified();
}

} // namespace media
} // namespace cgv
