#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>

#include "msdf_font.h"
#include "msdf_text_geometry.h"
#include "2d/shape2d_styles.h"

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API msdf_gl_font_renderer;

extern CGV_API msdf_gl_font_renderer& ref_msdf_gl_font_renderer(cgv::render::context& ctx, int ref_count_change = 0);

class CGV_API msdf_gl_font_renderer : public cgv::render::render_types {
protected:
	cgv::render::shader_program prog;

	bool build_shader_program(const cgv::render::context& ctx);

public:
	void manage_singleton(cgv::render::context& ctx, const std::string& name, int& ref_count, int ref_count_change);

	void destruct(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);

	cgv::render::shader_program& ref_prog();

	//cgv::render::shader_program& enable_prog(cgv::render::context& ctx);

	bool enable(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg, const shape2d_style& style);

	bool disable(cgv::render::context& ctx, msdf_text_geometry& tg);

	void draw(cgv::render::context& ctx, msdf_text_geometry& tg, size_t offset = 0, int count = -1);

	bool render(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg, const shape2d_style& style, size_t offset = 0, int count = -1);
};

}
}

#include <cgv/config/lib_end.h>
