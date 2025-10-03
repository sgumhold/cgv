#pragma once

#include <cgv/render/texture.h>

#include "algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// Extension of the default GPU algorithm that defines common helper functions used for texture targets
class CGV_API texture_algorithm : public algorithm {
protected:
	using TextureType = cgv::render::TextureType;
	
public:
	texture_algorithm(const std::string& type_name, std::initializer_list<TextureType> supported_texture_types) : algorithm(type_name) {
		_supported_texture_types = supported_texture_types;
		// Overwrite default value of group size from algorithm to ensure correct group count.
		_group_size = 4u;
	}

	bool is_texture_type_supported(TextureType texture_type) const;

protected:
	struct texture_algorithm_create_info : public algorithm_create_info {
		TextureType texture_type = TextureType::TT_UNDEF;
		sl::ImageFormatLayoutQualifier image_format = sl::ImageFormatLayoutQualifier::k_rgba8;
	};

	cgv::render::shader_compile_options get_compile_options(const texture_algorithm_create_info& create_info);

	bool init(cgv::render::context& ctx, const texture_algorithm_create_info& create_info, const std::vector<compute_kernel_info>& kernel_infos);

	bool is_initialized_for_texture(const cgv::render::texture& texture) const;

	static uint32_t get_texture_type_dimensionality(TextureType texture_type);
	static uvec3 get_texture_size(const cgv::render::texture& texture);
	static uvec3 get_num_groups(const uvec3& texture_size, uint32_t base_group_size);

	bool bind_image_texture(cgv::render::context& ctx, cgv::render::texture& texture, int unit, int level = 0, cgv::render::AccessType access_type = cgv::render::AccessType::AT_WRITE_ONLY) const;

private:
	std::vector<TextureType> _supported_texture_types;

	/// The texture type used during initialization. Only textures of this type are supported by this instance of the texture_algorithm.
	TextureType _texture_type = TextureType::TT_UNDEF;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
