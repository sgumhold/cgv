#include "grabable_gizmo.h"

#include <cgv/math/proximity.h>

void cgv::nui::grabable_gizmo::on_grabbed_start(vec3 query_point)
{
	query_point_at_grab = query_point;
	position_at_grab = *position_member_ptr;
}

void cgv::nui::grabable_gizmo::on_grabbed_drag(vec3 query_point)
{
	*position_member_ptr = position_at_grab + query_point - query_point_at_grab;
}

void cgv::nui::grabable_gizmo::on_triggered_start(vec3 hit_point)
{
	hit_point_at_trigger = hit_point;
	position_at_trigger = *position_member_ptr;
}

void cgv::nui::grabable_gizmo::on_triggered_drag(vec3 ray_origin, vec3 ray_direction, vec3 hit_point)
{
	// to be save even without new intersection, find closest point on ray to hit point at trigger
	vec3 q = cgv::math::closest_point_on_line_to_point(ray_origin, ray_direction,hit_point_at_trigger);
	*position_member_ptr = position_at_trigger + q - hit_point_at_trigger;
}

void cgv::nui::grabable_gizmo::attach(compute_closest_point_callback_t compute_closest_point_callback,
	compute_intersection_callback_t compute_intersection_callback, void* position_member)
{
	gizmo::attach();
	this->compute_closest_point_callback = compute_closest_point_callback;
	this->compute_intersection_callback = compute_intersection_callback;
	position_member_ptr = static_cast<vec3*>(position_member);
}

void cgv::nui::grabable_gizmo::detach()
{
	gizmo::detach();
	compute_closest_point_callback = [](const vec3& a,vec3& b,vec3& c,size_t& d) { return false; };
	compute_intersection_callback = [](const vec3& a, const vec3& b, float& c, vec3& d, size_t& e) { return false; };
	position_member_ptr = nullptr;
}

void cgv::nui::grabable_gizmo::stream_help(std::ostream& os)
{
	os << "grabable_gizmo: grab and move it" << std::endl;
}

bool cgv::nui::grabable_gizmo::compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal,
                                                     size_t& primitive_idx)
{
	return compute_closest_point_callback(point, prj_point, prj_normal, primitive_idx);
}

bool cgv::nui::grabable_gizmo::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param,
	vec3& hit_normal, size_t& primitive_idx)
{
	return compute_intersection_callback(ray_start, ray_direction, hit_param, hit_normal, primitive_idx);
}

void cgv::nui::grabable_gizmo::draw(cgv::render::context& ctx)
{
	gizmo::draw(ctx);
	auto& sr = cgv::render::ref_sphere_renderer(ctx);
	sr.set_render_style(srs);
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

void cgv::nui::grabable_gizmo::create_gui()
{
	gizmo::create_gui();
	if (begin_tree_node(get_name(), debug_point_grab_color)) {
		align("\a");
		add_member_control(this, "debug point grab color", debug_point_grab_color);
		add_member_control(this, "debug point trigger color", debug_point_trigger_color);
		align("\b");
		end_tree_node(debug_point_grab_color);
	}
}
