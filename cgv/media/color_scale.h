#pragma once

#include <vector>

#include <cgv/data/time_stamp.h>
#include <cgv/math/fvec.h>

#include "color.h"
#include "color_scheme.h"

#include "lib_begin.h"

namespace cgv {
namespace media {

/// @brief Base class defining an interface for color scales.
///
/// A color_scale is a function that maps scalar values from an input domain to a RGB output range. The color_scale
/// can optionally also map to opacity. The mapping from input to output values can be linear or follow some other
/// transform, like log or power scales. color_scales can have a continuous or discrete input domain and can be sampled
/// with continuous scalar values or, in case of discrete scales, additionally with indexed lookup.
class CGV_API color_scale {
public:
	/// @brief Destruct this color scale.
	virtual ~color_scale() {}

	/// @brief Return whether the color scale represents a discrete mapping.
	/// If true, implementations should allow indexed color lookup.
	/// If false, indexed color lookup is potentially undefined.
	/// 
	/// @return True if the mapping uses discrete input values, false otherwise.
	virtual bool is_discrete() const = 0;

	/// @brief Return whether the color scale maps to opacity.
	/// If true, implementations should provide mapped opacity alongside mapped color values.
	/// If false, the returned opacity should always be 1.
	/// 
	/// @return True if opacity is supported, false otherwise.
	virtual bool is_opaque() const {
		return true;
	}

	/// @brief Set the input domain of scalars that will be mapped.
	/// Defaults to [0,1].
	/// 
	/// @param domain The new domain.
	virtual void set_domain(cgv::vec2 domain);

	/// @brief Get the input domain of scalars that will be mapped.
	/// 
	/// @return The domain.
	cgv::vec2 get_domain() const {
		return domain_;
	}

	/// @brief Set whether the input values are clamped to the domain before mapping.
	/// 
	/// @param clamped Set to true to enable clamping.
	virtual void set_clamped(bool clamped);

	/// @brief Get whether the input values are clamped to the domain before mapping.
	///
	/// @return True if clamped, false otherwise.
	bool is_clamped() const {
		return is_clamped_;
	}

	/// @brief Set whether the output color ramp is reversed.
	/// 
	/// @param reverse Set to true to reverse.
	virtual void set_reversed(bool reverse);

	/// @brief Get whether the output color ramp is reversed.
	///
	/// @return True if reversed, false otherwise.
	bool is_reversed() const {
		return is_reversed_;
	}

	/// @brief Set the color returned for scalars outside the domain if clamping is disabled.
	/// 
	/// @param color The color.
	virtual void set_unknown_color(cgv::rgba color) {
		if(unknown_color_ != color) {
			unknown_color_ = color;
			modified();
		}
	}

	/// @brief Get the color returned for scalars outside the domain if clamping is disabled.
	/// 
	/// @return The color.
	cgv::rgba get_unknown_color() const {
		return unknown_color_;
	}

	/// @brief Map a value through the scale and return a normalized value in the range [0,1].
	/// 
	/// @param value The value to map.
	/// @return The normalized value.
	virtual float normalize_value(float value) const {
		return 0.0f;
	};

	/// @brief Map a value through the scale and return a RGBA color.
	/// 
	/// @param value The value to map.
	/// @return The mapped color.
	virtual cgv::rgba map_value(float value) const {
		return { get_mapped_color(value), get_mapped_opacity(value) };
	}

	/// @brief Map a value through the scale and return a RGB color.
	/// 
	/// @param value The value to map.
	/// @return The mapped color.
	virtual cgv::rgb get_mapped_color(float value) const {
		return cgv::rgb(unknown_color_);
	}

	/// @brief Map a value through the scale and return an opacity.
	/// 
	/// @param value The value to map.
	/// @return The mapped color.
	virtual float get_mapped_opacity(float value) const {
		return 1.0f;
	}

	/// @brief Map a discrete index through the lookup table and return a RGBA color.
	/// This is typically only defined for discrete color scales.
	/// 
	/// @param index The index into the lookup table.
	/// @return The mapped color.
	virtual cgv::rgba get_indexed_color(size_t index) const {
		return unknown_color_;
	}

	/// @brief return the number of available indexed colors.
	/// This is typically only defined for discrete color scales.
	/// 
	/// @return The count.
	virtual size_t get_indexed_color_count() const {
		return 0;
	}

	/// @brief Return count uniformly spaced samples from the color ramp or all indexed colors.
	/// Will return a reversed color ramp if get_reversed() returns true.
	/// The number of returned samples may be different from count if color values cannot be interpolated as
	/// is typically the case for discrete color scales.
	/// 
	/// @param count The number of samples to return.
	/// @return The sequence of sampled colors.
	virtual std::vector<cgv::rgba> quantize(size_t count) const = 0;

	/// @brief Evaluate the color scheme at n uniformly-spaced positions within the range [0,1].
	/// 
	/// @param n The number of samples to interpolate.
	/// @return The sequence of interpolated colors.
	
	/// @brief Get a sequence of locations within the input domain representing tickmark locations for, e.g. a scale legend.
	/// The number of returned ticks is approximately equal to request_count.
	/// Implementations should aim to generate nicely-spaced and located ticks according to the set domain and mapping.
	/// 
	/// @param request_count The number of requested tickmarks.
	/// @return A sequence of tick locations.
	virtual std::vector<float> get_ticks(size_t request_count) const = 0;

	/// @brief Get the time point of the last modification of this object.
	/// 
	/// @return The time point.
	cgv::data::time_point get_modified_time() const {
		return time_.get_modified_time();
	}

protected:
	/// @brief Update the object's modified time.
	void modified() {
		time_.modified();
	};

	/// @brief Test whether the value is outside the domain according to the mapping options.
	/// 
	/// @param value The value to test.
	/// @return True if the value is unknown, false otherwise.
	bool is_unknown(float value) const;

	/// @brief Remap a scalar value from an input range to and output range while safely handling empty ranges.
	/// The value is not clamped to the input domain.
	/// 
	/// @param value The value to map.
	/// @param in_left The input range lower bound.
	/// @param in_right The input range upper bound.
	/// @param out_left The output range lower bound.
	/// @param out_right The output range upper bound.
	/// @return The mapped value.
	float map_range_safe(float value, float in_left, float in_right, float out_left, float out_right) const;

private:
	/// The modification timestamp of this object.
	cgv::data::time_stamp time_;

	/// The input domain of the scale.
	cgv::vec2 domain_ = { 0.0f, 1.0f };
	/// Whether to clamp values before mapping.
	bool is_clamped_ = true;
	/// Whether to reverse the color ramp.
	bool is_reversed_ = false;
	/// Input values outside of the domain get mapped to this value.
	cgv::rgba unknown_color_ = { 0.0f, 0.0f, 0.0f, 1.0f };
};

/// @brief The type of transform used for continuous scalar mappings.
enum class ContinuousMappingTransform {
	kLinear = 0,	// Linear: y = m * x + b
	kPow,			// Power: y = m * x^k + b
	kLog			// Logarithmic: y = m * log(x) + b
};

/// @brief Implementation of a color_scale with a continuous input domain and output range using a continuous_color_scheme.
/// The scale supports mapping input values to colors using one of the transforms defined in ContinuousMappingTransform.
class CGV_API continuous_color_scale : public color_scale {
public:
	/// @brief Construct using default arguments.
	continuous_color_scale() {};

	/// @brief Construct using the given color scheme.
	/// 
	/// @param scheme The color scheme used as the color ramp.
	continuous_color_scale(const continuous_color_scheme& scheme) : scheme_(scheme) {};

	/// See color_scale::is_discrete().
	/// @return False.
	bool is_discrete() const override {
		return false;
	}

	/// See color_scale::is_opaque().
	/// @return True.
	bool is_opaque() const override {
		return true;
	}

	/// See color_scale::set_domain().
	void set_domain(cgv::vec2 domain) override;

	/// @brief Set the transform used to map scalars to colors.
	/// Power and log scales support negative domains. For scales with power or log transform and negative domains,
	/// input and output values are implicitly multiplied by -1.
	/// Log scale domains must be strictly-positive or strictly-negative; the domain must not include or cross zero.
	/// The behavior of the scale is undefined if a negative value is passed to a log scale with a positive domain or vice versa.
	/// 
	/// @param transform The transform type.
	void set_transform(ContinuousMappingTransform transform);

	/// @brief Get the transform used to map scalars to colors.
	/// See also set_transform().
	/// 
	/// @return The transform type.
	ContinuousMappingTransform get_transform() const {
		return mapping_transform_;
	}

	/// @brief Set the exponent for power scales.
	/// Only used if get_transform() returns ContinuousMappingTransform::kPow.
	/// Defaults to 1.
	/// 
	/// @param exponent The exponent.
	virtual void set_pow_exponent(float exponent);

	/// @brief Get the exponent for power scales.
	/// 
	/// @return The exponent.
	float get_pow_exponent() const {
		return pow_exponent_;
	}

	/// @brief Set the base for logarithmic scales.
	/// Only used if get_transform() returns ContinuousMappingTransform::kLog.
	/// Defaults to 10.
	/// 
	/// @param exponent The exponent.
	virtual void set_log_base(float base);

	/// @brief Get the base for logarithmic scales.
	/// 
	/// @return The base.
	float get_log_base() const {
		return log_base_;
	}

	/// @brief Set whether the scale uses a diverging mapping.
	/// Diverging scales apply the transform mirrored around the midpoint.
	/// 
	/// @param diverging Set to true to enable diverging mapping.
	virtual void set_diverging(bool diverging);

	/// @brief Get whether the scale uses a diverging mapping.
	///
	/// @return True if diverging mapping is used, false otherwise.
	bool is_diverging() const {
		return is_diverging_;
	}

	/// @brief Set the midpoint for diverging scales.
	/// Only used if get_diverging() returns true. The midpoint should be inside the scales's domain.
	/// 
	/// @param midpoint The midpoint.
	virtual void set_midpoint(float midpoint);

	/// @brief Get the midpoint for diverging scales.
	/// 
	/// @return The midpoint.
	float get_midpoint() const {
		return diverging_midpoint_;
	}

	/// See color_scale::normalize_value()
	float normalize_value(float value) const override;

	/// See color_scale::get_mapped_value()
	cgv::rgb get_mapped_color(float value) const override;

	/// See color_scale::quantize().
	std::vector<cgv::rgba> quantize(size_t count) const override;

	/// See color_scale::get_ticks().
	std::vector<float> get_ticks(size_t request_count) const override;

	/// @brief Set the color scheme used as this scale's color ramp.
	/// 
	/// @param scheme The color scheme.
	void set_scheme(const continuous_color_scheme& scheme);

	/// @brief Get the color scheme used as this scale's color ramp.
	/// 
	/// @return The color scheme.
	continuous_color_scheme get_scheme() const {
		return scheme_;
	}

private:
	/// @brief Update internal precomputed parameters for logarithmic mapping.
	void update_log_invariants();

	/// The color scheme used to provide the output color ramp.
	continuous_color_scheme scheme_;
	/// The transform used to map scalars.
	ContinuousMappingTransform mapping_transform_ = ContinuousMappingTransform::kLinear;
	/// Whether the scale is diverging.
	bool is_diverging_ = false;
	/// The midpoint of a diverging scale.
	float diverging_midpoint_ = 0.5f;
	/// The exponent of a power scale.
	float pow_exponent_ = 1.0f;
	/// The base of a log scale.
	float log_base_ = 10.0f;

	/// The log of the base of a log scale.
	float log_of_base_ = 1.0f;
	/// The log of the midpoint of a diverging log scale.
	float log_of_midpoint_ = 0.5f;
	/// The log of the lower bound of the domain.
	float log_of_lower_bound_ = 0.0f;
	/// The log of the upper bound of the domain.
	float log_of_upper_bound_ = 1.0f;
	/// -1 for strictly negative domains of a log scale.
	float log_value_sign_ = 1.0f;
};

/// @brief Implementation of a color_scale with a discrete input domain and output range using a discrete_color_scheme.
/// The scale is mainly used for mapping discrete indices to discrete colors for, e.g., categorical data.
/// Additionally, discrete_color_scale supports mapping continuous input values to discrete colors.
class CGV_API discrete_color_scale : public color_scale {
public:
	// @brief Construct using default arguments. The scale will have a single black indexed color.
	discrete_color_scale() : colors_({ { 0.0f } }) {}

	/// @brief Construct using the given color scheme and size.
	/// 
	/// @param scheme The color scheme used as the source for indexed colors.
	/// @param size The number of colors to extract from the scheme.
	discrete_color_scale(const discrete_color_scheme& scheme, size_t size) {
		set_scheme(scheme, size);
	}

	/// See color_scale::is_discrete().
	/// @return True.
	bool is_discrete() const override {
		return true;
	}

	/// See color_scale::is_opaque().
	/// @return True.
	bool is_opaque() const override {
		return true;
	}

	/// See color_scale::normalize_value().
	float normalize_value(float value) const override;

	/// @brief Map a value through the scale and return a RGB color.
	/// Normalizes the value to the domain and discretizes it to an integer index in [0,n) where n is the number of indexed colors.
	/// 
	/// @param value The value to map.
	/// @return The mapped color.
	cgv::rgb get_mapped_color(float value) const override;

	/// @brief Return the RGBA color with the given index.
	/// 
	/// @param index The index into the lookup table.
	/// @return The mapped color.
	cgv::rgba get_indexed_color(size_t index) const override;

	/// See color_scale::get_indexed_color_count().
	size_t get_indexed_color_count() const override {
		return colors_.size();
	}

	/// @brief Return a sequence of all indexed colors, ignoring count.
	/// Will return the reversed sequence if get_reversed() returns true.
	/// 
	/// @param count Ignored.
	/// @return The sequence of indexed colors.
	std::vector<cgv::rgba> quantize(size_t count) const override;

	/// @brief Not implemented for discrete color scales.
	/// 
	/// @param request_count Ignored.
	/// @return An empty sequence.
	std::vector<float> get_ticks(size_t request_count) const override;

	/// @brief Set the color scheme and size used as this scale's indexed color source.
	/// 
	/// @param scheme The color scheme.
	void set_scheme(const discrete_color_scheme& scheme, size_t size);

	/// @brief Get the color scheme used as this scale's indexed color source.
	/// 
	/// @return The color scheme.
	discrete_color_scheme get_scheme() const {
		return cgv::media::discrete_color_scheme(colors_);
	}

private:
	/// The indexed colors.
	std::vector<cgv::rgb> colors_;
};

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
