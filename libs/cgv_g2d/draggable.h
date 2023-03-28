#pragma once

#include "rect.h"
#include "shape.h"

#include "lib_begin.h"

namespace cgv {
namespace g2d {

struct CGV_API draggable : public shape_base {
	const irect* constraint = nullptr;

	enum ConstraintReference {
		CR_CENTER,
		CR_MIN_POINT,
		CR_MAX_POINT,
		CR_FULL_SIZE
	} constraint_reference;

	draggable();

	void set_constraint(const irect* area);

	const irect* get_constraint() const;

	void apply_constraint();

	void apply_constraint(const irect& area);
};

}
}

#include <cgv/config/lib_end.h>
