#include "frame_buffer_container.h"

namespace cgv {
namespace glutil {

frame_buffer_container::frame_buffer_container() {

	size = uvec2(0);
}

frame_buffer_container::~frame_buffer_container() {

	attachments.clear();
}

void frame_buffer_container::clear(context& ctx) {

	fb.destruct(ctx);

	for(auto it = attachments.begin(); it != attachments.end(); ++it) {
		attachment& a = (*it).second;
		a.texture.destruct(ctx);
	}
}

void frame_buffer_container::add_attachment(std::string name, std::string format, TextureFilter tf, bool attach) {

	attachment a;
	a.attach = attach;
	a.format = format;
	a.tf = tf;
	attachments.insert(std::make_pair(name, a));
}

bool frame_buffer_container::enable_attachment(context& ctx, std::string name, int tex_unit) {

	auto elem = attachments.find(name);
	if(elem != attachments.end()) {
		attachment& a = (*elem).second;
		return a.texture.enable(ctx, tex_unit);
	}

	return false;
}

bool frame_buffer_container::disable_attachment(context& ctx, std::string name) {

	auto elem = attachments.find(name);
	if(elem != attachments.end()) {
		attachment& a = (*elem).second;
		return a.texture.disable(ctx);
	}

	return false;
}

bool frame_buffer_container::ensure(context& ctx) {

	unsigned w = size[0];
	unsigned h = size[1];

	if(size == uvec2(0)) {
		w = ctx.get_width();
		h = ctx.get_height();
	}

	if(!fb.is_created() || fb.get_width() != w || fb.get_height() != h) {
		clear(ctx);
		if(!create_and_validate(ctx)) {
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

bool frame_buffer_container::create_and_validate(context& ctx) {

	unsigned w = size[0];
	unsigned h = size[1];

	if(size == uvec2(0)) {
		w = ctx.get_width();
		h = ctx.get_height();
	}

	fb.create(ctx, w, h);

	unsigned i = 0;
	for(auto it = attachments.begin(); it != attachments.end(); ++it) {
		attachment& a = (*it).second;

		unsigned filter_specifier = (unsigned)a.tf;

		// Even filter specifiers are nearest and odd are linear
		TextureFilter mag_filter = (filter_specifier & 1) ? TF_LINEAR : TF_NEAREST;
		// Specifiers larger than 1 are using mipmaps
		bool use_mipmaps = filter_specifier > 1;

		a.texture = texture(a.format, mag_filter, a.tf);
		a.texture.create(ctx, TT_2D, w, h);

		if(use_mipmaps)
			a.texture.generate_mipmaps(ctx);

		if(a.format == "[D]" ||
			a.format == "uint16[D]" ||
			a.format == "uint32[D:24]" ||
			a.format == "uint32[D]") {
			fb.attach(ctx, a.texture);
		} else {
			if(a.attach) {
				fb.attach(ctx, a.texture, 0, i);
				++i;
			}
		}
	}

	return fb.is_complete(ctx);
}

}
}
