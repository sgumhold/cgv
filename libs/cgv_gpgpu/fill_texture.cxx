#include "fill_texture.h"

namespace cgv {
namespace gpgpu {

void fill_texture::destruct(const cgv::render::context& ctx) {

	fill_texture1d_prog.destruct(ctx);
	fill_texture2d_prog.destruct(ctx);
	fill_texture3d_prog.destruct(ctx);

	is_initialized_ = false;
}

bool fill_texture::load_shader_programs(cgv::render::context& ctx) {

	bool res = true;
	std::string where = "cgv::gpgpu::fill_texture::load_shader_programs()";

	res = res && cgv::render::shader_library::load(ctx, fill_texture1d_prog, "gpgpu_fill_texture1d", true, where);
	res = res && cgv::render::shader_library::load(ctx, fill_texture2d_prog, "gpgpu_fill_texture2d", true, where);
	res = res && cgv::render::shader_library::load(ctx, fill_texture3d_prog, "gpgpu_fill_texture3d", true, where);

	return res;
}

bool fill_texture::init(cgv::render::context& ctx) {

	is_initialized_ = false;

	if(!load_shader_programs(ctx))
		return false;

	group_size = 4;

	is_initialized_ = true;
	return true;
}

bool fill_texture::execute(cgv::render::context& ctx, cgv::render::texture& texture) {

	if(!(texture.tt == cgv::render::TextureType::TT_1D ||
		 texture.tt == cgv::render::TextureType::TT_2D ||
		 texture.tt == cgv::render::TextureType::TT_3D))
		return false;

	unsigned num_dimensions = texture.get_nr_dimensions();

	cgv::render::shader_program* prog = nullptr;

	switch(num_dimensions) {
	case 1:
		prog = &fill_texture1d_prog;
		break;
	case 2:
		prog = &fill_texture2d_prog;
		break;
	case 3:
		prog = &fill_texture3d_prog;
		break;
	default: break;
	}

	glActiveTexture(GL_TEXTURE0);
	texture.bind_as_image(ctx, 0);

	uvec3 size(texture.get_width(), texture.get_height(), texture.get_depth());

	prog->enable(ctx);
	prog->set_uniform(ctx, "value", value);

	uvec3 num_groups = calculate_num_groups(size, uvec3(group_size));

	if(num_dimensions == 1) {
		prog->set_uniform(ctx, "size", size.x());
		dispatch_compute1d(num_groups.x());
	} else if(num_dimensions == 2) {
		prog->set_uniform(ctx, "size", uvec2(size));
		dispatch_compute2d(uvec2(num_groups));
	} else if(num_dimensions == 3) {
		prog->set_uniform(ctx, "size", size);
		dispatch_compute3d(num_groups);
	}

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	prog->disable(ctx);
	return true;
}

} // namespace gpgpu
} // namespace cgv
