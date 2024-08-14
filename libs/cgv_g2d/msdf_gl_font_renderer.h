#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/attribute_array_manager.h>
#include <cgv_gl/gl/gl.h>

#include "canvas.h"
#include "msdf_font.h"
#include "msdf_text_geometry.h"
#include "shape2d_styles.h"

#include "lib_begin.h"

namespace cgv {
namespace g2d {

class CGV_API msdf_gl_font_renderer {
public:
	struct text_render_info {
		rgba color = { 0.0f, 0.0f, 0.0f, 1.0f };
		float scale = 1.0f;
		quat rotation;
		cgv::render::TextAlignment alignment = cgv::render::TextAlignment::TA_NONE;
	};

protected:
	std::string shader_prog_name;
	cgv::render::shader_program prog;

	cgv::render::attribute_array_manager attribute_arrays;

	bool build_shader_program(const cgv::render::context& ctx);

	vec2 alignment_to_offset(cgv::render::TextAlignment alignment);

	void draw_text(cgv::render::context& ctx, const msdf_font& font, const vec3& position, const msdf_text_geometry::text_info& text_info, const text_render_info& render_info);

public:
	void manage_singleton(cgv::render::context& ctx, const std::string& name, int& ref_count, int ref_count_change);

	bool init(cgv::render::context& ctx);

	void destruct(cgv::render::context& ctx);

	cgv::render::shader_program& ref_prog();

	bool enable(cgv::render::context& ctx, msdf_font& font, const text2d_style& style);

	bool enable(cgv::render::context& ctx, msdf_text_geometry& tg, const text2d_style& style);

	bool disable(cgv::render::context& ctx, msdf_font& font);

	bool disable(cgv::render::context& ctx, msdf_text_geometry& tg);

	void draw(cgv::render::context& ctx, msdf_font& font, const std::string& text, const vec3& position, const text_render_info& render_info);

	void draw(cgv::render::context& ctx, msdf_text_geometry& tg, size_t offset = 0, int count = -1);

	bool render(cgv::render::context& ctx, msdf_font& font, const std::string& text, const vec3& position, const text_render_info& render_info, const text2d_style& style);

	bool render(cgv::render::context& ctx, msdf_text_geometry& tg, const text2d_style& style, size_t offset = 0, int count = -1);
};



class CGV_API msdf_gl_font_renderer_2d : public msdf_gl_font_renderer {
public:
	msdf_gl_font_renderer_2d();

	bool enable(cgv::render::context& ctx, canvas& cvs, msdf_font& font, const text2d_style& style);

	bool enable(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, const text2d_style& style);

	void draw(cgv::render::context& ctx, canvas& cvs, msdf_font& font, const std::string& text, const vec2& position, const text_render_info& render_info);

	void draw(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, size_t offset = 0, int count = -1);

	bool render(cgv::render::context& ctx, canvas& cvs, msdf_font& font, const std::string& text, const vec2& position, const text_render_info& render_info, const text2d_style& style);

	bool render(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, const text2d_style& style, size_t offset = 0, int count = -1);
};

extern CGV_API msdf_gl_font_renderer_2d& ref_msdf_gl_font_renderer_2d(cgv::render::context& ctx, int ref_count_change = 0);



class CGV_API msdf_gl_font_renderer_3d : public msdf_gl_font_renderer {
public:
	msdf_gl_font_renderer_3d();
};

extern CGV_API msdf_gl_font_renderer_3d& ref_msdf_gl_font_renderer_3d(cgv::render::context& ctx, int ref_count_change = 0);

}
}

#include <cgv/config/lib_end.h>
