#include "clamp_texture.h"

namespace cgv {
namespace gpgpu {

void clamp_texture::destruct(const cgv::render::context& ctx) {

	clamp_texture1d_prog.destruct(ctx);
	clamp_texture2d_prog.destruct(ctx);
	clamp_texture3d_prog.destruct(ctx);
	
	is_initialized_ = false;
}

bool clamp_texture::load_shader_programs(cgv::render::context& ctx) {

	bool res = true;
	std::string where = "cgv::gpgpu::clamp_texture::load_shader_programs()";

	cgv::render::shader_define_map defines;
	cgv::render::shader_code::set_define(defines, "TEXTURE_FORMAT", texture_format, "r32f");

	res = res && cgv::render::shader_library::load(ctx, clamp_texture1d_prog, "gpgpu_clamp_texture1d", defines, true, where);
	res = res && cgv::render::shader_library::load(ctx, clamp_texture2d_prog, "gpgpu_clamp_texture2d", defines, true, where);
	res = res && cgv::render::shader_library::load(ctx, clamp_texture3d_prog, "gpgpu_clamp_texture3d", defines, true, where);

	return res;
}

bool clamp_texture::init(cgv::render::context& ctx) {

	is_initialized_ = false;

	if(!load_shader_programs(ctx))
		return false;

	group_size = 4;

	is_initialized_ = true;
	return true;
}

bool clamp_texture::execute(cgv::render::context& ctx, cgv::render::texture& texture) {

	if(!(texture.tt == cgv::render::TextureType::TT_1D ||
	   texture.tt == cgv::render::TextureType::TT_2D ||
	   texture.tt == cgv::render::TextureType::TT_3D))
		return false;

	unsigned num_dimensions = texture.get_nr_dimensions();

	cgv::render::shader_program* prog = nullptr;

	switch(num_dimensions) {
	case 1:
		prog = &clamp_texture1d_prog;
		break;
	case 2:
		prog = &clamp_texture2d_prog;
		break;
	case 3:
		prog = &clamp_texture3d_prog;
		break;
	default: break;
	}

	glActiveTexture(GL_TEXTURE0);
	texture.bind_as_image(ctx, 0, 0, false, 0, cgv::render::AccessType::AT_READ_WRITE);

	uvec3 size(texture.get_width(), texture.get_height(), texture.get_depth());

	prog->enable(ctx);
	prog->set_uniform(ctx, "range", range);

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
