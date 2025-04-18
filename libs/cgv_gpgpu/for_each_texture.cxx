#include "for_each_texture.h"

#include <cgv_gpgpu/utils.h>

using namespace cgv::gpgpu;

namespace cgv {
namespace gpgpu {

for_each_texture::for_each_texture() : texture_algorithm("for_each_texture", { TextureType::TT_1D, TextureType::TT_2D, TextureType::TT_3D }) {
	register_kernel(kernel, "gpgpu_for_each_texture");
}

bool for_each_texture::init(cgv::render::context& ctx, cgv::render::TextureType texture_type, const sl::named_variable_list& arguments, const std::string& unary_operation) {
	cgv::render::shader_compile_options config = get_configuration(texture_type, arguments, unary_operation);
	return texture_algorithm::init(ctx, texture_type, config);
}

bool for_each_texture::dispatch(cgv::render::context& ctx, cgv::render::texture& texture, const compute_kernel_arguments& arguments) {
	if(!is_initialized_for_texture(texture))
		return false;

	glActiveTexture(GL_TEXTURE0);
	texture.bind_as_image(ctx, 0);

	uvec3 size = get_texture_size(texture);
	const uint32_t group_size = 4;
	uvec3 num_groups = get_num_groups(size, group_size);

	kernel.enable(ctx);
	kernel.set_argument(ctx, "u_size", size);
	kernel.set_arguments(ctx, arguments);

	dispatch_compute(num_groups.x(), num_groups.y(), num_groups.z());
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	kernel.disable(ctx);

	texture.disable(ctx);
	return true;
}

cgv::render::shader_compile_options for_each_texture::get_configuration(cgv::render::TextureType texture_type, const sl::named_variable_list& arguments, const std::string& unary_operation) const {
	cgv::render::shader_compile_options config = texture_algorithm::get_configuration(texture_type);
	config.snippets.push_back({ "operation", unary_operation });
	config.snippets.push_back({ "arguments", to_string(arguments, "uniform") });
	return config;
}

} // namespace gpgpu
} // namespace cgv
