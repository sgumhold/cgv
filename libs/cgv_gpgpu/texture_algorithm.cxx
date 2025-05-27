#include "texture_algorithm.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

bool texture_algorithm::is_texture_type_supported(TextureType texture_type) const {
	return std::find(_supported_texture_types.begin(), _supported_texture_types.end(), texture_type) != _supported_texture_types.end();
}

bool texture_algorithm::init(cgv::render::context& ctx, TextureType texture_type, const std::vector<compute_kernel_info>& kernel_infos, const cgv::render::shader_compile_options& config) {
	if(is_texture_type_supported(texture_type)) {
		_texture_type = texture_type;
		return algorithm::init(ctx, kernel_infos, config);
	}
	return false;
}

bool texture_algorithm::is_initialized_for_texture(const cgv::render::texture& texture) const {
	return texture.tt == _texture_type;
}

cgv::render::shader_compile_options texture_algorithm::get_configuration(TextureType texture_type, const argument_definitions& arguments) const {
	uint32_t dims = 0;
	sl::data_type index_type = sl::Type::kVoid;
	sl::data_type coord_type = sl::Type::kVoid;
	uvec3 local_size = { 1, 1, 1 };
	std::string size_guard;

	switch(texture_type) {
	case TextureType::TT_1D:
		dims = 1;
		index_type = sl::Type::kInt;
		coord_type = sl::Type::kFloat;
		local_size = { 4, 1, 1 };
		size_guard = "((IDX) < (SIZE).x)";
		break;
	case TextureType::TT_2D:
		dims = 2;
		index_type = sl::Type::kIVec2;
		coord_type = sl::Type::kVec2;
		local_size = { 4, 4, 1 };
		size_guard = "((IDX).x < (SIZE).x && (IDX).y < (SIZE).y)";
		break;
	case TextureType::TT_3D:
		dims = 3;
		index_type = sl::Type::kIVec3;
		coord_type = sl::Type::kVec3;
		local_size = { 4, 4, 4 };
		size_guard = "((IDX).x < (SIZE).x && (IDX).y < (SIZE).y && (IDX).z < (SIZE).z)";
		break;
	default:
		break;
	}

	// TODO: use larger group size for lower dimensional textures (try to aim for total occupancy of Streaming multiprocessor, e.g. 64 for RTX 2080)
	cgv::render::shader_compile_options config = algorithm::get_configuration(arguments);
	config.defines["LOCAL_SIZE_X"] = std::to_string(local_size.x());
	config.defines["LOCAL_SIZE_Y"] = std::to_string(local_size.y());
	config.defines["LOCAL_SIZE_Z"] = std::to_string(local_size.z());
	config.defines["DIMS"] = std::to_string(dims);
	config.defines["INDEX_TYPE"] = index_type.type_name();
	config.defines["COORD_TYPE"] = coord_type.type_name();
	std::string dims_suffix = std::to_string(dims) + "D";
	config.defines["SAMPLER_TYPE"] = "sampler" + dims_suffix;
	config.defines["IMAGE_TYPE"] = "image" + dims_suffix;
	config.defines["SIZE_GUARD(IDX, SIZE)"] = size_guard;

	return config;
}

uvec3 texture_algorithm::get_texture_size(const cgv::render::texture& texture) const {
	return {
		static_cast<uint32_t>(texture.get_width()),
		static_cast<uint32_t>(texture.get_height()),
		static_cast<uint32_t>(texture.get_depth())
	};
}

uvec3 texture_algorithm::get_num_groups(const uvec3& texture_size, uint32_t base_group_size) const {
	uvec3 num_groups = cgv::math::div_round_up(texture_size, uvec3(base_group_size));
	// ensure at least one group is launched per dimension
	return max(num_groups, uvec3(1));
}

} // namespace gpgpu
} // namespace cgv
