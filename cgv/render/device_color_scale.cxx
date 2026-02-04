#include "device_color_scale.h"

namespace cgv {
namespace render {

device_color_scale_arguments device_color_scale::get_arguments() const {
	const auto& to_vec = [](const cgv::rgba& color) {
		return cgv::vec4(color.R(), color.G(), color.B(), color.alpha());
	};

	const cgv::media::color_scale* color_scale = get_color_scale();
	device_color_scale_arguments arguments;
	arguments.unknown_color = to_vec(color_scale->get_unknown_color());
	arguments.domain = color_scale->get_domain();
	arguments.clamped = color_scale->is_clamped();
	update_color_scale_specific_arguments(arguments);
	return arguments;
}

std::vector<cgv::rgba> device_color_scale::get_texture_data(size_t texture_resolution) const {
	auto color_scale = get_color_scale();
	if(color_scale)
		return color_scale->quantize(texture_resolution);
	else
		return std::vector<cgv::rgba>(texture_resolution, { 0.0f });
}

void device_continuous_color_scale::update_color_scale_specific_arguments(device_color_scale_arguments& out_arguments) const {
	out_arguments.transform = static_cast<int>(color_scale->get_transform());
	out_arguments.diverging = color_scale->is_diverging();
	out_arguments.midpoint = color_scale->get_midpoint();
	out_arguments.exponent = color_scale->get_pow_exponent();

	if(color_scale->get_transform() == cgv::media::ContinuousMappingTransform::kLog) {
		vec2 domain = color_scale->get_domain();
		out_arguments.log_sign = domain[0] < 0.0f && domain[1] < 0.0f ? -1.0f : 1.0f;
		out_arguments.log_base = std::log(color_scale->get_log_base());
		out_arguments.log_midpoint = std::log(out_arguments.log_sign * color_scale->get_midpoint()) / out_arguments.log_base;
		out_arguments.log_lower_bound = std::log(out_arguments.log_sign * domain[0]) / out_arguments.log_base;
		out_arguments.log_upper_bound = std::log(out_arguments.log_sign * domain[1]) / out_arguments.log_base;
	}
}

} // namespace render
} // namespace cgv
