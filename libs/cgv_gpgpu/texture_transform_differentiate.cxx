#include "texture_transform_differentiate.h"

#include "texture_differentiate.h"

namespace cgv {
namespace gpgpu {

texture_transform_differentiate::texture_transform_differentiate() : texture_differentiate_base("texture_transform_differentiate", { TextureType::TT_1D, TextureType::TT_2D, TextureType::TT_3D }) {}

bool texture_transform_differentiate::init(
	cgv::render::context& ctx,
	cgv::render::TextureType texture_type,
	sl::ImageFormatLayoutQualifier image_format,
	WrapMode wrap_mode,
	const std::string& unary_operation,
	const argument_definitions& arguments,
	DifferentiationOperator differentiation_operator,
	DifferentiationOutput differentiation_output
) {
	sl::data_type scalar_type(sl::get_type_info(sl::get_data_type(image_format).type()).component_type);

	texture_algorithm_create_info info = get_create_info(texture_type, image_format, wrap_mode, differentiation_operator, differentiation_output);
	info.arguments = &arguments;
	info.typedefs.push_back({ "value_type", sl::get_data_type(image_format) });
	info.typedefs.push_back({ "scalar_type", scalar_type });
	info.options.define_macro("USE_TRANSFORM");
	info.options.define_snippet("operation", unary_operation);

	return texture_algorithm::init(ctx, info, { { &_kernel, "gpgpu_texture_differentiate" } });
}

void texture_transform_differentiate::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	algorithm::destruct(ctx);
}

bool texture_transform_differentiate::dispatch(cgv::render::context& ctx, cgv::render::texture& input_texture, cgv::render::texture& output_texture, const argument_bindings& arguments) {
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
	_kernel.set_argument(ctx, "u_border_value", input_texture.get_border_color());
	_kernel.set_arguments(ctx, arguments);
	bind_buffer_like_arguments(ctx, arguments);

	uvec3 num_groups = get_num_groups(input_size, _group_size);
	dispatch_compute(num_groups.x(), num_groups.y(), num_groups.z());
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	unbind_buffer_like_arguments(ctx, arguments);
	_kernel.disable(ctx);

	return true;
}

} // namespace gpgpu
} // namespace cgv
