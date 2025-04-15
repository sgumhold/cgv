#pragma once

#include "algorithm.h"

#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for computing texture mipmaps.
class CGV_API mipmap : public algorithm {
public:
	mipmap();

	bool init(cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, cgv::render::texture& texture);

private:
	compute_kernel kernel_1d;
	compute_kernel kernel_2d;
	compute_kernel kernel_3d;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
