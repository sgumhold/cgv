#include "mipmap.h"

namespace cgv {
namespace gpgpu {

void mipmap::destruct(const cgv::render::context& ctx) {

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

	GLenum texture_target = 0;
	cgv::render::shader_program* mipmap_prog = nullptr;

	switch(num_dimensions) {
	case 1:
		texture_target = GL_TEXTURE_1D;
		mipmap_prog = &mipmap1d_prog;
		break;
	case 2:
		texture_target = GL_TEXTURE_2D;
		mipmap_prog = &mipmap2d_prog;
		break;
	case 3:
		texture_target = GL_TEXTURE_3D;
		mipmap_prog = &mipmap3d_prog;
		break;
	default: break;
	}

	const unsigned texture_handle = (const unsigned&)source_texture.handle - 1;
	GLuint gl_format = (const GLuint&)source_texture.internal_format;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(texture_target, texture_handle);

	mipmap_prog->enable(ctx);

	uvec3 size(source_texture.get_width(), source_texture.get_height(), source_texture.get_depth());

	unsigned max_size = cgv::math::max_value(size);
	unsigned num_levels = 1 + static_cast<unsigned>(log2(static_cast<float>(max_size)));

	uvec3 input_size = size;

	for(unsigned level = 0; level < num_levels - 1; ++level) {
		glBindImageTexture(1, texture_handle, level + 1, GL_TRUE, 0, GL_WRITE_ONLY, gl_format);
		
		uvec3 output_size = size;
		float divisor = static_cast<float>(pow(2, level + 1));

		output_size = static_cast<uvec3>(static_cast<vec3>(output_size) / divisor);
		output_size = cgv::math::max(output_size, uvec3(1u));
		
		mipmap_prog->set_uniform(ctx, "level", level);

		uvec3 num_groups = calculate_num_groups(output_size, group_size);

		if(num_dimensions == 1) {
			mipmap_prog->set_uniform(ctx, "output_size", output_size.x());
			num_groups[1] = 1;
			num_groups[2] = 1;
		} else if(num_dimensions == 2) {
			mipmap_prog->set_uniform(ctx, "output_size", uvec2(output_size));
			num_groups[2] = 1;
		} else if(num_dimensions == 3) {
			mipmap_prog->set_uniform(ctx, "output_size", output_size);
		}

		dispatch_compute3d(num_groups);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		input_size = output_size;
	}

	mipmap_prog->disable(ctx);

	glBindTexture(texture_target, 0);
}

} // namespace gpgpu
} // namespace cgv
