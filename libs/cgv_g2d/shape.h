#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
namespace g2d {

struct CGV_API shape_base : public frect {
	bool position_is_center = false;

	inline cgv::render::ivec2 int_position() const { return static_cast<cgv::render::ivec2>(position); }

	template<typename T>
	inline cgv::render::ivec2 int_size() const { return static_cast<cgv::render::ivec2>(size); }

	inline cgv::render::vec2 center() const { return position_is_center ? position : frect::center(); }

	virtual bool is_inside(const cgv::render::vec2& query_pos) const {

		return frect::is_inside(position_is_center ? query_pos + frect::center() - position : query_pos);
	}
};

}
}

#include <cgv/config/lib_end.h>
