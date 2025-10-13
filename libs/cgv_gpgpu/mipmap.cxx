#include "mipmap.h"

namespace cgv {
namespace gpgpu {

mipmap::mipmap() : texture_algorithm("mipmap", { TextureType::TT_1D, TextureType::TT_2D, TextureType::TT_3D }) {}

bool mipmap::init(cgv::render::context& ctx, cgv::render::TextureType texture_type) {
	texture_algorithm_create_info info;
	info.default_image_count = 1;
	info.default_texture_count = 1;
	info.texture_type = texture_type;
	return texture_algorithm::init(ctx, info, { { &_kernel, "gpgpu_mipmap" } });
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

	texture.enable(ctx, 0);

	_kernel.enable(ctx);

	uvec3 size = get_texture_size(texture);
	unsigned max_size = cgv::math::max_value(size);
	int num_levels = 1 + static_cast<int>(log2(static_cast<float>(max_size)));
	
	uvec3 input_size = size;

	for(int level = 0; level < num_levels - 1; ++level) {
		bind_image_texture(ctx, texture, 1, level + 1);

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
