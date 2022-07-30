#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>

#include "msdf_font.h"
#include "msdf_text_geometry.h"
#include "2d/canvas.h"

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API msdf_gl_font_renderer : public cgv::render::render_types {
protected:
	cgv::render::shader_program prog;

	bool build_shader_program(const cgv::render::context& ctx) {
		return prog.build_program(ctx, "sdf_font2d.glpr", true);
	}

public:
	void destruct(cgv::render::context& ctx) {
		prog.destruct(ctx);
	}

	bool init(cgv::render::context& ctx) {
		return build_shader_program(ctx);
	}

	cgv::render::shader_program& ref_prog() {
		return prog;
	}

	cgv::render::shader_program& enable_prog(cgv::render::context& ctx) {
		prog.enable(ctx);
		return prog;
	}

	bool enable(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg) {
		bool res = prog.is_enabled() ? true : prog.enable(ctx);
		res &= tg.enable(ctx);
		if(res) {
			prog.set_uniform(ctx, "resolution", viewport_resolution);
			prog.set_uniform(ctx, "src_size", tg.get_msdf_font()->get_initial_font_size());
			prog.set_uniform(ctx, "pixel_range", tg.get_msdf_font()->get_pixel_range());
			prog.set_uniform(ctx, "true_sdf_mix_factor", 0.0f);
		}
		return res;
	}

	bool disable(cgv::render::context& ctx, msdf_text_geometry& tg) {
		bool res = prog.disable(ctx);
		tg.disable(ctx);
		return res;
	}

	void draw(cgv::render::context& ctx, msdf_text_geometry& tg, size_t offset = 0, int count = -1);

	bool render(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg, size_t offset = 0, int count = -1);
};

class CGV_API msdf_gl_canvas_font_renderer : public msdf_gl_font_renderer {
public:
	bool enable(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg) {
		bool res = msdf_gl_font_renderer::enable(ctx, cvs.get_resolution(), tg);
		if(res)
			cvs.set_view(ctx, prog);
		return res;
	}

	void draw(cgv::render::context& ctx, msdf_text_geometry& tg, size_t offset = 0, int count = -1);

	void draw(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, size_t offset = 0, int count = -1);

	bool render(cgv::render::context& ctx, canvas& cvs, msdf_text_geometry& tg, size_t offset = 0, int count = -1);
};

}
}

#include <cgv/config/lib_end.h>
