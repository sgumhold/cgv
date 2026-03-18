#pragma once

#include <algorithm>
#include <vector>

#include <cgv/math/interpolate.h>
#include <cgv/math/fvec.h>

#include "color.h"
#include "color_scale.h"
#include "color_scheme.h"

#include "lib_begin.h"

namespace cgv {
namespace media {

/// @brief Implementation of a color_scale with a continuous input domain that maps explicit domain values to color and opacity 
/// using piecewise constant, piecewise linear or piecewise smooth (b-splines) interpolation. Control points of the color and opacity
/// function can be controlled independent of each other.
class CGV_API transfer_function : public color_scale {
public:
	/// @brief The interpolation modes supported by the transfer function.
	enum class InterpolationMode {
		kStep,
		kLinear,
		kSmooth
	};

	/// @brief The used color type.
	using color_type = cgv::rgb;
	/// @brief The used opacity type.
	using opacity_type = float;
	/// @brief The used color control point type.
	using color_point_type = std::pair<float, color_type>;
	/// @brief The used opacity control point type.
	using opacity_point_type = std::pair<float, opacity_type>;

	/// @brief Construct using default arguments.
	transfer_function() {}

	/// @brief Construct from a sequence of color control points.
	/// Sets the domain to the range governed by the control points.
	/// 
	/// @param colors The control points.
	transfer_function(std::initializer_list<color_point_type> colors);

	/// @brief Construct from separate sequences of color and opacity control points.
	/// Sets the domain to excatly cover the minimum and maximum position among all color and opacity control points.
	/// Additional control points are added to the ends of the individual piecwise functions if necessary.
	/// 
	/// @param colors The color control points.
	/// @param opacities The opacity control points.
	transfer_function(std::initializer_list<color_point_type> colors, std::initializer_list<opacity_point_type> opacities);

	/// @brief Return whether the color scale maps to opacity.
	/// 
	/// @return True if the opacity function is not empty, false otherwise.
	bool is_opaque() const override {
		return opacity_points_.empty();
	}

	/// See color_scale::is_discrete().
	/// @return False.
	bool is_discrete() const override {
		return false;
	}

	/// @brief Check whether the transfer function is empty.
	/// 
	/// @return true if both color and opacity functions have no control points.
	bool empty() const {
		return color_points_.empty() && opacity_points_.empty();
	}

	/// @brief Set the color function to use the given control points.
	/// Extends the domain if the new control points are outside the current domain.
	/// 
	/// @param colors The control points.
	void set_color_points(const std::vector<color_point_type>& colors);

	/// @brief Set the color function to use n uniformly sampled points from the given color scheme.
	/// Will set the domain to [0,1]. See also set_color_points().
	/// 
	/// @param scheme The color scheme.
	/// @param n The number of poitns to sample.
	void set_color_points_from_scheme(const cgv::media::continuous_color_scheme& scheme, size_t n);

	/// @brief Set the opacity function to use the given control points.
	/// Extends the domain if the new control points are outside the current domain.
	/// Opacity values are clamped to [0,1].
	/// 
	/// @param opacities The control points.
	void set_opacity_points(const std::vector<opacity_point_type>& opacities);

	/// @brief Add a color point at position x.
	/// Will replace the previous color point at position x if it exists.
	/// 
	/// @param x The control point position.
	/// @param color The control point color.
	void add_color_point(float x, const color_type& color);

	/// @brief Add an opacity point at position x.
	/// Will replace the previous opacity point at position x if it exists.
	/// The opacity is clamped to [0,1].
	/// 
	/// @param x The control point position.
	/// @param opacity The control point color.
	void add_opacity_point(float x, float opacity);

	/// @brief Remove the color point at position x if it exists.
	/// 
	/// @param x The position of the control point to be removed.
	/// @return True if a point was removed, false otherwise.
	bool remove_color_point(float x);

	/// @brief Remove the opacity point at position x if it exists.
	/// 
	/// @param x The position of the control point to be removed.
	/// @return True if a point was removed, false otherwise.
	bool remove_opacity_point(float x);

	/// @brief Set the domain.
	/// All control points outside of the new domain will be removed. New control points
	/// will be created at the ends of the new domain if they don't already exist. Affects
	/// color and opacity functions.
	/// 
	/// @param domain The new domain.
	void set_domain(cgv::vec2 domain) override;

	/// @brief Rescale the color and opactiy functions to the new domain.
	/// All control point positions are updated proportionally to fit inside the new domain.
	/// 
	/// @param domain The new domain.
	void rescale(cgv::vec2 domain);

	/// See color_scale::normalize_value().
	float normalize_value(float value) const override;

	/// See color_scale::map_value().
	cgv::rgba map_value(float value) const override;

	/// See color_scale::get_mapped_color().
	cgv::rgb get_mapped_color(float value) const override;

	/// See color_scale::get_mapped_opacity().
	float get_mapped_opacity(float value) const override;

	/// See color_scale::quantize().
	std::vector<cgv::rgba> quantize(size_t count) const override;

	/// @brief Quantize the color function only.
	/// See quantize().
	std::vector<cgv::rgb> quantize_color(size_t count) const;

	/// @brief Quantize the opacity function only.
	/// See quantize().
	std::vector<float> quantize_opacity(size_t count) const;

	/// See color_scale::get_ticks().
	std::vector<float> get_ticks(size_t request_count) const override;

	/// @brief Clear all color and opacity control points.
	void clear();

	/// @brief Clear color control points only.
	void clear_color_points();

	/// @brief Clear opacity control points only.
	void clear_opacity_points();

	/// @brief Get the color control points.
	/// 
	/// @return The control points. 
	const std::vector<color_point_type>& get_color_points() const {
		return color_points_;
	}

	/// @brief Get the opacity control points.
	/// 
	/// @return The control points. 
	const std::vector<opacity_point_type>& get_opacity_points() const {
		return opacity_points_;
	}

	/// @brief Set the interpolation mode of the color and opacity function.
	/// 
	/// @param interpolation The interpolation mode.
	void set_interpolation(InterpolationMode interpolation);

	/// @brief Set the interpolation mode of the color function.
	/// 
	/// @param interpolation The interpolation mode.
	void set_color_interpolation(InterpolationMode interpolation);

	/// @brief Get the interpolation mode of the color function.
	/// 
	/// @param interpolation The interpolation mode.
	InterpolationMode get_color_interpolation() const {
		return color_interpolation_;
	}

	/// @brief Set the interpolation mode of the opacity function.
	/// 
	/// @param interpolation The interpolation mode.
	void set_opacity_interpolation(InterpolationMode interpolation);

	/// @brief Get the interpolation mode of the opacity function.
	/// 
	/// @param interpolation The interpolation mode.
	InterpolationMode set_opacity_interpolation() const {
		return color_interpolation_;
	}

private:
	/// @brief Helper template to remove a control point of any value type by its position if it exists.
	/// 
	/// @tparam value_type The control point value type.
	/// @param points The control point vector.
	/// @param x The position of the control point to be removed.
	/// @return True if a point was removed, false otherwise.
	template<typename value_type>
	bool remove_point(std::vector<std::pair<float, value_type>>& points, float x) {
		auto it = std::find_if(points.begin(), points.end(), [&x](const std::pair<float, value_type>& point) { return point.first == x; });
		if(it == points.end() || it == points.begin() || it == --points.end())
			return false;
		points.erase(it);
		return true;
	}

	/// @brief Helper template to ensure control points at the bounds of the domain.
	/// Does not alter the domain.
	/// 
	/// @tparam value_type The control point value type.
	/// @param points The control points.
	template<typename value_type>
	void ensure_control_points_cover_domain(std::vector<std::pair<float, value_type>>& points) {
		if(!points.empty()) {
			cgv::vec2 domain = get_domain();
			if(domain[0] < points.front().first)
				points.insert(points.begin(), { domain[0], points.front().second });
			if(domain[1] > points.back().first)
				points.push_back({ domain[1], points.back().second });
		}
	}

	/// @brief Helper template to sort a sequence of control points of any value type by their position.
	/// 
	/// @tparam value_type The control point value type.
	/// @param points The control points.
	template<typename value_type>
	void sort_points_and_update_domain(std::vector<std::pair<float, value_type>>& points) {
		std::sort(points.begin(), points.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
		// Only call modified if this did not already happen in update_domain.
		if(!update_domain())
			modified();
	}

	/// @brief Helper template to interpolate control points of any value type using a piecewise constant function.
	/// 
	/// @tparam value_type The control point value type.
	/// @param points The control points.
	/// @param x The position at which to evaluate the function.
	/// @return The interpolated value.
	template<typename value_type>
	value_type interpolate_step(const std::vector<std::pair<float, value_type>>& points, float x) const {
		return cgv::math::detail::interpolate_piece(points, x, [](const value_type& a, const value_type& b, float t_local) { return a; });
	}

	/// @brief Helper template to sample the interpolation of control points of any value type using a piecewise constant function.
	/// 
	/// @tparam value_type The control point value type.
	/// @param points The control points.
	/// @param n The number of samples at which to evaluate the function.
	/// @return The sequence of interpolated values.
	template<typename value_type>
	std::vector<value_type> interpolate_step_n(const std::vector<std::pair<float, value_type>>& points, size_t n) const {
		return cgv::math::detail::interpolate_n(points, [](const value_type& a, const value_type& b, float t_local) { return a; }, n);
	}

	/// @brief Helper template to interpolate control points of any value type using the given mode.
	/// 
	/// @tparam value_type The control point value type.
	/// @param points The control points.
	/// @param x The position at which to evaluate the function.
	/// @param interpolation The interpolation mode to use.
	/// @return The interpolated value.
	template<typename value_type>
	value_type interpolate(const std::vector<std::pair<float, value_type>>& points, float x, InterpolationMode interpolation) const {
		switch(interpolation) {
		case InterpolationMode::kStep:
			return interpolate_step(points, x);
		case InterpolationMode::kSmooth:
			return cgv::math::interpolate_smooth_cubic(points, x);
		default:
			return cgv::math::interpolate_linear(points, x);
		}
	}

	/// @brief Helper template to uniformly sample the interpolation of control points of any value type at n positions using teh given mode.
	/// 
	/// @tparam value_type The control point value type.
	/// @param points The control points.
	/// @param n The number of samples at which to evaluate the function.
	/// @param interpolation The interpolation mode to use.
	/// @return The sequence of interpolated values.
	template<typename value_type>
	std::vector<value_type> quantize(const std::vector<std::pair<float, value_type>>& points, size_t n, InterpolationMode interpolation) const {
		switch(interpolation) {
		case InterpolationMode::kStep:
			return interpolate_step_n(points, n);
		case InterpolationMode::kSmooth:
			return cgv::math::interpolate_smooth_cubic_n(points, n);
		default:
			return cgv::math::interpolate_linear_n(points, n);
		}
	}

	/// @brief Update the domain bounds to the control point position ranges.
	/// Sets the domain to excatly cover the minimum and maximum position among all color and opacity control points.
	/// Does not alter the control points.
	/// 
	/// @return True if the domain has changed, false otherwise.
	bool update_domain();

	/// The interpolation mode used for the color function.
	InterpolationMode color_interpolation_ = InterpolationMode::kLinear;
	/// The interpolation mode used for the opacity function.
	InterpolationMode opacity_interpolation_ = InterpolationMode::kLinear;
	/// The control points of the color function.
	std::vector<color_point_type> color_points_;
	/// The control points of the opacity function.
	std::vector<opacity_point_type> opacity_points_;
};

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
