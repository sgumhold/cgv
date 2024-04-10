#pragma once

#include <cgv/render/context.h>
#include <cgv/render/performance_monitor.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {
		namespace gl {

class CGV_API gl_performance_monitor : public performance_monitor
{
protected:
	void draw_computed_bars(cgv::render::context& ctx, cgv::render::shader_program& prog);
	void draw_bar(cgv::render::context& ctx, cgv::render::shader_program& prog, int y, const frame_data& fdata);
	void draw_bars(cgv::render::context& ctx, cgv::render::shader_program& prog);
	void draw_lines(cgv::render::context& ctx, cgv::render::shader_program& prog);
public:
	/// construct performance monitor with standard configuration
	gl_performance_monitor();
	/// draw with OpenGL
	void draw(cgv::render::context& ctx);
};

		}
	}
}

#include <cgv/config/lib_end.h>