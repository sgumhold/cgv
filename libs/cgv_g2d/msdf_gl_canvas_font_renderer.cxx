#include "msdf_gl_canvas_font_renderer.h"

namespace cgv {
namespace g2d {

msdf_gl_canvas_font_renderer& ref_msdf_gl_canvas_font_renderer(cgv::render::context& ctx, int ref_count_change) {
	static int ref_count = 0;
	static msdf_gl_canvas_font_renderer r;
	r.manage_singleton(ctx, "msdf_gl_canvas_font_renderer", ref_count, ref_count_change);
	return r;
}

void msdf_gl_canvas_font_renderer::draw(cgv::render::context& ctx, msdf_text_geometry& tg, size_t offset, int count) {
	msdf_gl_font_renderer::draw(ctx, tg, offset, count);
}

void msdf_gl_canvas_font_renderer::draw(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, size_t offset, int count) {
	cvs.set_view(ctx, prog);
	draw(ctx, tg, offset, count);
}

bool msdf_gl_canvas_font_renderer::render(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, const shape2d_style& style, size_t offset, int count) {
	if(!enable(ctx, cvs, tg, style))
		return false;
	draw(ctx, tg, offset, count);
	return disable(ctx, tg);
}

}
}
