#include "translation_gizmo.h"

#include <cgv/math/intersection.h>
#include <cgv/math/proximity.h>


void cgv::nui::translation_gizmo::compute_geometry()
{
	compute_absolute_axis_parameters(current_anchor_position, current_anchor_rotation, current_anchor_scale, use_local_coords);

	arrow_positions.clear();
	arrow_directions.clear();
	for (int i = 0; i < absolute_axes_directions.size(); ++i) {
		arrow_positions.push_back(absolute_axes_positions[i]);
		arrow_directions.push_back(absolute_axes_directions[i] * translation_axes_length);
	}
	for (int i = 0; i < translation_axes_colors.size(); ++i) {
		arrow_colors.push_back(translation_axes_colors[i]);
	}
	// Fill rest of axis colors with last configured color if not enough colors where configured
	for (int i = 0; i < absolute_axes_directions.size() - translation_axes_colors.size(); i++) {
		arrow_colors.push_back(translation_axes_colors[translation_axes_colors.size()]);
	}
}

void cgv::nui::translation_gizmo::on_handle_grabbed()
{
	if (position_ptr != nullptr)
		position_at_grab = *position_ptr;
}

void cgv::nui::translation_gizmo::on_handle_drag()
{
	if (position_ptr == nullptr)
		return;

	vec3 axis = absolute_axes_directions[prim_idx];

	vec3 closest_point;
	if (ii_at_grab.is_pointing) {
		if (!cgv::math::closest_point_on_line_to_line(ii_at_grab.query_point, axis,
			ii_during_focus[activating_hid_id].hid_position, ii_during_focus[activating_hid_id].hid_direction, closest_point))
			return;
	}
	else {
		closest_point = cgv::math::closest_point_on_line_to_point(ii_at_grab.query_point, axis, ii_during_focus[activating_hid_id].hid_position);
	}

	vec3 movement = closest_point - ii_at_grab.query_point;
	*position_ptr = position_at_grab + movement;
}

void cgv::nui::translation_gizmo::attach(base_ptr obj, vec3* position_ptr, quat* rotation_ptr, vec3* scale_ptr)
{
	this->position_ptr = position_ptr;
	gizmo::attach(obj, position_ptr, rotation_ptr, scale_ptr);
}

void cgv::nui::translation_gizmo::detach()
{
	if (!is_attached)
		return;
	position_ptr = nullptr;
	gizmo::detach();
	// TODO: Handle focus (release grabs and foci)
}

void cgv::nui::translation_gizmo::configure_axes_directions(std::vector<vec3> axes)
{
	gizmo_functionality_configurable_axes::configure_axes_directions(axes);

	// Default configuration
	for (int i = 0; i < axes.size(); ++i) {
		translation_axes_colors.push_back(rgb(0.2f, 0.6f, 0.84f));
	}
	configure_axes_geometry(0.015f, 0.2f);
}

void cgv::nui::translation_gizmo::configure_axes_coloring(std::vector<rgb> colors)
{
	translation_axes_colors = colors;
	fill_with_last_value_if_not_full(translation_axes_colors, absolute_axes_directions.size());
	// TODO: Update rendering, Handle switching during use
}

void cgv::nui::translation_gizmo::configure_axes_geometry(float radius, float length)
{
	ars.radius_relative_to_length = 0;
	ars.radius_lower_bound = radius;
	arrow_radius = radius;
	translation_axes_length = length;
	// TODO: Update rendering, Handle switching during use
}

bool cgv::nui::translation_gizmo::compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal,
                                                        size_t& primitive_idx)
{
	if (!is_attached)
		return false;

	size_t idx = -1;
	vec3 p, n;
	float dist_min = std::numeric_limits<float>::max();
	for (size_t i = 0; i < arrow_positions.size(); ++i) {
		vec3 p1, n1;
		cgv::math::closest_point_on_cylinder_to_point(arrow_positions[i], arrow_directions[i], arrow_radius, point, p1, n1);
		float dist = (p1 - point).length();
		if (dist < dist_min) {
			dist_min = dist;
			p = p1;
			n = n1;
			idx = i + 1;
		}
	}

	prj_point = p;
	prj_normal = n;
	primitive_idx = idx;
	return true;
}

bool cgv::nui::translation_gizmo::compute_intersection(const vec3& ray_start, const vec3& ray_direction,
	float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	if (!is_attached)
		return false;

	// TODO: Find out why the cylinder intersection doesn't work

	//size_t idx = -1;
	//float t = std::numeric_limits<float>::max();
	//vec3 n;
	//for (size_t i = 0; i < arrow_positions.size(); ++i) {
	//	vec3 n0;
	//	float t0 = cgv::math::ray_cylinder_intersection(ray_start, ray_direction, arrow_positions[i], arrow_directions[i], arrow_radius, n0);
	//	if (t0 < t) {
	//		t = t0;
	//		n = n0;
	//		idx = i;
	//	}
	//}
	//
	//if (t == std::numeric_limits<float>::max())
	//	return false;
	//hit_param = t;
	//hit_normal = n;
	//primitive_idx = idx;
	//return true;

	// Intersection test with box as temporary replacement of not working cylinder intersection
	for (int i = 0; i < arrow_positions.size(); ++i) {
	
		vec3 ro = ray_start - (arrow_positions[i] + arrow_directions[i] / 2.0);
		vec3 rd = ray_direction;
		quat rot;
		rot.set_normal(normalize(arrow_directions[i]));
		rot.inverse_rotate(ro);
		rot.inverse_rotate(rd);
		vec3 n;
		vec2 res;
		if (cgv::math::ray_box_intersection(ro, rd, vec3(arrow_directions[i].length() / 2.0, arrow_radius, arrow_radius), res, n) == 0)
			continue;
		if (res[0] < 0) {
			if (res[1] < 0)
				continue;
			hit_param = res[1];
		}
		else {
			hit_param = res[0];
		}
		rot.rotate(n);
		hit_normal = n;
		primitive_idx = i;
		return true;
	}
	return false;
}

bool cgv::nui::translation_gizmo::init(cgv::render::context& ctx)
{
	if (!gizmo::init(ctx))
		return false;
	cgv::render::ref_arrow_renderer(ctx, 1);
	return true;
}

void cgv::nui::translation_gizmo::clear(cgv::render::context& ctx)
{
	cgv::render::ref_arrow_renderer(ctx, -1);
	gizmo::clear(ctx);
}

void cgv::nui::translation_gizmo::draw(cgv::render::context& ctx)
{
	if (!is_attached)
		return;

	gizmo::draw(ctx);

	if (!arrow_positions.empty()) {
		auto& ar = cgv::render::ref_arrow_renderer(ctx);
		ar.set_render_style(ars);

		ar.set_position_array(ctx, arrow_positions);
		ar.set_direction_array(ctx, arrow_directions);
		ar.set_color_array(ctx, arrow_colors);
		ar.render(ctx, 0, (GLsizei)arrow_positions.size());
	}
}

void cgv::nui::translation_gizmo::create_gui()
{
	gizmo::create_gui();
	if (begin_tree_node("arrow style", ars)) {
		align("\a");
		add_gui("ars", ars);
		align("\b");
		end_tree_node(ars);
	}
}
