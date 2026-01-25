#pragma once

#include <algorithm>
#include <vector>

#include <cgv/math/interpolate.h>
#include <cgv/math/fvec.h>

#include "color.h"
#include "color_scale.h"

namespace cgv {
namespace media {

class transfer_function : public color_scale {
public:
	using color_type = cgv::rgb;
	using opacity_type = float;
	using color_point_type = std::pair<float, color_type>;
	using opacity_point_type = std::pair<float, opacity_type>;

	//enum class Interpolation {
	//	kNone,
	//	kLinear,
	//	kSmooth
	//};

	static std::unique_ptr<transfer_function> new_instance() {
		return std::unique_ptr<transfer_function>(new transfer_function());
	}

	bool is_opaque() const override {
		return opacity_points_.empty();
	}

	bool is_discrete() const override {
		return false;
	}

	transfer_function() {}

	transfer_function(std::initializer_list<color_point_type> colors) {
		set_color_points(colors);
	}

	transfer_function(std::initializer_list<color_point_type> colors, std::initializer_list<opacity_point_type> opacities) {
		set_color_points(colors);
		set_opacity_points(opacities);
	}

	// Todo: Rescale, reverse, to_log, to_linear?

	void set_color_points(const std::vector<color_point_type>& colors) {
		color_points_ = colors;
		sort_points(color_points_);
		update_domain();
	}

	void set_opacity_points(const std::vector<opacity_point_type>& opacities) {
		opacity_points_ = opacities;
		sort_points(opacity_points_);
		update_domain();
	}

	void add_color_point(float t, const color_type& color) {
		// Remove any previous point at this position.
		remove_color_point(t);

		color_points_.push_back({ t, color });
		sort_points(color_points_);
		update_domain();
	}

	void add_opacity_point(float t, float opacity) {
		// Remove any previous point at this position.
		remove_opacity_point(t);

		opacity_points_.push_back({ t, opacity });
		sort_points(opacity_points_);
		update_domain();
	}

	bool remove_color_point(float t) {
		return remove_point(color_points_, t);
	}

	bool remove_opacity_point(float t) {
		return remove_point(opacity_points_, t);
	}

	bool has_opacity() const {
		return !opacity_points_.empty();
	}

	cgv::vec2 get_domain() const {
		return domain_;
	}

	cgv::rgb get_color(float t) const {
		// Todo: Clamp t to domain if enabled. If not, check if t is outside domain and return unknown color.
		if(color_points_.empty())
			return { 0.0f };
		return interpolate(color_points_, t);
	}

	std::vector<cgv::rgb> quantize_color(size_t n) const {
		if(color_points_.empty())
			return std::vector<cgv::rgb>(n, { 0.0f });
		return quantize(color_points_, n);
	}

	float get_opacity(float t) const {
		if(opacity_points_.empty())
			return 1.0f;
		return interpolate(opacity_points_, t);
	}

	std::vector<float> quantize_opacity(size_t n) const {
		if(opacity_points_.empty())
			return std::vector<float>(n, 1.0f);
		return quantize(opacity_points_, n);
	}

	cgv::rgba get_value(float t) const {
		cgv::rgb color = get_color(t);
		float opacity = get_opacity(t);

		return { color.R(), color.G(), color.B(), opacity };
	}

	std::vector<cgv::rgba> quantize_value(size_t n) const {
		const std::vector<cgv::rgb> colors = quantize_color(n);
		const std::vector<float> opacities = quantize_opacity(n);

		std::vector<cgv::rgba> data;
		data.reserve(n);
		for(size_t i = 0; i < n; ++i) {
			const cgv::rgb& color = colors[i];
			data.push_back({ color.R(), color.G(), color.B(), opacities[i] });
		}

		return data;
	}

	void clear() {
		color_points_.clear();
		opacity_points_.clear();
	}

	void clear_color_points() {
		color_points_.clear();
	}

	void clear_opacity_points() {
		opacity_points_.clear();
	}

	const std::vector<color_point_type>& get_color_points() const {
		return color_points_;
	}

	const std::vector<opacity_point_type>& get_opacity_points() const {
		return opacity_points_;
	}

	cgv::rgba unknown_color = { 0.0f, 0.0f, 0.0f, 0.0f };
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
	void sort_points(std::vector<std::pair<float, value_type>>& points) {
		std::sort(points.begin(), points.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
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

	bool update_domain() {
		cgv::vec2 old_domain = domain_;

		domain_ = { 0.0f };
		if(!color_points_.empty()) {
			domain_[0] = color_points_.front().first;
			domain_[1] = color_points_.back().first;
		}
		if(!opacity_points_.empty()) {
			domain_[0] = std::min(domain_[0], opacity_points_.front().first);
			domain_[1] = std::max(domain_[1], opacity_points_.back().first);
		}

		if(old_domain != domain_) {
			// Todo: Update modified time.
			return true;
		}
		return false;
	}

	cgv::vec2 domain_ = { 0.0f };
	std::vector<color_point_type> color_points_;
	std::vector<opacity_point_type> opacity_points_;
};

} // namespace media
} // namespace cgv
