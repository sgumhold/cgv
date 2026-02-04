#pragma once

#include <algorithm>
#include <vector>

#include <cgv/math/interpolate.h>
#include <cgv/math/fvec.h>

#include "color.h"
#include "color_scale.h"

#include "lib_begin.h"

namespace cgv {
namespace media {

class CGV_API transfer_function : public color_scale {
public:
	using color_type = cgv::rgb;
	using opacity_type = float;
	using color_point_type = std::pair<float, color_type>;
	using opacity_point_type = std::pair<float, opacity_type>;

	transfer_function() {}

	transfer_function(std::initializer_list<color_point_type> colors);

	transfer_function(std::initializer_list<color_point_type> colors, std::initializer_list<opacity_point_type> opacities);

	bool is_opaque() const override {
		return opacity_points_.empty();
	}

	bool is_discrete() const override {
		return false;
	}

	bool empty() const {
		return color_points_.empty() && opacity_points_.empty();
	}

	// Todo: Add rescale function to rescale all poitns to a new domain.

	void set_color_points(const std::vector<color_point_type>& colors);

	void set_opacity_points(const std::vector<opacity_point_type>& opacities);

	void add_color_point(float t, const color_type& color);

	void add_opacity_point(float t, float opacity);

	bool remove_color_point(float t);

	bool remove_opacity_point(float t);

	void set_domain(cgv::vec2 domain) override;

	cgv::rgba get_mapped_value(float value) const override;

	cgv::rgb get_mapped_color(float value) const override;

	float get_mapped_opacity(float value) const override;

	std::vector<cgv::rgba> quantize(size_t n) const override;

	std::vector<cgv::rgb> quantize_color(size_t count) const;

	std::vector<float> quantize_opacity(size_t count) const;

	void clear();

	void clear_color_points();

	void clear_opacity_points();

	const std::vector<color_point_type>& get_color_points() const {
		return color_points_;
	}

	const std::vector<opacity_point_type>& get_opacity_points() const {
		return opacity_points_;
	}

	// Todo: Make private, use getter/setter and call modified.
	bool use_interpolation = true;

private:
	// Todo: Make step only affect color.

	template<typename value_type>
	bool remove_point(std::vector<std::pair<float, value_type>>& points, float t) {
		auto it = std::find_if(points.begin(), points.end(), [&t](const std::pair<float, value_type>& point) { return point.first == t; });
		if(it == points.end())
			return false;
		points.erase(it);
		return true;
	}

	template<typename value_type>
	void sort_points_and_update_domain(std::vector<std::pair<float, value_type>>& points) {
		std::sort(points.begin(), points.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
		// Only call modified if this did not already happen in update_domain.
		if(!update_domain())
			modified();
	}

	template<typename value_type>
	value_type interpolate_step(const std::vector<std::pair<float, value_type>>& points, float t) const {
		return cgv::math::detail::interpolate_piece(points, t, [](const value_type& a, const value_type& b, float t_local) { return a; });
	}

	template<typename value_type>
	std::vector<value_type> interpolate_step_n(const std::vector<std::pair<float, value_type>>& points, size_t n) const {
		return cgv::math::detail::interpolate_n(points, [](const value_type& a, const value_type& b, float t_local) { return a; }, n);
	}

	template<typename value_type>
	value_type interpolate(const std::vector<std::pair<float, value_type>>& points, float t) const {
		if(use_interpolation)
			return cgv::math::interpolate_linear(points, t);
		else
			return interpolate_step(points, t);
	}

	template<typename value_type>
	std::vector<value_type> quantize(const std::vector<std::pair<float, value_type>>& points, size_t n) const {
		if(use_interpolation)
			return cgv::math::interpolate_linear_n(points, n);
		else
			return interpolate_step_n(points, n);
	}

	bool update_domain();

	std::vector<color_point_type> color_points_;
	std::vector<opacity_point_type> opacity_points_;
};

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
