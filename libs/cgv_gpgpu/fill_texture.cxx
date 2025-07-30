#include "fill_texture.h"

namespace cgv {
namespace gpgpu {

fill_texture::fill_texture() : texture_algorithm("fill_texture", { TextureType::TT_1D, TextureType::TT_2D, TextureType::TT_3D }) {}

bool fill_texture::init(cgv::render::context& ctx, cgv::render::TextureType texture_type) {
	cgv::render::shader_compile_options config = get_configuration(texture_type, {});
	return texture_algorithm::init(ctx, texture_type, { { &_kernel, "gpgpu_fill_texture" } }, config);
}

void fill_texture::destruct(const cgv::render::context& ctx) {
	texture_algorithm::destruct(ctx);
}

bool fill_texture::dispatch(cgv::render::context& ctx, cgv::render::texture& texture, const vec4& value) {
	if(!is_initialized_for_texture(texture))
		return false;

	glActiveTexture(GL_TEXTURE0);
	texture.bind_as_image(ctx, 0);

	uvec3 size = get_texture_size(texture);
	const uint32_t group_size = 4;
	uvec3 num_groups = get_num_groups(size, group_size);
	
	_kernel.enable(ctx);
	_kernel.set_argument(ctx, "u_size", size);
	_kernel.set_argument(ctx, "u_value", value);

	dispatch_compute(num_groups.x(), num_groups.y(), num_groups.z());
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	_kernel.disable(ctx);

	texture.disable(ctx);
	return true;
}

} // namespace gpgpu
} // namespace cgv
