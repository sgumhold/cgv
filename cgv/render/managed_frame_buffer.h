#pragma once

#include <unordered_map>

#include <cgv/render/frame_buffer.h>

#include "lib_begin.h"

namespace cgv {
namespace render {

/// provides a class that manages a frame buffer along with its attachments
class CGV_API managed_frame_buffer
{
private:
	ivec2 get_actual_size(context& ctx);

	bool create_and_validate(context& ctx, const ivec2& size);

protected:
	struct attachment {
		unsigned index;
		bool attach;
		std::string format;
		TextureFilter tf;
		TextureWrap tw;
		texture tex;

		bool is_depth_attachment() const {
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
	ivec2 size;

public:
	managed_frame_buffer();

	void destruct(const context& ctx);

	ivec2 get_size();

	/// @brief Set the size of the framebuffer attachments. Requires to call ensure afterwards to actually construct the framebuffer.
	/// 
	/// If the given width or height is <= 0, the attachment widths or heights will be set to the context width or height respectively.
	/// 
	/// @param size The requested size.
	void set_size(const ivec2& size);

	void add_attachment(const std::string& name, const std::string& format = "uint8[R,G,B]", TextureFilter tf = TF_NEAREST, TextureWrap tw = TW_CLAMP_TO_EDGE, bool attach = true);

	bool enable_attachment(context& ctx, const std::string& name, int tex_unit = -1);

	bool disable_attachment(context& ctx, const std::string& name);

	texture* attachment_texture_ptr(const std::string& name);

	/// @brief Ensure the framebuffer is constructed with the specified size.
	/// 
	/// @param ctx The graphics context.
	/// @return True if the framebuffer could be constructed wit the specified attachments and size, false otherwise.
	bool ensure(context& ctx);

	bool enable(context& ctx, bool push_viewport=true);
	
	bool disable(context& ctx, bool pop_viewport=true);
	
	frame_buffer& ref_frame_buffer() { return fb; }
};

}
}

#include <cgv/config/lib_end.h>
