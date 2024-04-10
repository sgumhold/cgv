#pragma once

#include <cgv/math/fvec.h>

#include "trect.h"

namespace cgv {
namespace g2d {

struct shape_base : public rect {

	bool position_is_center = false;

	using rect::rect;

	vec2 center() const { return position_is_center ? position : rect::center(); }

	virtual bool contains(const vec2& query_pos) const {
		return rect::contains(position_is_center ? query_pos + rect::center() - position : query_pos);
	}
};

}
}
