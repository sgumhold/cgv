#include "mipmap.h"

namespace cgv {
namespace gpgpu {

void mipmap::destruct(const cgv::render::context& ctx) {

	mipmap1d_prog.destruct(ctx);
	mipmap2d_prog.destruct(ctx);
	mipmap3d_prog.destruct(ctx);
	
	is_initialized_ = false;
}

bool mipmap::load_shader_programs(cgv::render::context& ctx) {

	bool res = true;
	std::string where = "cgv::gpgpu::mipmap::load_shader_programs()";

	res = res && cgv::render::shader_library::load(ctx, mipmap1d_prog, "gpgpu_mipmap1d", true, where);
	res = res && cgv::render::shader_library::load(ctx, mipmap2d_prog, "gpgpu_mipmap2d", true, where);
	res = res && cgv::render::shader_library::load(ctx, mipmap3d_prog, "gpgpu_mipmap3d", true, where);

	return res;
}

bool mipmap::init(cgv::render::context& ctx) {

	is_initialized_ = false;

	if(!load_shader_programs(ctx))
		return false;

	group_size = 4;

	is_initialized_ = true;
	return true;
}

bool mipmap::execute(cgv::render::context& ctx, cgv::render::texture& source_texture) {

	if(!(source_texture.tt == cgv::render::TextureType::TT_1D ||
	   source_texture.tt == cgv::render::TextureType::TT_2D ||
	   source_texture.tt == cgv::render::TextureType::TT_3D))
		return false;

	if(!source_texture.have_mipmaps)
		source_texture.create_mipmaps(ctx);

	unsigned num_dimensions = source_texture.get_nr_dimensions();

	cgv::render::shader_program* prog = nullptr;

	switch(num_dimensions) {
	case 1:
		prog = &mipmap1d_prog;
		break;
	case 2:
		prog = &mipmap2d_prog;
		break;
	case 3:
		prog = &mipmap3d_prog;
		break;
	default: break;
	}

	glActiveTexture(GL_TEXTURE0);
	source_texture.enable(ctx, 0);

	prog->enable(ctx);

	uvec3 size(source_texture.get_width(), source_texture.get_height(), source_texture.get_depth());

	unsigned max_size = cgv::math::max_value(size);
	unsigned num_levels = 1 + static_cast<unsigned>(log2(static_cast<float>(max_size)));

	uvec3 input_size = size;

	for(unsigned level = 0; level < num_levels - 1; ++level) {
		source_texture.bind_as_image(ctx, 1, level + 1);
		
		uvec3 output_size = size;
		float divisor = static_cast<float>(pow(2, level + 1));

		output_size = static_cast<uvec3>(static_cast<vec3>(output_size) / divisor);
		output_size = cgv::math::max(output_size, uvec3(1u));
		
		prog->set_uniform(ctx, "level", level);

		uvec3 num_groups = calculate_num_groups(output_size, uvec3(group_size));

		if(num_dimensions == 1) {
			prog->set_uniform(ctx, "output_size", output_size.x());
			num_groups[1] = 1;
			num_groups[2] = 1;
		} else if(num_dimensions == 2) {
			prog->set_uniform(ctx, "output_size", uvec2(output_size));
			num_groups[2] = 1;
		} else if(num_dimensions == 3) {
			prog->set_uniform(ctx, "output_size", output_size);
		}

		dispatch_compute3d(num_groups);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		input_size = output_size;
	}

	prog->disable(ctx);

	source_texture.disable(ctx);
	return true;
}

} // namespace gpgpu
} // namespace cgv
