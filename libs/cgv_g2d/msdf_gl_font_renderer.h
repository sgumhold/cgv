#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>

#include "msdf_font.h"
#include "msdf_text_geometry.h"
#include "shape2d_styles.h"

#include "lib_begin.h"

namespace cgv {
namespace g2d {

class CGV_API msdf_gl_font_renderer {
protected:
	cgv::render::vertex_buffer geometry_buffer;

	cgv::render::shader_program prog;

	bool build_shader_program(const cgv::render::context& ctx);

	void draw_text(cgv::render::context& ctx, const msdf_font& font, const msdf_text_geometry::text_info& text);

public:
	void manage_singleton(cgv::render::context& ctx, const std::string& name, int& ref_count, int ref_count_change);

	void destruct(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);

	cgv::render::shader_program& ref_prog();

	bool enable(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_font& font, const text2d_style& style);

	bool enable(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg, const text2d_style& style);

	bool disable(cgv::render::context& ctx, msdf_font& font);

	bool disable(cgv::render::context& ctx, msdf_text_geometry& tg);

	void draw(cgv::render::context& ctx, msdf_font& font, const std::string& text, vec2 position, cgv::render::TextAlignment alignment = cgv::render::TextAlignment::TA_NONE, rgba color = rgba(0.0f, 0.0f, 0.0f, 1.0f), float scale = 1.0f);

	void draw(cgv::render::context& ctx, msdf_text_geometry& tg, size_t offset = 0, int count = -1);

	bool render(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_font& font, const std::string& text, const text2d_style& style, vec2 position, cgv::render::TextAlignment alignment = cgv::render::TextAlignment::TA_NONE, float scale = 1.0f);

	bool render(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg, const text2d_style& style, size_t offset = 0, int count = -1);
};

extern CGV_API msdf_gl_font_renderer& ref_msdf_gl_font_renderer(cgv::render::context& ctx, int ref_count_change = 0);

}
}

#include <cgv/config/lib_end.h>
