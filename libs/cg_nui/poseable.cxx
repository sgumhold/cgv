#include "poseable.h"

#include "cgv/math/proximity.h"
#include "cgv/math/ftransform.h"

cgv::nui::transforming* cgv::nui::poseable::get_transforming()
{
	if (!tried_transforming_cast) {
		_transforming = dynamic_cast<transforming*>(this);
		tried_transforming_cast = true;
	}
	return _transforming;
}

cgv::nui::translatable* cgv::nui::poseable::get_translatable()
{
	if (!tried_translatable_cast) {
		_translatable = dynamic_cast<translatable*>(this);
		tried_translatable_cast = true;
		if (!_translatable)
			std::cout << "A 'posable' requires interface 'translatable' to work correctly" << std::endl;
	}
	return _translatable;
}

cgv::nui::rotatable* cgv::nui::poseable::get_rotatable()
{
	if (!tried_rotatable_cast) {
		_rotatable = dynamic_cast<rotatable*>(this);
		tried_rotatable_cast = true;
		if (!_rotatable && manipulate_rotation)
			std::cout << "A 'posable' requires interface 'rotatable' to work correctly" << std::endl;
	}
	return _rotatable;
}

void cgv::nui::poseable::on_grabbed_start()
{
	position_at_grab = get_translatable()->get_position();
	rotation_at_grab = get_rotatable()->get_rotation();
}

void cgv::nui::poseable::on_grabbed_drag()
{
	vec3 movement = ii_during_focus[activating_hid_id].query_point - ii_at_grab.query_point;
	if (get_transforming()) {
		// movement is in the local coordinate system of the transforming. Its position however is in it's parents coordinate system.
		// Therefor movement needs to be transformed into the parents coordinate system.
		_transforming->get_local_rotation().inverse_rotate(movement);
		get_translatable()->set_position(get_translatable()->get_position() + movement);
	}
	else {
		get_translatable()->set_position(position_at_grab + movement);
	}
	
}

void cgv::nui::poseable::on_triggered_start()
{
	position_at_grab = get_translatable()->get_position();
	//rotation_at_grab = get_rotatable()->get_rotation();
}

void cgv::nui::poseable::on_triggered_drag()
{
	vec3 q = cgv::math::closest_point_on_line_to_point(ii_during_focus[activating_hid_id].hid_position,
		ii_during_focus[activating_hid_id].hid_direction, ii_at_grab.query_point);
	vec3 movement = q - ii_at_grab.query_point;
	if (get_transforming()) {
		// movement is in the local coordinate system of the transforming. Its position however is in it's parents coordinate system.
		// Therefor movement needs to be transformed into the parents coordinate system.
		_transforming->get_local_rotation().rotate(movement);
		get_translatable()->set_position(get_translatable()->get_position() + movement);
	}
	else {
		get_translatable()->set_position(position_at_grab + movement);
	}
}

bool cgv::nui::poseable::compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal,
	size_t& primitive_idx)
{
	return _compute_closest_point(point, prj_point, prj_normal, primitive_idx);
}

bool cgv::nui::poseable::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param,
	vec3& hit_normal, size_t& primitive_idx)
{
	// Make sure there is still an intersection if the object is grabbed but the ray temporarily doesn't actually intersect it.
	bool result = _compute_intersection(ray_start, ray_direction, hit_param, hit_normal, primitive_idx);
	if (!result && state == state_enum::grabbed || state == state_enum::triggered) {
		vec3 v = ii_during_focus[activating_hid_id].query_point - ray_start;
		vec3 n = ray_direction;
		hit_param = (math::dot(v, n) / math::dot(n, n) * n).length();
		primitive_idx = prim_idx;
		return true;
	}
	return result;
}