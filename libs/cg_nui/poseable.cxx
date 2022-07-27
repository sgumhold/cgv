#include "poseable.h"

#include "cgv/math/proximity.h"
#include "cgv/math/ftransform.h"

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

//cgv::nui::rotatable* cgv::nui::poseable::get_rotatable()
//{
//	if (!tried_rotatable_cast) {
//		_rotatable = dynamic_cast<rotatable*>(this);
//		tried_rotatable_cast = true;
//		if (!_rotatable && manipulate_rotation)
//			std::cout << "A 'posable' requires interface 'rotatable' to work correctly" << std::endl;
//	}
//	return _rotatable;
//}

void cgv::nui::poseable::on_grabbed_start()
{
	position_at_grab = get_translatable()->get_position();
	//rotation_at_grab = get_rotatable()->get_rotation();
}

void cgv::nui::poseable::on_grabbed_drag()
{
	//get_translatable()->set_position(get_translatable()->get_position() + ii_during_focus[activating_hid_id].query_point - ii_at_grab.query_point);
	get_translatable()->set_position(position_at_grab + (ii_during_focus[activating_hid_id].query_point_global - ii_at_grab.query_point_global));
}

void cgv::nui::poseable::on_triggered_start()
{
	position_at_grab = get_translatable()->get_position();
	std::cout << "grab: global: " << ii_at_grab.query_point_global << " local: " << ii_at_grab.query_point << std::endl;
	//rotation_at_grab = get_rotatable()->get_rotation();
}

void cgv::nui::poseable::on_triggered_drag()
{
	vec3 q = cgv::math::closest_point_on_line_to_point(ii_during_focus[activating_hid_id].hid_position_global,
		ii_during_focus[activating_hid_id].hid_direction_global, ii_at_grab.query_point_global);
	//get_translatable()->set_position(get_translatable()->get_position() + (q - ii_at_grab.query_point));
	get_translatable()->set_position(position_at_grab + (q - ii_at_grab.query_point_global));
	vec3 q2 = cgv::math::closest_point_on_line_to_point(ii_during_focus[activating_hid_id].hid_position,
		ii_during_focus[activating_hid_id].hid_direction, ii_at_grab.query_point);
	//std::cout << "drag: global: " << q - ii_at_grab.query_point_global << " local: " << q2 - ii_at_grab.query_point << std::endl;
	std::cout << "drag: global: " << ii_at_grab.query_point_global << " local: " <<  ii_at_grab.query_point << std::endl;
}

void cgv::nui::poseable::draw(cgv::render::context& ctx)
{
	interactable::draw(ctx);
}

void cgv::nui::poseable::create_gui()
{
	interactable::create_gui();
}