#include "for_each_texture.h"

namespace cgv {
namespace gpgpu {

for_each_texture::for_each_texture() : texture_algorithm("for_each_texture", { TextureType::TT_1D, TextureType::TT_2D, TextureType::TT_3D }) {}

bool for_each_texture::init(cgv::render::context& ctx, cgv::render::TextureType texture_type, sl::ImageFormatLayoutQualifier image_format, const std::string& unary_operation) {
	return init(ctx, texture_type, image_format, {}, unary_operation);
}

bool for_each_texture::init(cgv::render::context& ctx, cgv::render::TextureType texture_type, sl::ImageFormatLayoutQualifier image_format, const argument_definitions& arguments, const std::string& unary_operation) {
	texture_algorithm_create_info info;
	info.arguments = &arguments;
	info.typedefs.push_back({ "value_type", sl::get_data_type(image_format) });
	info.default_image_count = 1;
	info.options.snippets.push_back({ "operation", unary_operation });
	info.texture_type = texture_type;
	info.image_format = image_format;
	return texture_algorithm::init(ctx, info, { { &_kernel, "gpgpu_for_each_texture"} });
}

void for_each_texture::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	algorithm::destruct(ctx);
}

bool for_each_texture::dispatch(cgv::render::context& ctx, cgv::render::texture& texture, const argument_bindings& arguments) {
	if(!is_initialized_for_texture(texture))
		return false;

	glActiveTexture(GL_TEXTURE0);
	texture.bind_as_image(ctx, 0);

	uvec3 size = get_texture_size(texture);
	const uint32_t group_size = 4;
	uvec3 num_groups = get_num_groups(size, group_size);

	_kernel.enable(ctx);
	_kernel.set_argument(ctx, "u_size", size);
	_kernel.set_arguments(ctx, arguments);
	bind_buffer_like_arguments(ctx, arguments);

	dispatch_compute(num_groups.x(), num_groups.y(), num_groups.z());
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	unbind_buffer_like_arguments(ctx, arguments);
	_kernel.disable(ctx);

	texture.disable(ctx);
	return true;
}

} // namespace gpgpu
} // namespace cgv
