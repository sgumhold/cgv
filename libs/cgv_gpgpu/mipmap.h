#pragma once

#include "gpu_algorithm.h"

#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/** GPU compute shader implementation for computing texture mipmaps. */
class CGV_API mipmap : public gpu_algorithm {
protected:
	std::string image_format = "r8";
	unsigned component_count = 1;

	unsigned group_size = 0;
	
	/// shader programs
	cgv::render::shader_program mipmap2d_prog;
	cgv::render::shader_program mipmap3d_prog;
	
	bool load_shader_programs(cgv::render::context& ctx);

	void compute_mipmaps2d(cgv::render::context& ctx, cgv::render::texture& source_texture);

	void compute_mipmaps3d(cgv::render::context& ctx, cgv::render::texture& source_texture);

public:
	mipmap() : gpu_algorithm() {}

	void destruct(const cgv::render::context& ctx);

	bool init(cgv::render::context& ctx, size_t count) { return init(ctx); }

	bool init(cgv::render::context& ctx);

	void execute(cgv::render::context& ctx, cgv::render::texture& source_texture);

	/** Specifies the format of the texture consisting of component type and count.
		The image_format is used as the layout parameter in the shader and must be
		one of the supported OpenGL image variable formats. The component count
		specifies the number of texture channels. The image_format and component_count
		must match.
		Example: for a two-channel 8 bit texture interpreted as float use: ("rg8", 2).*/
	void set_image_format_and_component_count(const std::string& image_format, unsigned component_count) { this->image_format = image_format; this->component_count = component_count; }
};

}
}

#include <cgv/config/lib_end.h>
