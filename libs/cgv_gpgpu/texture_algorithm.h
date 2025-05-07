#pragma once

#include "algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// Extension of the default GPU algorithm that defines common helper functions used for texture targets
class CGV_API texture_algorithm : public algorithm {
	using TextureType = cgv::render::TextureType;
	
public:
	texture_algorithm(const std::string& type_name, std::initializer_list<TextureType> supported_texture_types) : algorithm(type_name) {
		_supported_texture_types = supported_texture_types;
	}

	bool is_texture_type_supported(TextureType texture_type) const;

protected:
	bool init(cgv::render::context& ctx, TextureType texture_type, const cgv::render::shader_compile_options& config);

	bool is_initialized_for_texture(const cgv::render::texture& texture) const;

	cgv::render::shader_compile_options get_configuration(TextureType texture_type, const argument_definitions& arguments) const;

	uvec3 get_texture_size(const cgv::render::texture& texture) const;
	uvec3 get_num_groups(const uvec3& texture_size, uint32_t base_group_size) const;

private:
	std::vector<TextureType> _supported_texture_types;

	/// The texture type used during initialization. Only textures of this type are supported by this instance of the texture_algorithm.
	TextureType _texture_type = TextureType::TT_UNDEF;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
