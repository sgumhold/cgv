#include "frame_buffer_container.h"

namespace cgv {
namespace glutil {

frame_buffer_container::frame_buffer_container() {

	index_counter = 0;
	size = ivec2(0);
}

frame_buffer_container::~frame_buffer_container() {

	attachments.clear();
}

void frame_buffer_container::clear(context& ctx) {

	fb.destruct(ctx);

	index_counter = 0;
	for(auto it = attachments.begin(); it != attachments.end(); ++it) {
		attachment& a = (*it).second;
		a.texture.destruct(ctx);
	}
}

frame_buffer_container::ivec2 frame_buffer_container::get_size() {

	return ivec2(fb.get_width(), fb.get_height());
}

bool frame_buffer_container::set_size(const ivec2& size) {

	GLint max_render_buffer_size;
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &max_render_buffer_size);

	if(size.x() > max_render_buffer_size || size.y() > max_render_buffer_size) {
		this->size = ivec2(-1);
		return false;
	}
	this->size = size;
	return true;
}

void frame_buffer_container::add_attachment(const std::string& name, const std::string& format, TextureFilter tf, TextureWrap tw, bool attach) {

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

bool frame_buffer_container::enable_attachment(context& ctx, const std::string& name, int tex_unit) {

	auto elem = attachments.find(name);
	if(elem != attachments.end()) {
		attachment& a = (*elem).second;
		return a.texture.enable(ctx, tex_unit);
	}

	return false;
}

bool frame_buffer_container::disable_attachment(context& ctx, const std::string& name) {

	auto elem = attachments.find(name);
	if(elem != attachments.end()) {
		attachment& a = (*elem).second;
		return a.texture.disable(ctx);
	}

	return false;
}

texture* frame_buffer_container::attachment_texture_ptr(const std::string& name) {

	auto elem = attachments.find(name);
	if(elem != attachments.end()) {
		attachment& a = (*elem).second;
		return &a.texture;
	}

	return nullptr;
}

bool frame_buffer_container::ensure(context& ctx) {

	ivec2 actual_size = get_actual_size(ctx);

	if(!fb.is_created() || fb.get_width() != actual_size.x() || fb.get_height() != actual_size.y()) {
		clear(ctx);
		if(!create_and_validate(ctx, actual_size)) {
			std::cerr << "Error: fbo not complete" << std::endl;
			abort();
		}
		return true;
	}

	return false;
}

bool frame_buffer_container::enable(context& ctx) {

	bool success = fb.enable(ctx);
	fb.push_viewport(ctx);
	return success;
}

bool frame_buffer_container::disable(context& ctx) {

	fb.pop_viewport(ctx);
	return fb.disable(ctx);
}

bool frame_buffer_container::create_and_validate(context& ctx, const ivec2& size) {

	fb.create(ctx, size.x(), size.y());

	for(auto it = attachments.begin(); it != attachments.end(); ++it) {
		attachment& a = (*it).second;

		unsigned filter_specifier = (unsigned)a.tf;

		// even filter specifiers are nearest and odd are linear
		TextureFilter mag_filter = (filter_specifier & 1) ? TF_LINEAR : TF_NEAREST;
		// specifiers larger than 1 are using mipmaps
		bool use_mipmaps = filter_specifier > 1;

		a.texture = texture(a.format, mag_filter, a.tf, a.tw, a.tw);
		a.texture.create(ctx, TT_2D, size.x(), size.y());

		if(use_mipmaps)
			a.texture.generate_mipmaps(ctx);

		if(a.is_depth_attachment()) {
			fb.attach(ctx, a.texture);
		} else {
			if(a.attach) {
				fb.attach(ctx, a.texture, 0, a.index);
			}
		}
	}

	return fb.is_complete(ctx);
}

frame_buffer_container::ivec2 frame_buffer_container::get_actual_size(context& ctx) {

	ivec2 actual_size(size);
	if(size.x() <= 0)
		actual_size.x() = ctx.get_width();
	if(size.y() <= 0)
		actual_size.y() = ctx.get_height();
	return actual_size;
}

}
}
