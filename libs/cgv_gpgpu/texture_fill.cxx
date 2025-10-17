#include "texture_fill.h"

namespace cgv {
namespace gpgpu {

texture_fill::texture_fill() : texture_algorithm("texture_fill", { TextureType::TT_1D, TextureType::TT_2D, TextureType::TT_3D }) {}

bool texture_fill::init(cgv::render::context& ctx, cgv::render::TextureType texture_type, sl::ImageFormatLayoutQualifier image_format) {
	texture_algorithm_create_info info;
	info.typedefs.push_back({ "value_type", sl::get_data_type(image_format) });
	info.default_image_count = 1;
	info.texture_type = texture_type;
	info.image_format = image_format;
	return texture_algorithm::init(ctx, info, { { &_kernel, "gpgpu_texture_fill" } });
}

void texture_fill::destruct(const cgv::render::context& ctx) {
	texture_algorithm::destruct(ctx);
}

bool texture_fill::dispatch(cgv::render::context& ctx, cgv::render::texture& texture, const vec4& value) {
	if(!is_initialized_for_texture(texture))
		return false;

	bind_image_texture(ctx, texture, 0);

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
