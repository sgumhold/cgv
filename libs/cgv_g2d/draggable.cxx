#include "draggable.h"

namespace cgv {
namespace g2d {

void draggable::set_constraint(const irect* area) {
	constraint = area;
}

const irect* draggable::get_constraint() const {
	return constraint;
}

void draggable::apply_constraint() {
	if(constraint)
		apply_constraint(*constraint);
}

void draggable::apply_constraint(const irect& area) {
	vec2 min_pnt = vec2(area.a());
	vec2 max_pnt = vec2(area.b());

	vec2 s = size;

	if(position_is_center)
		s *= 0.5f;

	switch(constraint_reference) {
	case CR_MIN_POINT:
		if(position_is_center) {
			min_pnt += s;
			max_pnt += s;
		}
		break;
	case CR_MAX_POINT:
		min_pnt -= s;
		max_pnt -= s;
		break;
	case CR_FULL_SIZE:
		if(position_is_center)
			min_pnt += s;
		max_pnt -= s;
		break;
	case CR_CENTER:
		if(!position_is_center) {
			min_pnt -= 0.5f*s;
			max_pnt -= 0.5f*s;
		}
	default:
		break;
	}

	position = cgv::math::clamp(position, min_pnt, max_pnt);
}

circle_draggable::circle_draggable() : draggable() {
	position_is_center = true;
}

circle_draggable::circle_draggable(const vec2& position, const vec2& size) : draggable(position, size) {
	position_is_center = true;
}

}
}
