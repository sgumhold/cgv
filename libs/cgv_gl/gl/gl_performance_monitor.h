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
	void draw_bar(int y, const frame_data& fdata);
	void prepare_draw_lines();
	void draw_lines();
	void finish_draw_lines();
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