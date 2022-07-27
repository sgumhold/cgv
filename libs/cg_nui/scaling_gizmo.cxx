#include "scaling_gizmo.h"

#include <cgv/math/intersection.h>
#include <cgv/math/proximity.h>


void cgv::nui::scaling_gizmo::precompute_geometry()
{
	cube_positions.clear();
	cube_rotations.clear();
	splines.clear();
	handle_positions.clear();
	handle_directions.clear();
	for (int i = 0; i < axes_directions.size(); ++i) {
		splines.push_back(spline_data_t());
		cube_positions.push_back(vec3(0.0f));
		handle_positions.push_back(vec3(0.0f));
		handle_directions.push_back(vec3(1.0f, 0.0f, 0.0f));
		quat rot;
		vec3 axis = axes_directions[i];
		if (axis == vec3(-1.0, 0.0, 0.0))
			axis = axis * -1.0f;
		rot.set_normal(axis);
		cube_rotations.push_back(rot);
	}
	for (int i = 0; i < scaling_axes_colors.size(); ++i) {
		handle_colors.push_back(scaling_axes_colors[i]);
	}
	// Fill rest of axis colors with last configured color if not enough colors where configured
	for (int i = 0; i < axes_directions.size() - scaling_axes_colors.size(); i++) {
		handle_colors.push_back(scaling_axes_colors[scaling_axes_colors.size()]);
	}
}

void cgv::nui::scaling_gizmo::compute_geometry(const vec3& scale)
{
	for (int i = 0; i < axes_directions.size(); ++i) {
		vec3 start_point = scale_dependent_axes_positions[i] * scale + scale_independent_axes_positions[i];
		vec3 end_point = start_point + axes_directions[i] * scaling_axes_length;
		vec3 axis = axes_directions[i];
		splines.push_back(spline_data_t());
		splines[i].first.push_back(start_point);
		splines[i].second.push_back(vec4(axis, 0.0));
		splines[i].first.push_back(end_point);
		splines[i].second.push_back(vec4(axis, 0.0));
		cube_positions.push_back(end_point);
		handle_positions[i] = start_point;
		handle_directions[i] = end_point - start_point;
	}
}

bool cgv::nui::scaling_gizmo::validate_configuration()
{
	// TODO: Add configuration validation
	return true;
}

void cgv::nui::scaling_gizmo::on_handle_grabbed()
{
	if (scale_ptr != nullptr)
		scale_at_grab = *scale_ptr;
	//vec3 current_position = accumulate_transforming_hierarchy().col(3);
	//distance_at_grab = (ii_at_grab.query_point - current_position).length();
}

void cgv::nui::scaling_gizmo::on_handle_drag()
{
	if (scale_ptr == nullptr)
		return;

	vec3 axis = axes_directions[prim_idx];

	vec3 closest_point;
	if (ii_at_grab.is_pointing) {
		if (!cgv::math::closest_point_on_line_to_line(ii_at_grab.query_point, axis,
			ii_during_focus[activating_hid_id].hid_position, ii_during_focus[activating_hid_id].hid_direction, closest_point))
			return;
	}
	else {
		closest_point = cgv::math::closest_point_on_line_to_point(ii_at_grab.query_point, axis, ii_during_focus[activating_hid_id].hid_position);
	}
	//vec3 current_position = accumulate_transforming_hierarchy().col(3);
	//float distance_at_grab = (ii_at_grab.query_point - current_position).length();
	//float current_distance = (closest_point - current_position).length();
	//
	//vec3 scale_ratio = scaling_axes_scale_ratios[prim_idx];
	//*scale_ptr = scale_at_grab * (vec3(1.0) - scale_ratio) +  (scale_at_grab * current_distance / distance_at_grab) * scale_ratio;
}

void cgv::nui::scaling_gizmo::attach(base_ptr obj, vec3* scale_ptr, vec3* position_ptr, quat* rotation_ptr)
{
	this->scale_ptr = scale_ptr;
	//gizmo::attach(obj, position_ptr, rotation_ptr, scale_ptr);
	//gizmo::attach(obj);
}

void cgv::nui::scaling_gizmo::detach()
{
	if (!is_attached)
		return;
	scale_ptr = nullptr;
	gizmo::detach();
	// TODO: Handle focus (release grabs and foci)
}

void cgv::nui::scaling_gizmo::configure_axes_directions(std::vector<vec3> axes_directions)
{
	gizmo_functionality_configurable_axes::configure_axes_directions(axes_directions);
	// Default configuration
	for (int i = 0; i < this->axes_directions.size(); ++i) {
		scaling_axes_colors.push_back(rgb(0.2f, 0.6f, 0.84f));
		scaling_axes_scale_ratios.push_back(abs(axes_directions[i]));
	}
	configure_axes_geometry(0.015f, 0.2f, 0.035f);
}

void cgv::nui::scaling_gizmo::configure_axes_coloring(std::vector<rgb> colors)
{
	scaling_axes_colors = colors;
	fill_with_last_value_if_not_full(scaling_axes_colors, axes_directions.size());
	// TODO: Update rendering, Handle switching during use
}

void cgv::nui::scaling_gizmo::configure_axes_geometry(float radius, float length, float cube_size)
{
	this->spline_tube_radius = radius;
	strs.radius = radius;
	this->scaling_axes_length = length;
	this->cube_size = cube_size;
	// TODO: Update rendering, Handle switching during use
}

void cgv::nui::scaling_gizmo::configure_axes_scale_ratios(std::vector<vec3> scale_ratios)
{
	scaling_axes_scale_ratios.clear();
	for (auto scale_ratio : scale_ratios) {
		scaling_axes_scale_ratios.push_back(abs(scale_ratio));
	}
}

bool cgv::nui::scaling_gizmo::_compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal,
	size_t& primitive_idx)
{
	size_t idx = -1;
	vec3 p, n;
	float dist_min = std::numeric_limits<float>::max();
	for (size_t i = 0; i < axes_directions.size(); ++i) {
		vec3 p1, n1;
		cgv::math::closest_point_on_cylinder_to_point(
			handle_positions[i], handle_directions[i],
			cube_size / 2.0f, point, p1, n1);
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

//bool cgv::nui::scaling_gizmo::_compute_closest_point_local_orientation(const vec3& point, vec3& prj_point,
//	vec3& prj_normal, size_t& primitive_idx, const vec3& inverse_translation, const quat& inverse_rotation,
//	const vec3& scale, const mat4& view_matrix)
//{
//	if (use_global_orientation)
//		return false;
//
//	compute_geometry(scale);
//	return _compute_closest_point(point, prj_point, prj_normal, primitive_idx);
//}
//
//bool cgv::nui::scaling_gizmo::_compute_closest_point_global_orientation(const vec3& point, vec3& prj_point,
//	vec3& prj_normal, size_t& primitive_idx, const vec3& inverse_translation, const quat& rotation, const vec3& scale,
//	const mat4& view_matrix)
//{
//	if (!use_global_orientation)
//		return false;
//
//	compute_geometry(scale);
//	return _compute_closest_point(point, prj_point, prj_normal, primitive_idx);
//}

bool cgv::nui::scaling_gizmo::_compute_intersection(const vec3& ray_start, const vec3& ray_direction,
	float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	// TODO: Find out why the cylinder intersection doesn't work

	//size_t idx = -1;
	//float t = std::numeric_limits<float>::max();
	//vec3 n;
	//for (size_t i = 0; i < absolute_axes_directions.size(); ++i) {
	//	vec3 n0;
	//	float t0 = cgv::math::ray_cylinder_intersection(
	//		ray_start, ray_direction,
	//		absolute_axes_positions[i],
	//		absolute_axes_directions[i],
	//		spline_tube_radius, n0);
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
	for (int i = 0; i < axes_directions.size(); ++i) {

		vec3 ro = ray_start - (handle_positions[i] + handle_directions[i] / 2.0);
		vec3 rd = ray_direction;
		quat rot;
		if (axes_directions[i] == vec3(-1.0, 0.0, 0.0))
			rot.set_normal(axes_directions[i] * -1.0f);
		else
			rot.set_normal(axes_directions[i]);
		rot.inverse_rotate(ro);
		rot.inverse_rotate(rd);
		vec3 n;
		vec2 res;
		if (cgv::math::ray_box_intersection(ro, rd, vec3(scaling_axes_length / 2.0, cube_size / 2.0, cube_size / 2.0), res, n) == 0)
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

//bool cgv::nui::scaling_gizmo::_compute_intersection_local_orientation(const vec3& ray_start,
//	const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx,
//	const vec3& inverse_translation, const quat& inverse_rotation, const vec3& scale, const mat4& view_matrix)
//{
//	if (use_global_orientation)
//		return false;
//
//	compute_geometry(scale);
//	return _compute_intersection(ray_start, ray_direction, hit_param, hit_normal, primitive_idx);
//}
//
//bool cgv::nui::scaling_gizmo::_compute_intersection_global_orientation(const vec3& ray_start,
//	const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx,
//	const vec3& inverse_translation, const quat& rotation, const vec3& scale, const mat4& view_matrix)
//{
//	if (!use_global_orientation)
//		return false;
//
//	compute_geometry(scale);
//	return _compute_intersection(ray_start, ray_direction, hit_param, hit_normal, primitive_idx);
//}

bool cgv::nui::scaling_gizmo::init(cgv::render::context& ctx)
{
	if (!gizmo::init(ctx))
		return false;
	cgv::render::ref_spline_tube_renderer(ctx, 1);
	auto& br = cgv::render::ref_box_renderer(ctx, 1);
	return true;
}

void cgv::nui::scaling_gizmo::clear(cgv::render::context& ctx)
{
	cgv::render::ref_box_renderer(ctx, -1);
	cgv::render::ref_spline_tube_renderer(ctx, -1);
	gizmo::clear(ctx);
}

void cgv::nui::scaling_gizmo::_draw(cgv::render::context& ctx)
{
	auto& str = cgv::render::ref_spline_tube_renderer(ctx);
	str.set_render_style(strs);
	for (auto spline : splines) {
		if (spline.first.empty())
			continue;
		str.set_position_array(ctx, spline.first);
		str.set_tangent_array(ctx, spline.second);
		str.set_color_array(ctx, &handle_colors[0], handle_colors.size());
		str.render(ctx, 0, spline.first.size(), true);
	}

	auto& br = cgv::render::ref_box_renderer(ctx);
	br.set_render_style(brs);
	br.set_position_array(ctx, cube_positions);
	br.set_color_array(ctx, &handle_colors[0], handle_colors.size());
	br.set_extent(ctx, vec3(cube_size));
	br.set_rotation_array(ctx, &cube_rotations[0], cube_rotations.size());
	br.render(ctx, 0, cube_positions.size());
}

//void cgv::nui::scaling_gizmo::_draw_local_orientation(cgv::render::context& ctx, const vec3& inverse_translation,
//	const quat& inverse_rotation, const vec3& scale, const mat4& view_matrix)
//{
//	if (use_global_orientation)
//		return;
//
//	compute_geometry(scale);
//	_draw(ctx);
//}
//
//void cgv::nui::scaling_gizmo::_draw_global_orientation(cgv::render::context& ctx, const vec3& inverse_translation,
//	const quat& rotation, const vec3& scale, const mat4& view_matrix)
//{
//	if (!use_global_orientation)
//		return;
//
//	compute_geometry(scale);
//	_draw(ctx);
//}

void cgv::nui::scaling_gizmo::create_gui()
{
	gizmo::create_gui();
	if (begin_tree_node("box style", brs)) {
		align("\a");
		add_gui("brs", brs);
		align("\b");
		end_tree_node(brs);
	}
	if (begin_tree_node("spline tube style", strs)) {
		align("\a");
		add_gui("strs", strs);
		align("\b");
		end_tree_node(strs);
	}
}
