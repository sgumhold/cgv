#include "for_each_texture.h"

namespace cgv {
namespace gpgpu {

for_each_texture::for_each_texture() : texture_algorithm("for_each_texture", { TextureType::TT_1D, TextureType::TT_2D, TextureType::TT_3D }) {
	register_kernel(_kernel, "gpgpu_for_each_texture");
}

bool for_each_texture::init(cgv::render::context& ctx, cgv::render::TextureType texture_type, const std::string& unary_operation) {
	return init(ctx, texture_type, {}, unary_operation);
}

bool for_each_texture::init(cgv::render::context& ctx, cgv::render::TextureType texture_type, const argument_definitions& arguments, const std::string& unary_operation) {
	cgv::render::shader_compile_options config = get_configuration(texture_type, arguments, unary_operation);
	return texture_algorithm::init(ctx, texture_type, config);
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
	bind_buffer_arguments(ctx, arguments);

	dispatch_compute(num_groups.x(), num_groups.y(), num_groups.z());
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	unbind_buffer_arguments(ctx, arguments);
	_kernel.disable(ctx);

	texture.disable(ctx);
	return true;
}

cgv::render::shader_compile_options for_each_texture::get_configuration(cgv::render::TextureType texture_type, const argument_definitions& arguments, const std::string& unary_operation) const {
	cgv::render::shader_compile_options config = texture_algorithm::get_configuration(texture_type, arguments);
	config.snippets.push_back({ "operation", unary_operation });
	return config;
}

} // namespace gpgpu
} // namespace cgv
