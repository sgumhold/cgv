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

	cgv::render::shader_define_map defines;
	cgv::render::shader_code::set_define(defines, "IMAGE_FORMAT", image_format, "");
	cgv::render::shader_code::set_define(defines, "NUM_COMPONENTS", component_count, 1);
	
	res = res && cgv::render::shader_library::load(ctx, mipmap3d_prog, "gpgpu_mipmap2d", defines, true, where);
	res = res && cgv::render::shader_library::load(ctx, mipmap3d_prog, "gpgpu_mipmap3d", defines, true, where);

	return res;
}

void mipmap::compute_mipmaps2d(cgv::render::context& ctx, cgv::render::texture& source_texture) {

	const unsigned texture_handle = (const unsigned&)source_texture.handle - 1;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_handle);

	mipmap2d_prog.enable(ctx);

	uvec2 size(source_texture.get_width(), source_texture.get_height());

	unsigned max_size = cgv::math::max_value(size);
	unsigned num_levels = 1 + static_cast<unsigned>(log2(static_cast<float>(max_size)));

	uvec2 input_size = size;

	for(unsigned level = 0; level < num_levels - 1; ++level) {
		glBindImageTexture(1, texture_handle, level + 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R8);

		uvec2 output_size = size;
		float divisor = static_cast<float>(pow(2, level + 1));

		output_size = static_cast<uvec2>(static_cast<vec2>(output_size) / divisor);
		output_size = cgv::math::max(output_size, uvec2(1u));

		mipmap2d_prog.set_uniform(ctx, "level", level);
		mipmap2d_prog.set_uniform(ctx, "output_size", output_size);

		GLuint work_groups_x = calculate_num_groups(output_size.x(), group_size);
		GLuint work_groups_y = calculate_num_groups(output_size.y(), group_size);

		glDispatchCompute(work_groups_x, work_groups_y, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		input_size = output_size;
	}

	mipmap2d_prog.disable(ctx);
}

void mipmap::compute_mipmaps3d(cgv::render::context& ctx, cgv::render::texture& source_texture) {

	const unsigned texture_handle = (const unsigned&)source_texture.handle - 1;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_handle);

	mipmap3d_prog.enable(ctx);

	uvec3 size(source_texture.get_width(), source_texture.get_height(), source_texture.get_depth());

	unsigned max_size = cgv::math::max_value(size);
	unsigned num_levels = 1 + static_cast<unsigned>(log2(static_cast<float>(max_size)));

	uvec3 input_size = size;

	for(unsigned level = 0; level < num_levels - 1; ++level) {
		// TODO: use correct image format!
		glBindImageTexture(1, texture_handle, level + 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R8);

		uvec3 output_size = size;
		float divisor = static_cast<float>(pow(2, level + 1));

		output_size = static_cast<uvec3>(static_cast<vec3>(output_size) / divisor);
		output_size = cgv::math::max(output_size, uvec3(1u));
		
		mipmap3d_prog.set_uniform(ctx, "level", level);
		mipmap3d_prog.set_uniform(ctx, "output_size", output_size);

		GLuint work_groups_x = calculate_num_groups(output_size.x(), group_size);
		GLuint work_groups_y = calculate_num_groups(output_size.y(), group_size);
		GLuint work_groups_z = calculate_num_groups(output_size.z(), group_size);

		glDispatchCompute(work_groups_x, work_groups_y, work_groups_z);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		input_size = output_size;
	}

	mipmap3d_prog.disable(ctx);
}

bool mipmap::init(cgv::render::context& ctx) {

	is_initialized_ = false;

	if(!load_shader_programs(ctx))
		return false;

	group_size = 4;

	is_initialized_ = true;
	return true;
}

void mipmap::execute(cgv::render::context& ctx, cgv::render::texture& source_texture) {

	if(!source_texture.have_mipmaps)
		source_texture.generate_mipmaps(ctx);

	if(source_texture.get_nr_dimensions() < 2)
		return;

	if(source_texture.get_nr_dimensions() == 2)
		compute_mipmaps2d(ctx, source_texture);
	else if(source_texture.get_nr_dimensions() == 3)
		compute_mipmaps3d(ctx, source_texture);
	else
		return;
}

} // namespace gpgpu
} // namespace cgv
