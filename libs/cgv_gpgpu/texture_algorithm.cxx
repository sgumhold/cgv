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
	cgv::render::shader_compile_options compile_options;// = algorithm::configure(info);// algorithm::get_configuration(info);
	compile_options.defines["LOCAL_SIZE_X"] = std::to_string(local_size.x());
	compile_options.defines["LOCAL_SIZE_Y"] = std::to_string(local_size.y());
	compile_options.defines["LOCAL_SIZE_Z"] = std::to_string(local_size.z());
	compile_options.defines["DIMS"] = std::to_string(dims);
	compile_options.defines["INDEX_TYPE"] = index_type.type_name();
	compile_options.defines["COORD_TYPE"] = coord_type.type_name();
	compile_options.defines["SAMPLER_TYPE"] = "sampler" + dims_suffix;
	compile_options.defines["IMAGE_TYPE"] = get_type_prefix(create_info.image_format) + "image" + dims_suffix;
	compile_options.defines["IMAGE_FORMAT"] = to_string(create_info.image_format);
	compile_options.defines["SIZE_GUARD(IDX, SIZE)"] = size_guard;

	return compile_options;
}

bool texture_algorithm::init(cgv::render::context& ctx, const texture_algorithm_create_info& create_info, const std::vector<compute_kernel_info>& kernel_infos) {
	if(!is_texture_type_supported(create_info.texture_type))
		return false;
	
	_texture_type = create_info.texture_type;

	cgv::render::shader_compile_options compile_options = get_compile_options(create_info);

	// TODO: write append_and_overwrite methjod for compile options and use it here and in algorithm.
	compile_options.snippets.insert(compile_options.snippets.end(), create_info.options.snippets.begin(), create_info.options.snippets.end());

	for(const auto& define : create_info.options.defines)
		compile_options.defines[define.first] = define.second;

	algorithm_create_info create_info2 = create_info;
	create_info2.options = compile_options;
		
	return algorithm::init(ctx, create_info2, kernel_infos);
}

bool texture_algorithm::is_initialized_for_texture(const cgv::render::texture& texture) const {
	return texture.tt == _texture_type;
}

uvec3 texture_algorithm::get_texture_size(const cgv::render::texture& texture) const {
	return {
		static_cast<uint32_t>(texture.get_width()),
		static_cast<uint32_t>(texture.get_height()),
		static_cast<uint32_t>(texture.get_depth())
	};
}

uvec3 texture_algorithm::get_num_groups(const uvec3& texture_size, uint32_t base_group_size) const {
	// TODO: Ensure base_group_size is equal to the local_size set in init (add second method that only takes the texture size and uses the local_size)
	uvec3 num_groups = cgv::math::div_round_up(texture_size, uvec3(base_group_size));
	// ensure at least one group is launched per dimension
	return max(num_groups, uvec3(1));
}

} // namespace gpgpu
} // namespace cgv
