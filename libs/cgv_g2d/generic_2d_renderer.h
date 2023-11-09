#pragma once

#include <cgv_gl/generic_renderer.h>
#include <cgv_g2d/canvas.h>
#include <cgv_g2d/shape2d_styles.h>

#include "lib_begin.h"

namespace cgv {
namespace g2d {

class generic_2d_renderer : public cgv::render::generic_renderer {
public:
	generic_2d_renderer() : cgv::render::generic_renderer() {}
	generic_2d_renderer(const std::string& prog_name) : cgv::render::generic_renderer(prog_name) {}

	void set_style(cgv::render::context& ctx, const shape2d_style& style) {
		bool is_enabled = prog.is_enabled();
		
		bool success = true;
		if(!is_enabled)
			success = prog.enable(ctx);

		if(success)
			style.apply(ctx, prog);

		if(!is_enabled)
			prog.disable(ctx);
	}

	bool enable(cgv::render::context& ctx, canvas& cvs, cgv::render::generic_render_data& geometry) {
		bool res = geometry.enable(ctx, prog);	

		if(res)
			res &= prog.is_enabled() ? true : prog.enable(ctx);
		if(res)
			cvs.set_view(ctx, prog);

		has_indices = geometry.has_indices();
		return res;
	}

	bool enable(cgv::render::context& ctx, canvas& cvs, cgv::render::generic_render_data& geometry, const shape2d_style& style) {
		if(enable(ctx, cvs, geometry)) {
			style.apply(ctx, prog);
			return true;
		}
		return false;
	}

	bool render(cgv::render::context& ctx, canvas& cvs, cgv::render::PrimitiveType type, cgv::render::generic_render_data& geometry, size_t start = 0, size_t count = 0) {
		if(!enable(ctx, cvs, geometry))
			return false;
		draw(ctx, type, geometry, start, count);
		return disable(ctx, geometry);
	}

	bool render(cgv::render::context& ctx, canvas& cvs, cgv::render::PrimitiveType type, cgv::render::generic_render_data& geometry, const shape2d_style& style, size_t start = 0, size_t count = 0) {
		if(!enable(ctx, cvs, geometry, style))
			return false;
		draw(ctx, type, geometry, start, count);
		return disable(ctx, geometry);
	}
};

}
}

#include <cgv/config/lib_end.h>
