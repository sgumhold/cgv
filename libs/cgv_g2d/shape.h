#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
namespace g2d {

struct CGV_API shape_base : public cgv::render::render_types {
	vec2 pos;
	vec2 size;
	bool position_is_center = false;

	ivec2 ipos() const { return static_cast<ivec2>(pos); }

	ivec2 isize() const { return static_cast<ivec2>(size); }

	vec2 center() const {
		return position_is_center ? pos : pos + 0.5f * size;
	}

	virtual bool is_inside(const vec2& query_pos) const {
		vec2 a = position_is_center ? pos - 0.5f * size : pos;
		vec2 b = a + size;
		
		return
			query_pos.x() >= a.x() && query_pos.x() <= b.x() &&
			query_pos.y() >= a.y() && query_pos.y() <= b.y();
	}
};

}
}

#include <cgv/config/lib_end.h>
