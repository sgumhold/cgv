#include "managed_frame_buffer.h"

namespace cgv {
namespace render {

managed_frame_buffer::managed_frame_buffer() {

	index_counter = 0;
	size = ivec2(0);
}

void managed_frame_buffer::destruct(const context& ctx) {

	fb.destruct(ctx);

	index_counter = 0;
	for(auto it = attachments.begin(); it != attachments.end(); ++it) {
		attachment& a = (*it).second;
		a.tex.destruct(ctx);
	}
}

ivec2 managed_frame_buffer::get_size() {

	return ivec2(fb.get_width(), fb.get_height());
}

void managed_frame_buffer::set_size(const ivec2& size) {

	this->size = size;
}

void managed_frame_buffer::add_attachment(const std::string& name, const std::string& format, TextureFilter tf, TextureWrap tw, bool attach) {

	attachment a;
	a.attach = attach;
	a.format = format;
	a.tf = tf;
	a.tw = tw;

	if(a.is_depth_attachment()) {
		a.index = 0;
	} else {
		a.index = index_counter;
		++index_counter;
	}

	attachments.insert(std::make_pair(name, a));
}

bool managed_frame_buffer::enable_attachment(context& ctx, const std::string& name, int tex_unit) {

	auto elem = attachments.find(name);
	if(elem != attachments.end()) {
		attachment& a = (*elem).second;
		return a.tex.enable(ctx, tex_unit);
	}

	return false;
}

bool managed_frame_buffer::disable_attachment(context& ctx, const std::string& name) {

	auto elem = attachments.find(name);
	if(elem != attachments.end()) {
		attachment& a = (*elem).second;
		return a.tex.disable(ctx);
	}

	return false;
}

texture* managed_frame_buffer::attachment_texture_ptr(const std::string& name) {

	auto elem = attachments.find(name);
	if(elem != attachments.end()) {
		attachment& a = (*elem).second;
		return &a.tex;
	}

	return nullptr;
}

bool managed_frame_buffer::ensure(context& ctx)
{
	int max_size = ctx.get_device_capabilities().max_render_buffer_size;
	if(max_size > 0 && size.x() > max_size || size.y() > max_size) {
		std::cerr << "Error: managed_framebuffer::ensure: requested size exceeds maximum supported size" << std::endl;
		return false;
	}

	ivec2 actual_size = get_actual_size(ctx);
	if (!fb.is_created() || fb.get_width() != actual_size.x() || fb.get_height() != actual_size.y())
	{
		destruct(ctx);
		if(!create_and_validate(ctx, actual_size))
			std::cerr << "Error: managed_framebuffer::ensure: framebuffer not complete" << std::endl;
		return true;
	}
	return false;
}

bool managed_frame_buffer::enable(context& ctx, bool push_viewport) {
	if (push_viewport)
		fb.push_viewport(ctx);
	bool success = fb.enable(ctx);
	return success;
}

bool managed_frame_buffer::disable(context& ctx, bool pop_viewport) {
	auto result = fb.disable(ctx);
	if (pop_viewport)
		fb.pop_viewport(ctx);
	return result;
}

bool managed_frame_buffer::create_and_validate(context& ctx, const ivec2& size) {

	fb.create(ctx, size.x(), size.y());

	for(auto it = attachments.begin(); it != attachments.end(); ++it) {
		attachment& a = (*it).second;

		unsigned filter_specifier = (unsigned)a.tf;

		// even filter specifiers are nearest and odd are linear
		TextureFilter mag_filter = (filter_specifier & 1) ? TF_LINEAR : TF_NEAREST;
		// specifiers larger than 1 are using mipmaps
		bool use_mipmaps = filter_specifier > 1;

		a.tex = texture(a.format, mag_filter, a.tf, a.tw, a.tw);
		a.tex.create(ctx, TT_2D, size.x(), size.y());

		if(use_mipmaps)
			a.tex.generate_mipmaps(ctx);

		if(a.is_depth_attachment()) {
			fb.attach(ctx, a.tex);
		} else {
			if(a.attach) {
				fb.attach(ctx, a.tex, 0, a.index);
			}
		}
	}

	return fb.is_complete(ctx);
}

ivec2 managed_frame_buffer::get_actual_size(context& ctx) {

	ivec2 actual_size(size);
	if(size.x() <= 0)
		actual_size.x() = ctx.get_width();
	if(size.y() <= 0)
		actual_size.y() = ctx.get_height();
	return actual_size;
}

}
}
