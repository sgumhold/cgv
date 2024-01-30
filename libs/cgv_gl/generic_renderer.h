#pragma once

#include <cgv/render/shader_program.h>
#include <cgv/render/context.h>

#include "generic_render_data.h"

namespace cgv {
namespace render{

class generic_renderer {
protected:
	std::string prog_name = "";
	shader_program prog;
	shader_define_map defines;

	bool has_indices = false;

	bool build_shader_program(const context& ctx, const shader_define_map& defines) {
		bool success = prog.build_program(ctx, prog_name, true, defines);
		prog.allow_context_to_set_color(false);
		return success;
	}

public:
	generic_renderer() {}
	generic_renderer(const std::string& prog_name) : prog_name(prog_name) {}

	void destruct(const context& ctx) {
		prog.destruct(ctx);
	}

	bool init(context& ctx) {
		return build_shader_program(ctx, defines);
	}

	bool set_shader_defines(const context& ctx, const shader_define_map& defines) {
		if(prog.is_created())
			prog.destruct(ctx);
		return build_shader_program(ctx, defines);
	}

	shader_program& ref_prog() {
		assert(prog.is_created() && "generic_renderer::ref_prog shader program is not created; call init before first use");
		return prog;
	}

	shader_program& enable_prog(context& ctx) {
		assert(prog.is_created() && "generic_renderer::enable_prog shader program is not created; call init before first use");
		prog.enable(ctx);
		return prog;
	}

	bool enable(context& ctx, generic_render_data& geometry) {
		bool res = geometry.enable(ctx, prog);

		if(res)
			res &= prog.is_enabled() ? true : prog.enable(ctx);
		
		has_indices = geometry.has_indices();
		return res;
	}

	bool disable(context& ctx, generic_render_data& geometry) {
		bool res = geometry.disable(ctx);
		res &= prog.disable(ctx);
		has_indices = false;
		return res;
	}

	void draw(context& ctx, PrimitiveType type, generic_render_data& geometry, size_t start = 0, size_t count = 0) {
		size_t draw_count = geometry.render_count();
		start = std::min(start, draw_count);
		draw_count = std::min(start + (count ? count : draw_count), draw_count) - start;

		GLenum pt = gl::map_to_gl(type);
		if(has_indices)
			glDrawElements(pt, (GLsizei)draw_count, GL_UNSIGNED_INT, (void*)(start * sizeof(unsigned)));
		else
			glDrawArrays(pt, (GLint)start, (GLsizei)draw_count);
	}


	bool render(context& ctx, PrimitiveType type, generic_render_data& geometry, size_t start = 0, size_t count = 0) {
		if(!enable(ctx, geometry))
			return false;
		draw(ctx, type, geometry, start, count);
		return disable(ctx, geometry);
	}
};

}
}

#include <cgv/config/lib_end.h>
