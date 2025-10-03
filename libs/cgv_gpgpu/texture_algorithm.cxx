#include "texture_algorithm.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

bool texture_algorithm::is_texture_type_supported(TextureType texture_type) const {
	return std::find(_supported_texture_types.begin(), _supported_texture_types.end(), texture_type) != _supported_texture_types.end();
}

cgv::render::shader_compile_options texture_algorithm::get_compile_options(const texture_algorithm_create_info& create_info) {
	uint32_t dims = 0;
	sl::data_type index_type = sl::Type::kVoid;
	sl::data_type coord_type = sl::Type::kVoid;
	uvec3 local_size = { 1, 1, 1 };
	std::string size_guard;

	switch(create_info.texture_type) {
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

	std::string dims_suffix = std::to_string(dims) + "D";

	// TODO: use larger group size for lower dimensional textures (try to aim for total occupancy of Streaming multiprocessor, e.g. 64 for RTX 2080)
	cgv::render::shader_compile_options options;
	options.define_macro("LOCAL_SIZE_X", local_size.x());
	options.define_macro("LOCAL_SIZE_Y", local_size.y());
	options.define_macro("LOCAL_SIZE_Z", local_size.z());
	options.define_macro("DIMS", dims);
	options.define_macro("INDEX_TYPE", index_type.type_name());
	options.define_macro("COORD_TYPE", coord_type.type_name());
	options.define_macro("SAMPLER_TYPE", "sampler" + dims_suffix);
	options.define_macro("IMAGE_TYPE", get_type_prefix(create_info.image_format) + "image" + dims_suffix);
	options.define_macro("IMAGE_FORMAT", to_string(create_info.image_format));
	options.define_macro("SIZE_GUARD(IDX, SIZE)", size_guard);

	return options;
}

bool texture_algorithm::init(cgv::render::context& ctx, const texture_algorithm_create_info& create_info, const std::vector<compute_kernel_info>& kernel_infos) {
	if(!is_texture_type_supported(create_info.texture_type))
		return false;
	
	_texture_type = create_info.texture_type;

	cgv::render::shader_compile_options compile_options = get_compile_options(create_info);
	compile_options.extend(create_info.options, true);

	algorithm_create_info create_info2 = create_info;
	create_info2.options = compile_options;
		
	return algorithm::init(ctx, create_info2, kernel_infos);
}

bool texture_algorithm::is_initialized_for_texture(const cgv::render::texture& texture) const {
	return texture.tt == _texture_type;
}

uint32_t texture_algorithm::get_texture_type_dimensionality(TextureType texture_type) {
	switch(texture_type) {
	case TextureType::TT_1D: return 1;
	case TextureType::TT_2D: return 2;
	case TextureType::TT_3D: return 3;
	default: return 0;
	}
}

uvec3 texture_algorithm::get_texture_size(const cgv::render::texture& texture) {
	return {
		static_cast<uint32_t>(texture.get_width()),
		static_cast<uint32_t>(texture.get_height()),
		static_cast<uint32_t>(texture.get_depth())
	};
}

uvec3 texture_algorithm::get_num_groups(const uvec3& texture_size, uint32_t base_group_size) {
	// TODO: Ensure base_group_size is equal to the local_size set in init (add second method that only takes the texture size and uses the local_size)
	uvec3 num_groups = cgv::math::div_round_up(texture_size, uvec3(base_group_size));
	// ensure at least one group is launched per dimension
	return max(num_groups, uvec3(1));
}

bool texture_algorithm::bind_image_texture(cgv::render::context& ctx, cgv::render::texture& texture, int unit, int level, cgv::render::AccessType access_type) const {
	// 3d image textures must be bound as layered image textures
	return texture.bind_as_image(ctx, unit, level, _texture_type == TextureType::TT_3D, 0, access_type);
}

} // namespace gpgpu
} // namespace cgv
