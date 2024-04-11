#pragma once

#include "msdf_gl_font_renderer.h"
#include "canvas.h"

#include "lib_begin.h"

namespace cgv {
namespace g2d {

class CGV_API msdf_gl_canvas_font_renderer : public msdf_gl_font_renderer {
public:
	bool enable(cgv::render::context& ctx, canvas& cvs, msdf_font& font, const text2d_style& style);

	bool enable(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, const text2d_style& style);

	using msdf_gl_font_renderer::draw;

	void draw(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, size_t offset = 0, int count = -1);
	
	bool render(cgv::render::context& ctx, canvas& cvs, msdf_font& font, const std::string& text, const text2d_style& style, vec2 position, cgv::render::TextAlignment alignment = cgv::render::TextAlignment::TA_NONE, float scale = 1.0f);

	bool render(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, const text2d_style& style, size_t offset = 0, int count = -1);
};

extern CGV_API msdf_gl_canvas_font_renderer& ref_msdf_gl_canvas_font_renderer(cgv::render::context& ctx, int ref_count_change = 0);

}
}

#include <cgv/config/lib_end.h>
