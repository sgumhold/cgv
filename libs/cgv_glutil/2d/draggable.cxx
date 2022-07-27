#include "draggable.h"

namespace cgv {
namespace glutil {

draggable::draggable() : position_is_center(false), constraint_reference(CR_FULL_SIZE) {}

draggable::vec2 draggable::center() const {
	if(position_is_center)
		return pos;
	else
		return pos + size;
}

void draggable::apply_constraint(const rect& area) {
	vec2 min_pnt = vec2(area.box.get_min_pnt());
	vec2 max_pnt = vec2(area.box.get_max_pnt());

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
	
	pos = cgv::math::clamp(pos, min_pnt, max_pnt);
}

bool draggable::is_inside(const ivec2& p) const {
	vec2 a = pos;
	vec2 b = pos + size;
	return
		p.x() >= a.x() && p.x() <= b.x() &&
		p.y() >= a.y() && p.y() <= b.y();
}

}
}
