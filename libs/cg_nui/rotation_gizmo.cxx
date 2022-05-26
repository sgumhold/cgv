#include "rotation_gizmo.h"

#include <cgv/math/intersection.h>
#include <cgv/math/proximity.h>

#include "debug_visualization_helper.h"
#include "cgv/math/ftransform.h"
#include "cgv_gl/box_renderer.h"


void cgv::nui::rotation_gizmo::compute_geometry()
{
	compute_absolute_axis_parameters(current_anchor_position, current_anchor_rotation, current_anchor_scale, use_local_coords);

	ring_splines.clear();
	for (int i = 0; i < absolute_axes_directions.size(); ++i) {
		ring_splines.push_back(std::pair<std::vector<vec3>, std::vector<vec4>>());
		quat rot;
		rot.set_normal(axes_directions[i]);
		for (unsigned j = 0; j <= ring_nr_spline_segments; ++j) {
			float a = float(2 * M_PI * j * (1.0f / ring_nr_spline_segments));
			float c = cos(a);
			float s = sin(a);
			vec3 spline_position = relative_to_absolute_position(rot.apply(ring_radius * max_value(current_anchor_scale / 2.0) * vec3(0, c, s)), use_local_coords, true);
			vec4 spline_tangent = vec4(
				relative_to_absolute_direction(rot.apply(6.6f * ring_radius * max_value(current_anchor_scale / 2.0) / ring_nr_spline_segments * vec3(0, -s, c)), use_local_coords),
				0);
			ring_splines.back().first.push_back(spline_position);
			ring_splines.back().second.push_back(spline_tangent);
		}
	}
}

void cgv::nui::rotation_gizmo::on_handle_grabbed()
{
	if (rotation_ptr != nullptr)
		rotation_at_grab = *rotation_ptr;

	auto& dvh = ref_debug_visualization_helper();
	dvh.update_debug_value_vector_position(direction_at_grab_handle, current_anchor_position);
	dvh.update_debug_value_vector_position(direction_currently_handle, current_anchor_position);
	dvh.enable_debug_value_visualization(projected_point_handle);
	dvh.enable_debug_value_visualization(direction_at_grab_handle);
	dvh.enable_debug_value_visualization(direction_currently_handle);
}

void cgv::nui::rotation_gizmo::on_handle_drag()
{
	if (rotation_ptr == nullptr)
		return;
	vec3 axis = absolute_axes_directions[prim_idx];
	vec3 axis_origin = absolute_axes_positions[prim_idx];

	auto& dvh = ref_debug_visualization_helper();


	vec3 closest_point;
	if (ii_at_grab.is_pointing) {
		cgv::math::closest_point_on_line_to_circle(ii_during_focus[activating_hid_id].hid_position, ii_during_focus[activating_hid_id].hid_direction,
			axis_origin, axis, ring_radius * max_value(current_anchor_scale / 2.0), closest_point);
		//vec3 closest_point_on_line;
		//cgv::math::closest_point_on_line_to_circle(ii_during_focus.hid_position, ii_during_focus.hid_direction,
		//	axis_origin, axis, ring_radius, closest_point_on_line);
		//if (!cgv::math::closest_point_on_circle_to_point(axis_origin, axis, ring_radius,
		//	closest_point_on_line, closest_point))
		//	return;
	}
	else {
		if (!cgv::math::closest_point_on_circle_to_point(axis_origin,axis, ring_radius,
			ii_during_focus[activating_hid_id].hid_position, closest_point))
			return;
	}

	dvh.update_debug_value_position(projected_point_handle, closest_point);

	vec3 direction_at_grab = cross(cross(axis, ii_at_grab.query_point - axis_origin), axis);
	vec3 direction_currently = cross(cross(axis, closest_point - axis_origin), axis);
	//vec3 direction_at_grab = ii_at_grab.query_point - axis_origin;
	//vec3 direction_currently = closest_point - axis_origin;

	dvh.update_debug_value_vector_direction(direction_at_grab_handle, direction_at_grab);
	dvh.update_debug_value_vector_direction(direction_currently_handle, direction_currently);

	float s = dot(cross(direction_at_grab, direction_currently), axis);
	float c = dot(direction_at_grab, direction_currently);
	float da = atan2(s, c);
	quat new_rotation = quat(axis, da);
	*rotation_ptr = new_rotation * rotation_at_grab;
}

void cgv::nui::rotation_gizmo::attach(base_ptr obj, vec3* position_ptr, quat* rotation_ptr, vec3* scale_ptr)
{
	this->rotation_ptr = rotation_ptr;
	gizmo::attach(obj, position_ptr, rotation_ptr, scale_ptr);
}

void cgv::nui::rotation_gizmo::detach()
{
	if (!is_attached)
		return;
	rotation_ptr = nullptr;
	gizmo::detach();
	// TODO: Handle focus (release grabs and foci)

	auto& dvh = ref_debug_visualization_helper();
	dvh.disable_debug_value_visualization(projected_point_handle);
	dvh.disable_debug_value_visualization(direction_at_grab_handle);
	dvh.disable_debug_value_visualization(direction_currently_handle);
}

void cgv::nui::rotation_gizmo::configure_axes_directions(std::vector<vec3> axes)
{
	gizmo_functionality_configurable_axes::configure_axes_directions(axes);

	// Default configuration
	for (int i = 0; i < axes.size(); ++i) {
		rotation_axes_colors.push_back(rgb(0.2f, 0.6f, 0.84f));
	}
	configure_axes_geometry(1.2f, 0.02f);
}

void cgv::nui::rotation_gizmo::configure_axes_coloring(std::vector<rgb> colors)
{
	rotation_axes_colors = colors;
	fill_with_last_value_if_not_full(rotation_axes_colors, absolute_axes_directions.size());
	// TODO: Update rendering, Handle switching during use
}

void cgv::nui::rotation_gizmo::configure_axes_geometry(float ring_radius, float ring_tube_radius)
{
	strs.radius = ring_tube_radius;
	this->ring_spline_radius = ring_tube_radius;
	this->ring_radius = ring_radius;
	// TODO: Update rendering, Handle switching during use
}

bool cgv::nui::rotation_gizmo::compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal,
	size_t& primitive_idx)
{
	if (!is_attached)
		return false;

	size_t idx = -1;
	vec3 p, n;
	float dist_min = std::numeric_limits<float>::max();

	for (size_t i = 0; i < absolute_axes_directions.size(); ++i) {
		vec3 origin = absolute_axes_positions[i];
		vec3 d_xz = point - origin;
		d_xz.y() = 0;

		vec3 q0;
		float r0 = d_xz.length();
		if (r0 < 1e-6)
			q0 = origin + (vec3(ring_radius, 0, 0));
		else
			q0 = origin + (ring_radius / r0) * d_xz;
		vec3 d_ry = point - q0;
		float r1 = d_ry.length();
		vec3 q1;
		vec3 n1;
		if (r1 < 1e-6) {
			q1 = q0 + (ring_spline_radius / r0) * d_xz;
			n1 = (1 / r0) * d_xz;
		}
		else {
			q1 = q0 + (ring_spline_radius / r1) * d_ry;
			n1 = (1 / r1) * d_ry;
		}
		float dist = (q1 - point).length();
		if (dist < dist_min) {
			p = q1;
			n = n1;
			dist_min = dist;
			idx = i;
		}
	}

	prj_point = p;
	prj_normal = n;
	primitive_idx = idx;
	return true;
}

bool cgv::nui::rotation_gizmo::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param,
	vec3& hit_normal, size_t& primitive_idx)
{
	if (!is_attached)
		return false;

	//size_t idx = -1;
	//float t = std::numeric_limits<float>::max();
	//vec3 n;
	//for (size_t i = 0; i < absolute_axes_directions.size(); ++i) {
	//	vec3 n0;
	//	float t0 = cgv::math::ray_torus_intersection(ray_start, ray_direction,
	//		absolute_axes_positions[i],
	//		absolute_axes_directions[i],
	//		vec2(ring_radius, ring_spline_radius), n0);
	//	if (t0 < t) {
	//		t = t0;
	//		n = n0;
	//		idx = i;
	//	}
	//}
	//if (t == std::numeric_limits<float>::max())
	//	return false;
	//hit_param = t;
	//hit_normal = n;
	//primitive_idx = idx;
	//return true;

	// TODO: Find out why torus intersection is not working
	// Temporary intersection approximation by multiple cubes along the ring
	int nr_ring_boxes = 20;
	for (int i = 0; i < absolute_axes_directions.size(); ++i) {
		for (int j = 0; j < nr_ring_boxes; ++j) {
			float a = 2.0 * M_PI * j * (1.0 / nr_ring_boxes);
			vec3 relative_position = ring_radius * max_value(current_anchor_scale) * 0.5f * vec3(0.0, cos(a), sin(a));
			quat rot;
			rot.set_normal(absolute_axes_directions[i]);
			rot.rotate(relative_position);
			vec3 ro = ray_start - (absolute_axes_positions[i] + relative_position);
			vec3 rd = ray_direction;
			vec3 n;
			vec2 res;

			if (cgv::math::ray_box_intersection(ro, rd, vec3(ring_spline_radius), res, n) == 0)
				continue;
			if (res[0] < 0) {
				if (res[1] < 0)
					continue;
				hit_param = res[1];
			}
			else {
				hit_param = res[0];
			}
			hit_normal = n;
			primitive_idx = i;
			return true;
		}
	}
	return false;
}

bool cgv::nui::rotation_gizmo::init(cgv::render::context& ctx)
{
	if (!gizmo::init(ctx))
		return false;
	cgv::render::ref_spline_tube_renderer(ctx, 1);
	cgv::render::ref_box_renderer(ctx, 1);

	auto& dvh = cgv::nui::ref_debug_visualization_helper();
	projected_point_handle = dvh.register_debug_value_position(vec3(0.0));
	dvh.configure_debug_value_position(projected_point_handle, 0.01f, rgb(1.0, 0.0, 0.0));
	dvh.disable_debug_value_visualization(projected_point_handle);
	direction_at_grab_handle = dvh.register_debug_value_vector(vec3(1.0));
	dvh.configure_debug_value_vector(direction_at_grab_handle, 0.01f, rgb(0.0, 1.0, 0.0));
	dvh.disable_debug_value_visualization(direction_at_grab_handle);
	direction_currently_handle = dvh.register_debug_value_vector(vec3(1.0));
	dvh.configure_debug_value_vector(direction_currently_handle, 0.01f, rgb(0.0, 0.0, 1.0));
	dvh.disable_debug_value_visualization(direction_currently_handle);
	return true;
}

void cgv::nui::rotation_gizmo::clear(cgv::render::context& ctx)
{
	cgv::render::ref_spline_tube_renderer(ctx, -1);
	cgv::render::ref_box_renderer(ctx, -1);

	auto& dvh = cgv::nui::ref_debug_visualization_helper();
	dvh.deregister_debug_value(projected_point_handle);
	dvh.deregister_debug_value(direction_at_grab_handle);
	dvh.deregister_debug_value(direction_currently_handle);
	gizmo::clear(ctx);
}

void cgv::nui::rotation_gizmo::draw(cgv::render::context& ctx)
{
	if (!is_attached)
		return;

	gizmo::draw(ctx);

	auto& str = cgv::render::ref_spline_tube_renderer(ctx);
	str.set_render_style(strs);
	for (auto ring_spline : ring_splines) {
		if (ring_spline.first.empty())
			continue;
		str.set_position_array(ctx, ring_spline.first);
		str.set_tangent_array(ctx, ring_spline.second);
		str.render(ctx, 0, ring_spline.first.size(), true);
	}

	// Debug visualization of approximated torus intersection with cubes
	//std::vector<vec3> ps;
	//std::vector<vec3> bs;
	//std::vector<quat> rs;
	//int nr_ring_boxes = 20;
	//for (int j = 0; j < nr_ring_boxes; ++j) {
	//	float a = 2.0 * M_PI * j * (1.0 / nr_ring_boxes);
	//	vec3 relative_position = ring_radius * max_value(current_anchor_scale) * 0.5f * vec3(0.0, cos(a), sin(a));
	//	quat rot;
	//	rot.set_normal(absolute_axes_directions[0]);
	//	rot.rotate(relative_position);
	//	vec3 absolute_position = absolute_axes_positions[0] + relative_position;
	//	vec3 extent = vec3(ring_spline_radius * 2.0);
	//	ps.push_back(absolute_position);
	//	bs.push_back(extent);
	//	rs.push_back(quat());
	//}
	//auto& br = cgv::render::ref_box_renderer(ctx);
	//auto brs = cgv::render::box_render_style();
	//brs.rounding = false;
	//br.set_render_style(brs);
	//br.set_position_array(ctx, ps);
	//br.set_extent_array(ctx, bs);
	//br.set_rotation_array(ctx, rs);
	//br.render(ctx, 0, ps.size());
}

void cgv::nui::rotation_gizmo::create_gui()
{
	gizmo::create_gui();
	if (begin_tree_node("spline tube style", strs)) {
		align("\a");
		add_gui("strs", strs);
		align("\b");
		end_tree_node(strs);
	}
}
