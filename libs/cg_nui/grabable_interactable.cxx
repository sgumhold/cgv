#include "grabable_interactable.h"

#include "cgv/math/proximity.h"

void cgv::nui::grabable_interactable::on_grabbed_start(vec3 query_point)
{
	query_point_at_grab = query_point;
	if (position_ptr_ptr != nullptr && *position_ptr_ptr != nullptr)
		position_at_grab = **position_ptr_ptr;
	else if (position_ptr != nullptr)
		position_at_grab = *position_ptr;
}

void cgv::nui::grabable_interactable::on_grabbed_drag(vec3 query_point)
{
	if (position_ptr_ptr != nullptr && *position_ptr_ptr != nullptr)
		**position_ptr_ptr = position_at_grab + query_point - query_point_at_grab;
	else if (position_ptr != nullptr)
		*position_ptr = position_at_grab + query_point - query_point_at_grab;
}

void cgv::nui::grabable_interactable::on_triggered_start(vec3 hit_point)
{
	hit_point_at_trigger = hit_point;
	if (position_ptr_ptr != nullptr && *position_ptr_ptr != nullptr)
		position_at_trigger = **position_ptr_ptr;
	else if (position_ptr != nullptr)
		position_at_trigger = *position_ptr;

}

void cgv::nui::grabable_interactable::on_triggered_drag(vec3 ray_origin, vec3 ray_direction, vec3 hit_point)
{
	// to be save even without new intersection, find closest point on ray to hit point at trigger
	vec3 q = cgv::math::closest_point_on_line_to_point(ray_origin, ray_direction, hit_point_at_trigger);
	if (position_ptr_ptr != nullptr && *position_ptr_ptr != nullptr)
		**position_ptr_ptr = position_at_trigger + q - hit_point_at_trigger;
	else if (position_ptr != nullptr)
		*position_ptr = position_at_trigger + q - hit_point_at_trigger;
}

void cgv::nui::grabable_interactable::draw(cgv::render::context& ctx)
{
	interactable::draw(ctx);
	return;
	auto& sr = cgv::render::ref_sphere_renderer(ctx);
	sr.set_render_style(debug_sphere_rs);
	if (state == state_enum::grabbed) {
		sr.set_position(ctx, query_point_at_grab);
		sr.set_color(ctx, debug_point_grab_color);
		sr.render(ctx, 0, 1);
	}
	if (state == state_enum::triggered) {
		sr.set_position(ctx, hit_point_at_trigger);
		sr.set_color(ctx, debug_point_trigger_color);
		sr.render(ctx, 0, 1);
	}
}

void cgv::nui::grabable_interactable::create_gui()
{
	if (begin_tree_node("grabable_interactable", debug_point_grab_color)) {
		align("\a");
		interactable::create_gui();
		add_member_control(this, "debug point grab color", debug_point_grab_color);
		add_member_control(this, "debug point trigger color", debug_point_trigger_color);
		align("\b");
		end_tree_node(debug_point_grab_color);
	}
}