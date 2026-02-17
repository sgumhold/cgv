#include "transfer_function.h"

#include <cgv/media/ticks.h>
#include <cgv/utils/algorithm.h>

namespace cgv {
namespace media {

transfer_function::transfer_function(std::initializer_list<color_point_type> colors) {
	set_color_points(colors);
}

transfer_function::transfer_function(std::initializer_list<color_point_type> colors, std::initializer_list<opacity_point_type> opacities) {
	set_color_points(colors);
	set_opacity_points(opacities);
}

void transfer_function::set_color_points(const std::vector<color_point_type>& colors) {
	color_points_ = colors;
	sort_points_and_update_domain(color_points_);
	ensure_domain(color_points_);
	ensure_domain(opacity_points_);
}

void transfer_function::set_opacity_points(const std::vector<opacity_point_type>& opacities) {
	opacity_points_ = opacities;
	std::for_each(opacity_points_.begin(), opacity_points_.end(), [](const opacity_point_type& point) { cgv::math::saturate(point.second); });
	sort_points_and_update_domain(opacity_points_);
	ensure_domain(opacity_points_);
	ensure_domain(color_points_);
}

void transfer_function::add_color_point(float t, const color_type& color) {
	remove_color_point(t);
	color_points_.push_back({ t, color });
	sort_points_and_update_domain(color_points_);
	ensure_domain(opacity_points_);
}

void transfer_function::add_opacity_point(float t, float opacity) {
	remove_opacity_point(t);
	opacity_points_.push_back({ t, cgv::math::saturate(opacity) });
	sort_points_and_update_domain(opacity_points_);
	ensure_domain(color_points_);
}

bool transfer_function::remove_color_point(float t) {
	if(remove_point(color_points_, t)) {
		modified();
		return true;
	}
	return false;
}

bool transfer_function::remove_opacity_point(float t) {
	if(remove_point(opacity_points_, t)) {
		modified();
		return true;
	}
	return false;
}

void transfer_function::set_domain(cgv::vec2 domain) {
	const vec2 current_domain = get_domain();

	// Make sure we have points at either end of the new domain.
	float lower_bound = current_domain[0] < domain[0] ? domain[0] : current_domain[0];
	float upper_bound = current_domain[1] > domain[1] ? domain[1] : current_domain[1];

	if(!color_points_.empty() && color_points_.front().first != lower_bound)
		add_color_point(domain[0], get_mapped_color(lower_bound));
	if(!opacity_points_.empty() && opacity_points_.front().first != lower_bound)
		add_opacity_point(domain[0], get_mapped_opacity(lower_bound));
	
	if(!color_points_.empty() && color_points_.back().first != upper_bound)
		add_color_point(domain[1], get_mapped_color(upper_bound));
	if(!opacity_points_.empty() && opacity_points_.back().first != upper_bound)
		add_opacity_point(domain[1], get_mapped_opacity(upper_bound));

	// Remove all points that are outside the new domain.
	const auto is_point_out_of_range = [domain](const auto& point) {
		return point.first < domain[0] || point.first > domain[1];
	};

	const auto remove_points_out_of_range = [domain, &is_point_out_of_range](auto& points) {
		bool done = false;
		bool removed = false;
		while(!done) {
			done = true;

			auto it = std::find_if(points.begin(), points.end(), is_point_out_of_range);
			if(it != points.end()) {
				points.erase(it);
				done = false;
				removed = true;
			}
		}
		return removed;
	};

	// Use temporary variables to ensure both functions are called which they otherwise may not due to short-circuit evaluation.
	bool removed_color_points = remove_points_out_of_range(color_points_);
	bool removed_opacity_points = remove_points_out_of_range(opacity_points_);
	if(removed_color_points || removed_opacity_points)
		modified();

	sort_points_and_update_domain(color_points_);
}

void transfer_function::rescale(cgv::vec2 domain) {
	const cgv::vec2 current_domain = get_domain();

	if(current_domain == domain)
		return;

	const auto map_point = [this, current_domain, domain](auto& point) {
		point.first = map_range_safe(point.first, current_domain[0], current_domain[1], domain[0], domain[1]);
	};

	if(!color_points_.empty())
		std::for_each(color_points_.begin(), color_points_.end(), map_point);

	if(!opacity_points_.empty())
		std::for_each(opacity_points_.begin(), opacity_points_.end(), map_point);

	color_scale::set_domain(domain);
}

float transfer_function::normalize_value(float value) const {
	const vec2 domain = get_domain();
	value = cgv::math::clamp(value, domain[0], domain[1]);
	return map_range_safe(value, domain[0], domain[1], 0.0f, 1.0f);
}

cgv::rgba transfer_function::map_value(float value) const {
	cgv::rgb color = get_mapped_color(value);
	float opacity = get_mapped_opacity(value);

	return { color.R(), color.G(), color.B(), opacity };
}

cgv::rgb transfer_function::get_mapped_color(float value) const {
	if(is_unknown(value) || color_points_.empty())
		return get_unknown_color();
	//const vec2 domain = get_domain();
	//value = cgv::math::clamp(value, domain[0], domain[1]);
	return interpolate(color_points_, value, color_interpolation_);
}

float transfer_function::get_mapped_opacity(float value) const {
	if(opacity_points_.empty())
		return 1.0f;
	return interpolate(opacity_points_, value, opacity_interpolation_);
}

std::vector<cgv::rgba> transfer_function::quantize(size_t n) const {
	const std::vector<cgv::rgb> colors = quantize_color(n);
	const std::vector<float> opacities = quantize_opacity(n);

	std::vector<cgv::rgba> values;
	values.reserve(n);
	std::transform(colors.begin(), colors.end(), opacities.begin(), std::back_inserter(values), [](const cgv::rgba& color, float opacity) {
		return cgv::rgba(color, opacity);
	});

	return values;
}

std::vector<cgv::rgb> transfer_function::quantize_color(size_t count) const {
	if(color_points_.empty())
		return std::vector<cgv::rgb>(count, { 0.0f });

	std::vector<cgv::rgb> colors = quantize(color_points_, count, color_interpolation_);
	if(is_reversed())
		std::reverse(colors.begin(), colors.end());
	return colors;
}

std::vector<float> transfer_function::quantize_opacity(size_t count) const {
	if(opacity_points_.empty())
		return std::vector<float>(count, 1.0f);
	std::vector<float> opacities = quantize(opacity_points_, count, opacity_interpolation_);
	if(is_reversed())
		std::reverse(opacities.begin(), opacities.end());
	return opacities;
}

std::vector<float> transfer_function::get_ticks(size_t request_count) const {
	/* This would return one tick for every color or opacity point.
	std::vector<float> ticks;
	if(!color_points_.empty())
		std::transform(color_points_.begin(), color_points_.end(), std::back_inserter(ticks), cgv::utils::get_first<>{});// [] (const color_point_type& point))
	else
		std::transform(opacity_points_.begin(), opacity_points_.end(), std::back_inserter(ticks), cgv::utils::get_first<>{});
	return ticks;
	*/
	const vec2 domain = get_domain();
	return compute_ticks(domain[0], domain[1], request_count);
}

void transfer_function::clear() {
	color_points_.clear();
	opacity_points_.clear();
	update_domain();
}

void transfer_function::clear_color_points() {
	color_points_.clear();
	update_domain();
}

void transfer_function::clear_opacity_points() {
	opacity_points_.clear();
	update_domain();
}

void transfer_function::set_interpolation(InterpolationMode interpolation) {
	if(color_interpolation_ != interpolation || opacity_interpolation_ != interpolation) {
		color_interpolation_ = interpolation;
		opacity_interpolation_ = interpolation;
		modified();
	}
}

void transfer_function::set_color_interpolation(InterpolationMode interpolation) {
	if(color_interpolation_ != interpolation) {
		color_interpolation_ = interpolation;
		modified();
	}
}

void transfer_function::set_opacity_interpolation(InterpolationMode interpolation) {
	if(opacity_interpolation_ != interpolation) {
		opacity_interpolation_ = interpolation;
		modified();
	}
}

bool transfer_function::update_domain() {
	cgv::vec2 old_domain = get_domain();

	cgv::vec2 domain = { 0.0f };
	if(!color_points_.empty()) {
		domain[0] = color_points_.front().first;
		domain[1] = color_points_.back().first;
	}
	if(!opacity_points_.empty()) {
		domain[0] = std::min(domain[0], opacity_points_.front().first);
		domain[1] = std::max(domain[1], opacity_points_.back().first);
	}

	if(old_domain != domain) {
		color_scale::set_domain(domain);
		return true;
	}
	return false;
}

} // namespace media
} // namespace cgv
