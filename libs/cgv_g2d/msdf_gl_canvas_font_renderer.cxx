#include "msdf_gl_canvas_font_renderer.h"

namespace cgv {
namespace g2d {

bool msdf_gl_canvas_font_renderer::enable(cgv::render::context& ctx, canvas& cvs, msdf_font& font, const text2d_style& style) {
	bool res = msdf_gl_font_renderer::enable(ctx, cvs.get_resolution(), font, style);
	if(res)
		cvs.set_view(ctx, prog);
	return res;
}

bool msdf_gl_canvas_font_renderer::enable(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, const text2d_style& style) {
	bool res = msdf_gl_font_renderer::enable(ctx, cvs.get_resolution(), tg, style);
	if(res)
		cvs.set_view(ctx, prog);
	return res;
}

void msdf_gl_canvas_font_renderer::draw(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, size_t offset, int count) {
	cvs.set_view(ctx, prog);
	msdf_gl_font_renderer::draw(ctx, tg, offset, count);
}

bool msdf_gl_canvas_font_renderer::render(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, const text2d_style& style, size_t offset, int count) {
	if(!enable(ctx, cvs, tg, style))
		return false;
	msdf_gl_font_renderer::draw(ctx, tg, offset, count);
	return disable(ctx, tg);
}

bool msdf_gl_canvas_font_renderer::render(cgv::render::context& ctx, canvas& cvs, msdf_font& font, const std::string& text, const text2d_style& style, vec2 position, cgv::render::TextAlignment alignment, float scale) {
	if(!enable(ctx, cvs, font, style))
		return false;
	msdf_gl_font_renderer::draw(ctx, font, text, position, alignment, rgba(0.0f, 0.0f, 0.0f, 1.0f), scale);
	return disable(ctx, font);
}

msdf_gl_canvas_font_renderer& ref_msdf_gl_canvas_font_renderer(cgv::render::context& ctx, int ref_count_change) {
	static int ref_count = 0;
	static msdf_gl_canvas_font_renderer r;
	r.manage_singleton(ctx, "msdf_gl_canvas_font_renderer", ref_count, ref_count_change);
	return r;
}

}
}
