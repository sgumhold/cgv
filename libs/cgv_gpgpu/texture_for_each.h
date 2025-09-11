#pragma once

#include "texture_algorithm.h"

#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for applying a function to each texture texel.
class CGV_API texture_for_each : public texture_algorithm {
public:
	texture_for_each();

	bool init(cgv::render::context& ctx, cgv::render::TextureType texture_type, sl::ImageFormatLayoutQualifier image_format, const std::string& unary_operation);
	bool init(cgv::render::context& ctx, cgv::render::TextureType texture_type, sl::ImageFormatLayoutQualifier image_format, const argument_definitions& arguments, const std::string& unary_operation);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, cgv::render::texture& texture, const argument_bindings& arguments = {});

private:
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
