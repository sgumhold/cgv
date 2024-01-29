#pragma once

#include <cgv/math/fvec.h>

#include "trect.h"

namespace cgv {
namespace g2d {

struct shape_base : public rect {

	using rect::rect;

	bool position_is_center = false;

	// TODO: remove and use position_as<T> instead
	ivec2 int_position() const { return static_cast<ivec2>(position); }

	// TODO: remove and use size_as<T> instead
	template<typename T>
	ivec2 int_size() const { return static_cast<ivec2>(size); }

	vec2 center() const { return position_is_center ? position : rect::center(); }

	virtual bool contains(const vec2& query_pos) const {
		return rect::contains(position_is_center ? query_pos + rect::center() - position : query_pos);
	}
};

}
}
