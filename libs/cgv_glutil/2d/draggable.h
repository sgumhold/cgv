#pragma once

#include <cgv/render/render_types.h>

#include "rect.h"

#include "../lib_begin.h"

namespace cgv {
namespace glutil {

struct CGV_API draggable : public cgv::render::render_types {
	vec2 pos;
	vec2 size;
	bool position_is_center;
	const rect* constraint = nullptr;

	enum ConstraintReference {
		CR_CENTER,
		CR_MIN_POINT,
		CR_MAX_POINT,
		CR_FULL_SIZE
	} constraint_reference;

	draggable();

	vec2 center() const;

	void set_constraint(const rect* constraint);

	const rect* get_constraint() const;

	void apply_constraint();

	void apply_constraint(const rect& area);

	virtual bool is_inside(const ivec2& p) const;
};

}
}

#include <cgv/config/lib_end.h>
