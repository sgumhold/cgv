#include "mipmap.h"

namespace cgv {
namespace gpgpu {

mipmap::mipmap() : algorithm("mipmap") {
	register_kernel(kernel_1d, "gpgpu_mipmap1d");
	register_kernel(kernel_2d, "gpgpu_mipmap2d");
	register_kernel(kernel_3d, "gpgpu_mipmap3d");
}

bool mipmap::init(cgv::render::context& ctx) {
	cgv::render::shader_compile_options config;
	return init_kernels(ctx, config);
}

bool mipmap::dispatch(cgv::render::context& ctx, cgv::render::texture& texture) {
	if(!(texture.tt == cgv::render::TextureType::TT_1D ||
		texture.tt == cgv::render::TextureType::TT_2D ||
		texture.tt == cgv::render::TextureType::TT_3D))
		return false;

	if(!texture.have_mipmaps)
		texture.create_mipmaps(ctx);

	unsigned num_dimensions = texture.get_nr_dimensions();

	compute_kernel* kernel = nullptr;

	switch(num_dimensions) {
	case 1:
		kernel = &kernel_1d;
		break;
	case 2:
		kernel = &kernel_2d;
		break;
	case 3:
		kernel = &kernel_3d;
		break;
	default: break;
	}

	glActiveTexture(GL_TEXTURE0);
	texture.enable(ctx, 0);

	kernel->enable(ctx);

	uvec3 size(texture.get_width(), texture.get_height(), texture.get_depth());

	unsigned max_size = cgv::math::max_value(size);
	unsigned num_levels = 1 + static_cast<unsigned>(log2(static_cast<float>(max_size)));

	uvec3 input_size = size;

	for(unsigned level = 0; level < num_levels - 1; ++level) {
		texture.bind_as_image(ctx, 1, level + 1);

		uvec3 output_size = size;
		float divisor = static_cast<float>(pow(2, level + 1));

		output_size = static_cast<uvec3>(static_cast<vec3>(output_size) / divisor);
		output_size = cgv::math::max(output_size, uvec3(1u));

		// TODO: add u_suffix to uniforms
		kernel->set_argument(ctx, "level", level);

		const uint32_t group_size = 4;
		uvec3 num_groups = div_round_up(output_size, uvec3(group_size));

		if(num_dimensions == 1) {
			kernel->set_argument(ctx, "output_size", output_size.x());
			num_groups[1] = 1;
			num_groups[2] = 1;
		} else if(num_dimensions == 2) {
			kernel->set_argument(ctx, "output_size", uvec2(output_size));
			num_groups[2] = 1;
		} else if(num_dimensions == 3) {
			kernel->set_argument(ctx, "output_size", output_size);
		}

		dispatch_compute(num_groups.x(), num_groups.y(), num_groups.z());
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		input_size = output_size;
	}

	kernel->disable(ctx);

	texture.disable(ctx);
}

} // namespace gpgpu
} // namespace cgv
