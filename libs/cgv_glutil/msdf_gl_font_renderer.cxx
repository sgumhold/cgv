#include "msdf_gl_font_renderer.h"

namespace cgv {
namespace glutil {

void msdf_gl_font_renderer::draw(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg, size_t offset, int count) {
	if(offset >= tg.size() || !tg.enable(ctx))
		return;

	prog.set_uniform(ctx, "resolution", viewport_resolution);
	prog.set_uniform(ctx, "src_size", tg.get_msdf_font()->get_initial_font_size());
	prog.set_uniform(ctx, "pixel_range", tg.get_msdf_font()->get_pixel_range());
	prog.set_uniform(ctx, "true_sdf_mix_factor", 0.0f);

	size_t end = count < 0 ? tg.size() : offset + static_cast<size_t>(count);
	end = std::min(end, tg.size());

	//for(const auto& text : tg.ref_texts()) {
	for(size_t i = offset; i < end; ++i) {
		const auto& text = tg.ref_texts()[i];
		vec2 position(text.position);
		vec2 size = text.size * tg.get_font_size();

		position -= 0.5f * size;

		if(text.alignment & cgv::render::TA_LEFT)
			position.x() += 0.5f * size.x();
		else if(text.alignment & cgv::render::TA_RIGHT)
			position.x() -= 0.5f * size.x();

		if(text.alignment & cgv::render::TA_TOP)
			position.y() -= 0.5f * size.y();
		else if(text.alignment & cgv::render::TA_BOTTOM)
			position.y() += 0.5f * size.y();


		prog.set_uniform(ctx, "position", ivec2(round(position)));
		prog.set_uniform(ctx, "font_size", tg.get_font_size());

		glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP, (GLint)0, (GLsizei)4, (GLsizei)text.str.size(), (GLuint)text.offset);
	}

	tg.disable(ctx);
}

bool msdf_gl_font_renderer::render(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg, size_t offset, int count) {
	if(!enable(ctx))
		return false;
	draw(ctx, viewport_resolution, tg, offset, count);
	return disable(ctx);
}

}
}
