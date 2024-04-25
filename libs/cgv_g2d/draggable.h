#pragma once

#include <cgv/math/fvec.h>

#include "trect.h"
#include "shape.h"

#include "lib_begin.h"

namespace cgv {
namespace g2d {

/// @brief Defines a draggable shape using rectangular geometry that can be used in a draggable collection.
struct CGV_API draggable : public shape_base {

	using shape_base::shape_base;

	const irect* constraint = nullptr;

	enum ConstraintReference {
		CR_CENTER,
		CR_MIN_POINT,
		CR_MAX_POINT,
		CR_FULL_SIZE
	} constraint_reference = CR_FULL_SIZE;

	void set_constraint(const irect* area);

	const irect* get_constraint() const;

	void apply_constraint();

	void apply_constraint(const irect& area);
};

/// @brief Defines a draggable shape using circular geometry that can be used in a draggable collection.
/// Per default, shape_base::position_is_center is set to true. The size is interpreted as the diameter
/// and only the x-component is used while the y-component is ignored.
struct CGV_API circle_draggable : public draggable {

	using draggable::draggable;

	circle_draggable();

	circle_draggable(const vec2& position, const vec2& size);

	bool contains(const vec2& mp) const override {
		float dist = length(mp - center());
		return dist <= 0.5f * size.x();
	}
};

}
}

#include <cgv/config/lib_end.h>
