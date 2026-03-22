#include "device_color_scale.h"

namespace cgv {
namespace render {

device_color_scale_arguments device_color_scale::get_arguments() const {
	const cgv::media::color_scale* color_scale = get_color_scale();
	device_color_scale_arguments arguments;
	// Set arguments that are available to every color scale through the base class
	// and then update the implementation-specific fields.
	arguments.unknown_color = color_scale->get_unknown_color();
	arguments.domain = color_scale->get_domain();
	if(color_scale->is_clamped())
		arguments.mapping_options |= DeviceColorScaleMappingOptions::kClamped;
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
	out_arguments.transform = static_cast<uint16_t>(color_scale->get_transform());
	if(color_scale->is_diverging())
		out_arguments.mapping_options |= DeviceColorScaleMappingOptions::kDiverging;
	out_arguments.midpoint = color_scale->get_midpoint();
	out_arguments.exponent = color_scale->get_pow_exponent();

	// Continuous color scales with logarithmic transform use precomputed values to make mapping calculations more efficient.
	// However, these invariants are private so we need to recompute them here.
	if(color_scale->get_transform() == cgv::media::ContinuousMappingTransform::kLog) {
		vec2 domain = color_scale->get_domain();
		out_arguments.log_sign = domain[0] < 0.0f && domain[1] < 0.0f ? -1.0f : 1.0f;
		out_arguments.log_base = std::log(color_scale->get_log_base());
		out_arguments.log_midpoint = std::log(out_arguments.log_sign * color_scale->get_midpoint()) / out_arguments.log_base;
		out_arguments.log_lower_bound = std::log(out_arguments.log_sign * domain[0]) / out_arguments.log_base;
		out_arguments.log_upper_bound = std::log(out_arguments.log_sign * domain[1]) / out_arguments.log_base;
	}
}

void device_discrete_color_scale::update_color_scale_specific_arguments(device_color_scale_arguments& out_arguments) const {
	size_t count = std::min(color_scale->get_indexed_color_count(), device_color_scale_arguments::k_max_indexed_color_count);
	out_arguments.indexed_color_count = static_cast<uint8_t>(count);
	out_arguments.sample_mode = DeviceColorScaleSampleMode::kDiscrete;
}

} // namespace render
} // namespace cgv
