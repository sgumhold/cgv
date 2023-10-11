#pragma once

#include "gpu_algorithm.h"

#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/** GPU compute shader implementation for clamping texture values. */
class CGV_API clamp_texture : public gpu_algorithm {
protected:
	unsigned group_size = 0;
	
	/// shader programs
	cgv::render::shader_program clamp_texture1d_prog;
	cgv::render::shader_program clamp_texture2d_prog;
	cgv::render::shader_program clamp_texture3d_prog;
	
	// configuration
	std::string texture_format = "r32f";
	vec2 range = vec2(0.0f, 1.0f);

	bool load_shader_programs(cgv::render::context& ctx);

public:
	clamp_texture() : gpu_algorithm() {}

	void destruct(const cgv::render::context& ctx);

	bool init(cgv::render::context& ctx, size_t count) { return init(ctx); }

	bool init(cgv::render::context& ctx);

	bool execute(cgv::render::context& ctx, cgv::render::texture& texture);

	void set_texture_format(const std::string& format) { texture_format = format; }

	void set_range(const vec2& range) { this->range = range; }
};

}
}

#include <cgv/config/lib_end.h>
