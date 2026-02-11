#pragma once

#include <vector>

#include <cgv/data/time_stamp.h>
#include <cgv/math/fvec.h>

#include "color.h"
#include "color_scheme.h"

#include "lib_begin.h"

namespace cgv {
namespace media {

class CGV_API color_scale {
public:
	virtual ~color_scale() {}

	virtual bool is_discrete() const = 0;

	virtual bool is_opaque() const {
		return true;
	}

	virtual void set_domain(cgv::vec2 domain);

	cgv::vec2 get_domain() const {
		return domain_;
	}

	virtual void set_clamped(bool clamped);

	bool is_clamped() const {
		return is_clamped_;
	}

	virtual void set_reversed(bool reverse);

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

	virtual float normalize_value(float value) const {
		return 0.0f;
	};

	virtual cgv::rgba map_value(float value) const {
		return { get_mapped_color(value), get_mapped_opacity(value) };
	}

	virtual cgv::rgb get_mapped_color(float value) const {
		return cgv::rgb(unknown_color_);
	}

	virtual float get_mapped_opacity(float value) const {
		return 1.0f;
	}

	virtual cgv::rgba get_indexed_color(size_t index) const {
		return unknown_color_;
	}

	virtual size_t get_indexed_color_count() const {
		return 0;
	}

	virtual std::vector<cgv::rgba> quantize(size_t count) const = 0;

	virtual std::vector<float> get_ticks(size_t request_count) const = 0;

	cgv::data::time_point get_modified_time() const {
		return time_.get_modified_time();
	}

protected:
	void modified() {
		time_.modified();
	};

	bool is_unknown(float value) const;

	float map_range_safe(float value, float in_left, float in_right, float out_left, float out_right) const;

private:
	cgv::data::time_stamp time_;

	cgv::vec2 domain_ = { 0.0f, 1.0f };
	bool is_clamped_ = true;
	bool is_reversed_ = false;
	cgv::rgba unknown_color_ = { 0.0f, 0.0f, 0.0f, 1.0f };
};

enum class ContinuousMappingTransform {
	kLinear = 0,
	kPow,
	kLog
};

class CGV_API continuous_color_scale : public color_scale {
public:
	continuous_color_scale() {};

	continuous_color_scale(const continuous_color_scheme& scheme) : scheme_(scheme) {};

	bool is_discrete() const override {
		return false;
	}

	bool is_opaque() const override {
		return true;
	}

	void set_domain(cgv::vec2 domain) override;

	void set_transform(ContinuousMappingTransform transform);

	ContinuousMappingTransform get_transform() const {
		return mapping_transform_;
	}

	virtual void set_pow_exponent(float exponent);

	float get_pow_exponent() const {
		return pow_exponent_;
	}

	virtual void set_log_base(float base);

	float get_log_base() const {
		return log_base_;
	}

	virtual void set_diverging(bool diverging);

	bool is_diverging() const {
		return is_diverging_;
	}

	virtual void set_midpoint(float midpoint);

	float get_midpoint() const {
		return diverging_midpoint_;
	}

	float normalize_value(float value) const override;

	cgv::rgb get_mapped_color(float value) const override;

	std::vector<cgv::rgba> quantize(size_t count) const override;

	std::vector<float> get_ticks(size_t request_count) const override;

	void set_scheme(const continuous_color_scheme& scheme);

	continuous_color_scheme get_scheme() const {
		return scheme_;
	}

private:
	void update_log_invariants();

	continuous_color_scheme scheme_;
	ContinuousMappingTransform mapping_transform_ = ContinuousMappingTransform::kLinear;
	bool is_diverging_ = false;
	float diverging_midpoint_ = 0.5f;
	float pow_exponent_ = 1.0f;
	float log_base_ = 10.0f;
	float log_of_base_ = 1.0f;
	float log_of_midpoint_ = 0.5f;
	float log_of_lower_bound_ = 0.0f;
	float log_of_upper_bound_ = 1.0f;
	float log_value_sign_ = 1.0f;
};

class CGV_API discrete_color_scale : public color_scale {
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

	float normalize_value(float value) const override;

	cgv::rgb get_mapped_color(float value) const override;

	cgv::rgba get_indexed_color(size_t index) const override;

	size_t get_indexed_color_count() const override {
		return colors_.size();
	}

	std::vector<cgv::rgba> quantize(size_t count) const override;

	std::vector<float> get_ticks(size_t request_count) const override;

	void set_scheme(const discrete_color_scheme& scheme, size_t size);

	discrete_color_scheme get_scheme() const {
		return cgv::media::discrete_color_scheme(colors_);
	}

private:
	std::vector<cgv::rgb> colors_;
};

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
