#pragma once

#include "texture_algorithm.h"

#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for applying a function to each texture texel.
class CGV_API for_each_texture : public texture_algorithm {
public:
	for_each_texture();

	bool init(cgv::render::context& ctx, cgv::render::TextureType texture_type, const sl::named_variable_list& arguments, const std::string& unary_operation);

	bool dispatch(cgv::render::context& ctx, cgv::render::texture& texture, const uniform_binding_list& arguments);

private:
	cgv::render::shader_compile_options get_configuration(cgv::render::TextureType texture_type, const sl::named_variable_list& arguments, const std::string& unary_operation) const;

	compute_kernel kernel;
};

}
}

#include <cgv/config/lib_end.h>
