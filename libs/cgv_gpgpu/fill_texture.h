#pragma once

#include "gpu_algorithm.h"

#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/** GPU compute shader implementation for filling a texture with a constant value. */
class CGV_API fill_texture : public gpu_algorithm {
protected:
	unsigned group_size = 0;

	/// shader programs
	cgv::render::shader_program fill_texture1d_prog;
	cgv::render::shader_program fill_texture2d_prog;
	cgv::render::shader_program fill_texture3d_prog;

	// configuration
	vec4 value = vec4(0.0f);

	bool load_shader_programs(cgv::render::context& ctx);

public:
	fill_texture() : gpu_algorithm() {}

	void destruct(const cgv::render::context& ctx);

	bool init(cgv::render::context& ctx, size_t count) { return init(ctx); }

	bool init(cgv::render::context& ctx);

	bool execute(cgv::render::context& ctx, cgv::render::texture& texture);

	void set_value(const vec4& value) { this->value = value; }
};

}
}

#include <cgv/config/lib_end.h>
