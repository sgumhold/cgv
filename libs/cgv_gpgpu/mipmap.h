#pragma once

#include "texture_algorithm.h"

#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for computing texture mipmaps.
class CGV_API mipmap : public texture_algorithm {
public:
	mipmap();

	bool init(cgv::render::context& ctx, cgv::render::TextureType texture_type);

	bool dispatch(cgv::render::context& ctx, cgv::render::texture& texture);

private:
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
