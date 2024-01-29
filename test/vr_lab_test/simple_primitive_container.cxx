#include "simple_primitive_container.h"
#include <cgv/math/proximity.h>
#include <cgv/math/intersection.h>
#include <random>

simple_primitive_container::rgb simple_primitive_container::get_modified_color(const rgb& color) const
{
	rgb mod_col(color);
	switch (state) {
	case state_enum::grabbed:
		mod_col[1] = std::min(1.0f, mod_col[0] + 0.2f);
	case state_enum::close:
		mod_col[0] = std::min(1.0f, mod_col[0] + 0.2f);
		break;
	case state_enum::triggered:
		mod_col[1] = std::min(1.0f, mod_col[0] + 0.2f);
	case state_enum::pointed:
		mod_col[2] = std::min(1.0f, mod_col[2] + 0.2f);
		break;
	}
	return mod_col;
}
simple_primitive_container::simple_primitive_container(const std::string& _name, unsigned sphere_count)
	: cgv::base::node(_name)
{
	std::default_random_engine e;
	std::uniform_real_distribution<float> dr(0.3f, 1.0f);
	std::uniform_real_distribution<float> dc(-0.5f, 0.5f);
	for (unsigned i = 0; i < sphere_count; ++i) {
		positions.push_back(vec3(dc(e), dr(e), dc(e)));
		colors.push_back(rgb(dr(e), dr(e), dr(e)));
		radii.push_back(0.1f*dr(e));
	}
	debug_point = vec3(0,0.5f,0);
	srs.radius = 0.01f;
}
std::string simple_primitive_container::get_type_name() const
{
	return "simple_primitive_container";
}
void simple_primitive_container::on_set(void* member_ptr)
{
	update_member(member_ptr);
	post_redraw();
}
bool simple_primitive_container::focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa, const cgv::nui::focus_demand& demand, const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info)
{
	switch (action) {
	case cgv::nui::focus_change_action::attach:
		if (state == state_enum::idle) {
			// set state based on dispatch mode
			state = dis_info.mode == cgv::nui::dispatch_mode::pointing ? state_enum::pointed : state_enum::close;
			on_set(&state);
			// store hid to filter handled events
			hid_id = dis_info.hid_id;
			return true;
		}
		// if focus is given to other hid, refuse attachment to new hid
		return false;
	case cgv::nui::focus_change_action::detach:
		// check whether detach corresponds to stored hid
		if (state != state_enum::idle && hid_id == dis_info.hid_id) {
			state = state_enum::idle;
			on_set(&state);
			return true;
		}
		return false;
	case cgv::nui::focus_change_action::index_change:
		prim_idx = int(reinterpret_cast<const cgv::nui::hit_dispatch_info&>(dis_info).get_hit_info()->primitive_index);
		on_set(&prim_idx);
		return true;
	}
	return true;
}
void simple_primitive_container::stream_help(std::ostream& os)
{
	os << "simple_primitive_container: grab and point at it" << std::endl;
}
bool simple_primitive_container::handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request)
{
	// ignore all events in idle mode
	if (state == state_enum::idle)
		return false;
	// ignore events from other hids
	if (!(dis_info.hid_id == hid_id))
		return false;
	bool pressed;
	// hid independent check if grabbing is activated or deactivated
	if (is_grab_change(e, pressed)) {
		if (pressed) {
			state = state_enum::grabbed;
			on_set(&state);
			drag_begin(request, false, original_config);
		}
		else {
			drag_end(request, original_config);
			state = state_enum::close;
			on_set(&state);
		}
		return true;
	}
	// check if event is for grabbing
	if (is_grabbing(e, dis_info)) {
		const auto& prox_info = get_proximity_info(dis_info);
		if (state == state_enum::close) {
			debug_point = prox_info.hit_point;
			query_point_at_grab = prox_info.query_point;
			prim_idx = int(prox_info.primitive_index);
			position_at_grab = positions[prim_idx];
		}
		else if (state == state_enum::grabbed) {
			debug_point = prox_info.hit_point;
			positions[prim_idx] = position_at_grab + prox_info.query_point - query_point_at_grab;
		}
		post_redraw();
		return true;
	}
	// hid independent check if object is triggered during pointing
	if (is_trigger_change(e, pressed)) {
		if (pressed) {
			state = state_enum::triggered;
			on_set(&state);
			drag_begin(request, true, original_config);
		}
		else {
			drag_end(request, original_config);
			state = state_enum::pointed;
			on_set(&state);
		}
		return true;
	}
	// check if event is for pointing
	if (is_pointing(e, dis_info)) {
		const auto& inter_info = get_intersection_info(dis_info);
		if (state == state_enum::pointed) {
			debug_point = inter_info.hit_point;
			hit_point_at_trigger = inter_info.hit_point;
			prim_idx = int(inter_info.primitive_index);
			position_at_trigger = positions[prim_idx];
		}
		else if (state == state_enum::triggered) {
			// if we still have an intersection point, use as debug point
			if (inter_info.ray_param != std::numeric_limits<float>::max())
				debug_point = inter_info.hit_point;
			// to be save even without new intersection, find closest point on ray to hit point at trigger
			vec3 q = cgv::math::closest_point_on_line_to_point(inter_info.ray_origin, inter_info.ray_direction, hit_point_at_trigger);
			positions[prim_idx] = position_at_trigger + q - hit_point_at_trigger;
		}
		post_redraw();
		return true;
	}
	return false;
}
bool simple_primitive_container::compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx)
{
	float min_dist = std::numeric_limits<float>::max();
	vec3 q, n;
	for (size_t i = 0; i < positions.size(); ++i) {
		cgv::math::closest_point_on_sphere_to_point(positions[i], radii[i], point, q, n);
		float dist = (point - q).length();
		if (dist < min_dist) {
			primitive_idx = i;
			prj_point = q;
			prj_normal = n;
			min_dist = dist;
		}
	}
	//std::cout << "min_dist = " << positions[0] << " <-> " << point << " | " << radii[0] << " at " << min_dist << " for " << primitive_idx << std::endl;
	return min_dist < std::numeric_limits<float>::max();
}
bool simple_primitive_container::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	vec3 ro = ray_start;
	vec3 rd = ray_direction;
	hit_param = std::numeric_limits<float>::max();
	for (size_t i = 0; i < positions.size(); ++i) {
		cgv::vec2 res;
		if(cgv::math::ray_sphere_intersection({ ro, rd }, positions[i], radii[i], res) == 0)
			continue;
		float param;
		if (res[0] < 0) {
			if (res[1] < 0)
				continue;
			param = res[1];
		}
		else
			param = res[0];
		if (param < hit_param) {
			primitive_idx = i;
			hit_param = param;
			hit_normal = normalize(ro+param*rd-positions[i]);
		}
	}
	return hit_param < std::numeric_limits<float>::max();
}
bool simple_primitive_container::init(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, 1);
	return true;
}
void simple_primitive_container::clear(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, -1);
}
void simple_primitive_container::draw(cgv::render::context& ctx)
{
	// show spheres
	auto& sr = cgv::render::ref_sphere_renderer(ctx);
	sr.set_render_style(srs);
	sr.set_position_array(ctx, positions);
	rgb tmp_color;
	if (prim_idx >= 0 && prim_idx < positions.size()) {
		tmp_color = colors[prim_idx];
		colors[prim_idx] = get_modified_color(colors[prim_idx]);
	}
	sr.set_color_array(ctx, colors);
	sr.set_radius_array(ctx, radii);
	sr.render(ctx, 0, positions.size());
	if (prim_idx >= 0 && prim_idx < positions.size())
		colors[prim_idx] = tmp_color;

	// show points
	sr.set_position(ctx, debug_point);
	rgb color(0.5f, 0.5f, 0.5f);
	sr.set_color_array(ctx, &color, 1);
	sr.render(ctx, 0, 1);
	if (state == state_enum::grabbed) {
		sr.set_position(ctx, query_point_at_grab);
		sr.set_color(ctx, rgb(0.5f, 0.5f, 0.5f));
		sr.render(ctx, 0, 1);
	}
	if (state == state_enum::triggered) {
		sr.set_position(ctx, hit_point_at_trigger);
		sr.set_color(ctx, rgb(0.3f, 0.3f, 0.3f));
		sr.render(ctx, 0, 1);
	}
}
void simple_primitive_container::create_gui()
{
	add_decorator(get_name(), "heading", "level=2");
	if (begin_tree_node("style", srs)) {
		align("\a");
		add_gui("srs", srs);
		align("\b");
		end_tree_node(srs);
	}

}
