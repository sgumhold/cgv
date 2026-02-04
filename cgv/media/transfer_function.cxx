#include "transfer_function.h"

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
}

void transfer_function::set_opacity_points(const std::vector<opacity_point_type>& opacities) {
	opacity_points_ = opacities;
	std::for_each(opacity_points_.begin(), opacity_points_.end(), [](const opacity_point_type& point) { cgv::math::saturate(point.second); });
	sort_points_and_update_domain(opacity_points_);
}

void transfer_function::add_color_point(float t, const color_type& color) {
	remove_color_point(t);
	color_points_.push_back({ t, color });
	sort_points_and_update_domain(color_points_);
}

void transfer_function::add_opacity_point(float t, float opacity) {
	remove_opacity_point(t);
	opacity_points_.push_back({ t, cgv::math::saturate(opacity) });
	sort_points_and_update_domain(opacity_points_);
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
	// Todo: Set domain, update points and call modified.
	
	// cut off any points from below or above the new domain and add points at the new domain lower and upper bounds.

	//if(get_domain() != domain) {
	//	color_scale::set_domain()
	//	modified
	//}
}

cgv::rgba transfer_function::get_mapped_value(float value) const {
	cgv::rgb color = get_mapped_color(value);
	float opacity = get_mapped_opacity(value);

	return { color.R(), color.G(), color.B(), opacity };
}

cgv::rgb transfer_function::get_mapped_color(float value) const {
	if(is_unknown(value) || color_points_.empty())
		return get_unknown_color();
	const vec2 domain = get_domain();
	value = cgv::math::clamp(value, domain[0], domain[1]);
	return interpolate(color_points_, value);
}

float transfer_function::get_mapped_opacity(float value) const {
	if(opacity_points_.empty())
		return 1.0f;
	return interpolate(opacity_points_, value);
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

	std::vector<cgv::rgb> colors = quantize(color_points_, count);
	if(is_reversed())
		std::reverse(colors.begin(), colors.end());
	return colors;
}

std::vector<float> transfer_function::quantize_opacity(size_t count) const {
	if(opacity_points_.empty())
		return std::vector<float>(count, 1.0f);
	std::vector<float> opacities = quantize(opacity_points_, count);
	if(is_reversed())
		std::reverse(opacities.begin(), opacities.end());
	return opacities;
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
