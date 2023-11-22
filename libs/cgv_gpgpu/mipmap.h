#pragma once

#include "gpu_algorithm.h"

#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/** GPU compute shader implementation for computing texture mipmaps. */
class CGV_API mipmap : public gpu_algorithm {
protected:
	unsigned group_size = 0;
	
	/// shader programs
	cgv::render::shader_program mipmap1d_prog;
	cgv::render::shader_program mipmap2d_prog;
	cgv::render::shader_program mipmap3d_prog;
	
	bool load_shader_programs(cgv::render::context& ctx);

public:
	mipmap() : gpu_algorithm() {}

	void destruct(const cgv::render::context& ctx);

	bool init(cgv::render::context& ctx, size_t count) { return init(ctx); }

	bool init(cgv::render::context& ctx);

	bool execute(cgv::render::context& ctx, cgv::render::texture& source_texture);
};

}
}

#include <cgv/config/lib_end.h>
