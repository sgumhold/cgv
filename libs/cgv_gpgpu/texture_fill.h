#pragma once

#include "texture_algorithm.h"

#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for filling a texture with a constant value.
class CGV_API texture_fill : public texture_algorithm {
public:
	texture_fill();

	bool init(cgv::render::context& ctx, cgv::render::TextureType texture_type, sl::ImageFormatLayoutQualifier image_format);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, cgv::render::texture& texture, const vec4& value);

private:
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
