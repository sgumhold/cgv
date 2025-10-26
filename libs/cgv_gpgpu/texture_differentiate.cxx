#include "texture_differentiate.h"

namespace cgv {
namespace gpgpu {

std::string to_string(DifferentiationOperator differentiation_operator) {
	switch(differentiation_operator) {
	case DifferentiationOperator::kForwardDifference:
		return "forward_difference";
	case DifferentiationOperator::kBackwardDifference:
		return "backward_difference";
	case DifferentiationOperator::kCentralDifference:
		return "central_difference";
	case DifferentiationOperator::kSobel:
		return "sobel";
	default:
		return "";
	}
}

texture_algorithm::texture_algorithm_create_info texture_differentiate_base::get_create_info(cgv::render::TextureType texture_type,
																							 sl::ImageFormatLayoutQualifier image_format,
																							 WrapMode wrap_mode,
																							 DifferentiationOperator differentiation_operator,
																							 DifferentiationOutput differentiation_output) {
	uint32_t texture_dimensionality = get_texture_type_dimensionality(texture_type);
	std::string diff_operator_str = to_string(differentiation_operator) + std::to_string(texture_dimensionality) + "d";
	std::string diff_scaling_func = get_scaling_func(differentiation_operator, differentiation_output, texture_dimensionality);
	bool map_to_unorm = differentiation_output == DifferentiationOutput::kScaledDerivativeUNorm || differentiation_output != DifferentiationOutput::kNormalizedDerivativeUNorm;

	texture_algorithm_create_info info;
	info.typedefs.push_back({ "value_type", sl::get_data_type(image_format) });
	info.default_image_count = 2;
	info.texture_type = texture_type;
	info.image_format = image_format;
	info.options.define_macro("TEXTURE_WRAP_MODE", static_cast<int32_t>(wrap_mode));
	info.options.define_macro("DIFF_OPERATOR_FUNC", diff_operator_str);
	info.options.define_macro_if_not_default("DIFF_SCALING_FUNC", diff_scaling_func, "");
	info.options.define_macro_if_true(map_to_unorm, "MAP_TO_UNORM");
	info.options.define_macro_if_true(differentiation_output == DifferentiationOutput::kMagnitude, "OUTPUT_MAGNITUDE");

	return info;
}

std::string texture_differentiate_base::get_scaling_func(DifferentiationOperator differentiation_operator, DifferentiationOutput differentiation_output, uint32_t dimensions) {
	if(differentiation_output == DifferentiationOutput::kDerivative || differentiation_output == DifferentiationOutput::kMagnitude)
		return "";

	std::string res = "";

	switch(differentiation_operator) {
	case DifferentiationOperator::kCentralDifference:
		res = "central_difference";
		break;
	case DifferentiationOperator::kSobel:
		res = "sobel" + std::to_string(dimensions) + "d";
		break;
	default:
		break;
	}

	if(res.empty())
		return res;

	if(differentiation_output == DifferentiationOutput::kScaledDerivative || differentiation_output == DifferentiationOutput::kScaledDerivativeUNorm)
		return res + "_scaling";
	else if(differentiation_output == DifferentiationOutput::kNormalizedDerivative || differentiation_output == DifferentiationOutput::kNormalizedDerivativeUNorm)
		return res + "_norm";
	else
		return "";
}

texture_differentiate::texture_differentiate() : texture_differentiate_base("texture_differentiate", { TextureType::TT_1D, TextureType::TT_2D, TextureType::TT_3D }) {}

bool texture_differentiate::init(cgv::render::context& ctx,
								 cgv::render::TextureType texture_type,
								 sl::ImageFormatLayoutQualifier image_format,
								 WrapMode wrap_mode,
								 DifferentiationOperator differentiation_operator,
								 DifferentiationOutput differentiation_output) {
	texture_algorithm_create_info info = get_create_info(texture_type, image_format, wrap_mode, differentiation_operator, differentiation_output);
	return texture_algorithm::init(ctx, info, { { &_kernel, "gpgpu_texture_differentiate" } });
}

void texture_differentiate::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	algorithm::destruct(ctx);
}

bool texture_differentiate::dispatch(cgv::render::context& ctx, cgv::render::texture& input_texture, cgv::render::texture& output_texture, TextureChannel texture_channel) {
	if(!is_initialized_for_texture(input_texture) || !is_initialized_for_texture(output_texture))
		return false;

	uvec3 input_size = get_texture_size(input_texture);
	uvec3 output_size = get_texture_size(output_texture);

	if(input_size != output_size)
		return false;

	bind_image_texture(ctx, input_texture, 0, 0, cgv::render::AccessType::AT_READ_ONLY);
	bind_image_texture(ctx, output_texture, 1, 0, cgv::render::AccessType::AT_WRITE_ONLY);

	_kernel.enable(ctx);
	_kernel.set_argument(ctx, "u_size", input_size);
	_kernel.set_argument<int32_t>(ctx, "u_channel", texture_channel);
	_kernel.set_argument(ctx, "u_border_value", input_texture.get_border_color());

	uvec3 num_groups = get_num_groups(input_size, _group_size);
	dispatch_compute(num_groups.x(), num_groups.y(), num_groups.z());
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	_kernel.disable(ctx);
	return true;
}

} // namespace gpgpu
} // namespace cgv
