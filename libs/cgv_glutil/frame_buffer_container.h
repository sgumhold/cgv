#pragma once

#include <unordered_map>

#include <cgv/render/context.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/render_types.h>
#include <cgv/render/shader_program.h>

#include "lib_begin.h"

using namespace cgv::render;

namespace cgv {
namespace glutil {

/// provides a shader library that handles shader loading
class CGV_API frame_buffer_container : public render_types
{
protected:
	struct attachment {
		unsigned index;
		bool attach;
		std::string format;
		TextureFilter tf;
		texture texture;

		bool is_depth_attachment() {
			return
				format == "[D]" ||
				format == "uint16[D]" ||
				format == "uint32[D:24]" ||
				format == "uint32[D]";
		}
	};

	frame_buffer fb;
	unsigned index_counter = 0;
	std::unordered_map<std::string, attachment> attachments;
	uvec2 size;

	bool create_and_validate(context& ctx);

public:
	frame_buffer_container();

	~frame_buffer_container();

	void clear(context& ctx);

	// Set this to uvec2(0) to use the size from the context
	void set_size(uvec2 size) { this->size = size; }

	void add_attachment(const std::string& name, const std::string& format = "uint8[R,G,B]", TextureFilter tf = TF_NEAREST, bool attach = true);

	bool enable_attachment(context& ctx, const std::string& name, int tex_unit = -1);

	bool disable_attachment(context& ctx, const std::string& name);

	texture* attachment_texture_ptr(const std::string& name);

	bool ensure(context& ctx);

	bool enable(context& ctx);
	
	bool disable(context& ctx);

	frame_buffer& ref_frame_buffer() { return fb; }
};

}
}

#include <cgv/config/lib_end.h>
