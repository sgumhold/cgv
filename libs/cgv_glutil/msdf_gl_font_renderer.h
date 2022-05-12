#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>
#include "msdf_font.h"
#include "msdf_text_geometry.h"

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API msdf_gl_font_renderer : public cgv::render::render_types {
protected:
	cgv::render::shader_program prog;

	bool build_shader_program(const cgv::render::context& ctx) {
		return prog.build_program(ctx, "sdf_font2d.glpr", true);// , defines);
	}

public:
	void destruct(cgv::render::context& ctx) {
		prog.destruct(ctx);
	}

	bool init(cgv::render::context& ctx) {
		return build_shader_program(ctx);
	}

	cgv::render::shader_program& ref_prog() { return prog; }

	bool enable(cgv::render::context& ctx) {
		return prog.enable(ctx);
	}

	bool disable(cgv::render::context& ctx) {
		return prog.disable(ctx);
	}

	void draw(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg, size_t offset = 0, int count = -1);

	bool render(cgv::render::context& ctx, const ivec2& viewport_resolution, msdf_text_geometry& tg, size_t offset = 0, int count = -1);
};

}
}

#include <cgv/config/lib_end.h>
