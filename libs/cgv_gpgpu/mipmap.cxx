#include "mipmap.h"

namespace cgv {
namespace gpgpu {

mipmap::mipmap() : texture_algorithm("mipmap", { TextureType::TT_1D, TextureType::TT_2D, TextureType::TT_3D }) {}

bool mipmap::init(cgv::render::context& ctx, cgv::render::TextureType texture_type) {
	cgv::render::shader_compile_options config = get_configuration(texture_type, {});
	return texture_algorithm::init(ctx, texture_type, { { &_kernel, "gpgpu_mipmap" } }, config);
}

void mipmap::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	algorithm::destruct(ctx);
}

bool mipmap::dispatch(cgv::render::context& ctx, cgv::render::texture& texture) {
	if(!is_initialized_for_texture(texture))
		return false;

	if(!texture.have_mipmaps)
		texture.create_mipmaps(ctx);

	glActiveTexture(GL_TEXTURE0);
	texture.enable(ctx, 0);

	_kernel.enable(ctx);

	uvec3 size = get_texture_size(texture);
	unsigned max_size = cgv::math::max_value(size);
	unsigned num_levels = 1 + static_cast<unsigned>(log2(static_cast<float>(max_size)));
	
	uvec3 input_size = size;

	for(unsigned level = 0; level < num_levels - 1; ++level) {
		texture.bind_as_image(ctx, 1, level + 1);

		uvec3 output_size = size;
		float divisor = static_cast<float>(pow(2, level + 1));

		output_size = uvec3(vec3(output_size) / divisor);
		output_size = cgv::math::max(output_size, uvec3(1));

		_kernel.set_argument(ctx, "u_level", level);

		const uint32_t group_size = 4;
		uvec3 num_groups = get_num_groups(output_size, group_size);

		_kernel.set_argument(ctx, "u_output_size", output_size);

		dispatch_compute(num_groups.x(), num_groups.y(), num_groups.z());
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		input_size = output_size;
	}

	_kernel.disable(ctx);

	texture.disable(ctx);
	return true;
}

} // namespace gpgpu
} // namespace cgv
