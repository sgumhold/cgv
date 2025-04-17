#include "texture_algorithm.h"

namespace cgv {
namespace gpgpu {

bool texture_algorithm::init(cgv::render::context& ctx, TextureType texture_type, const cgv::render::shader_compile_options& config) {
	if(is_texture_type_supported(texture_type)) {
		_texture_type = texture_type;
		return init_kernels(ctx, config);
	}
	return false;
}

bool texture_algorithm::is_texture_type_supported(TextureType texture_type) const {
	return std::find(_supported_texture_types.begin(), _supported_texture_types.end(), texture_type) != _supported_texture_types.end();
}

bool texture_algorithm::is_initialized_for_texture(const cgv::render::texture& texture) const {
	return texture.tt == _texture_type;
}

cgv::render::shader_compile_options texture_algorithm::get_configuration(TextureType texture_type) const {
	uvec3 local_size = { 1, 1, 1 };

	switch(texture_type) {
	case TextureType::TT_1D:
		local_size = { 4, 1, 1 };
		break;
	case TextureType::TT_2D:
		local_size = { 4, 4, 1 };
		break;
	case TextureType::TT_3D:
		local_size = { 4, 4, 4 };
		break;
	default:
		break;
	}

	cgv::render::shader_compile_options config;
	config.defines["LOCAL_SIZE_X"] = std::to_string(local_size.x());
	config.defines["LOCAL_SIZE_Y"] = std::to_string(local_size.y());
	config.defines["LOCAL_SIZE_Z"] = std::to_string(local_size.z());
	return config;
}

texture_algorithm::kernel_parameters_t texture_algorithm::get_kernel_launch_parameters(const cgv::render::texture& texture, uint32_t base_group_size) const {
	uvec3 texture_size = {
		static_cast<uint32_t>(texture.get_width()),
		static_cast<uint32_t>(texture.get_height()),
		static_cast<uint32_t>(texture.get_depth())
	};

	// TODO: use larger group size for lower dimensional textures (try to aim for total occupancy of Streaming multiprocessor, e.g. 64 for RTX 2080)
	uvec3 num_groups = div_round_up(texture_size, uvec3(base_group_size));
	// ensure at least one group is launched per dimension
	num_groups = max(num_groups, uvec3(1));

	const uint32_t max_size = std::numeric_limits<uint32_t>::max();

	switch(_texture_type) {
	case TextureType::TT_1D:
		texture_size.y() = max_size;
		texture_size.z() = max_size;
		break;
	case TextureType::TT_2D:
		texture_size.z() = max_size;
		break;
	}

	return { texture_size, num_groups };
}

} // namespace gpgpu
} // namespace cgv
