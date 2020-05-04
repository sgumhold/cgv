#pragma once

#include <array>

#include <cgv/math/fvec.h>
#include "rectangle.h"

using namespace cgv::data;
using namespace cgv::math;

namespace trajectory {
namespace util {
	namespace debug {
		struct point_t : public cgv::render::render_types {
			vec3 p;
			vec4 col;
		};
		struct line_t : public cgv::render::render_types {
			vec3 a;
			vec3 b;
			vec4 col;
		};
		struct rect_t : public cgv::render::render_types {
			rectangle rect;
			vec4 col;
		};

		void draw_point(const point_t &point, const float size);
		void draw_line(const line_t &line);
		void draw_quad(const rect_t &rect);
	} // namespace debug

} // namespace util
} // namespace trajectory