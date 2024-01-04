#pragma once

#include <cgv/render/render_types.h>

#include "trect.h"

namespace cgv {
namespace g2d {

struct shape_base : public rect {

	using rect::rect;

	bool position_is_center = false;

	cgv::render::ivec2 int_position() const { return static_cast<cgv::render::ivec2>(position); }

	template<typename T>
	cgv::render::ivec2 int_size() const { return static_cast<cgv::render::ivec2>(size); }

	cgv::render::vec2 center() const { return position_is_center ? position : rect::center(); }

	virtual bool contains(const cgv::render::vec2& query_pos) const {
		return rect::contains(position_is_center ? query_pos + rect::center() - position : query_pos);
	}
};

}
}
