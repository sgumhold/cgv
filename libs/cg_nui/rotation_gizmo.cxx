#include "rotation_gizmo.h"

#include <cgv/math/intersection.h>
#include <cgv/math/proximity.h>

#include "debug_visualization_helper.h"
#include "cgv/math/ftransform.h"
#include "cgv_gl/box_renderer.h"


void cgv::nui::rotation_gizmo::precompute_geometry()
{
	ring_splines.clear();
	precomputed_ring_splines.clear();
	for (int i = 0; i < axes_directions.size(); ++i) {
		ring_splines.push_back(spline_data_t());
		precomputed_ring_splines.push_back(spline_data_t());
		quat rot;
		rot.set_normal(axes_directions[i]);
		for (unsigned j = 0; j <= ring_nr_spline_segments; ++j) {
			float a = float(2 * M_PI * j * (1.0f / ring_nr_spline_segments));
			float c = cos(a);
			float s = sin(a);
			vec3 spline_position = rot.apply(ring_radius * vec3(0, c, s));
			vec4 spline_tangent = vec4(rot.apply(6.6f * ring_radius / ring_nr_spline_segments * vec3(0, -s, c)),0);
			precomputed_ring_splines.back().first.push_back(spline_position);
			precomputed_ring_splines.back().second.push_back(spline_tangent);
			ring_splines.back().first.push_back(spline_position);
			ring_splines.back().second.push_back(spline_tangent);
		}
	}
}

void cgv::nui::rotation_gizmo::compute_geometry(const vec3& scale)
{
	// Scale precomputed rings and move them to the correct axis positions
	for (int i = 0; i < axes_directions.size(); ++i) {
		for (unsigned j = 0; j <= ring_nr_spline_segments; ++j) {
			ring_splines[i].first[j] = precomputed_ring_splines[i].first[j] * max_value(scale)
				+ (scale_dependent_axes_positions[i] * scale + scale_independent_axes_positions[i]);
			ring_splines[i].second[j] = precomputed_ring_splines[i].second[j] * max_value(scale);
		}
	}
}

bool cgv::nui::rotation_gizmo::validate_configuration()
{
	bool configuration_valid = true;

	if (!(
		rotation_ptr ||
		(rotation_ptr_ptr && *rotation_ptr_ptr) ||
		rotatable_obj
		)) {
		std::cout << "Rotation gizmo requires a valid pointer to a rotation or a pointer to a pointer to a rotation or a reference to an object implementing rotatable" << std::endl;
		configuration_valid = false;
	}

	configuration_valid = configuration_valid && validate_axes();
	configuration_valid = configuration_valid && validate_handles(axes_directions.size());

	return configuration_valid && gizmo::validate_configuration();
}

void cgv::nui::rotation_gizmo::set_rotation_reference(quat* _rotation_ptr, cgv::base::base_ptr _on_set_obj)
{
	rotation_ptr = _rotation_ptr;
	gizmo::set_on_set_object(_on_set_obj);
}

void cgv::nui::rotation_gizmo::set_rotation_reference(quat** _rotation_ptr_ptr, cgv::base::base_ptr _on_set_obj)
{
	rotation_ptr_ptr = _rotation_ptr_ptr;
	gizmo::set_on_set_object(_on_set_obj);
}

void cgv::nui::rotation_gizmo::set_rotation_reference(rotatable* _rotatable_obj)
{
	rotatable_obj = _rotatable_obj;
}

cgv::render::render_types::quat cgv::nui::rotation_gizmo::get_rotation()
{
	if (rotatable_obj)
		return rotatable_obj->get_rotation();
	if (rotation_ptr_ptr)
		return **rotation_ptr_ptr;
	return *rotation_ptr;
}

void cgv::nui::rotation_gizmo::set_rotation(const quat& rotation)
{
	if (rotatable_obj) {
		rotatable_obj->set_rotation(rotation);
	}
	else {
		if (rotation_ptr_ptr)
			**rotation_ptr_ptr = rotation;
		else
			*rotation_ptr = rotation;
		if (on_set_obj)
			on_set_obj->on_set(rotation_ptr);
	}
}

void cgv::nui::rotation_gizmo::on_handle_grabbed()
{
	rotation_at_grab = get_rotation();
	grab_handle(prim_idx);

	auto& dvh = ref_debug_visualization_helper();
	//vec3 current_position = accumulate_transforming_hierarchy().col(3);
	//dvh.update_debug_value_vector_position(direction_at_grab_handle, current_position);
	//dvh.update_debug_value_vector_position(direction_currently_handle, current_position);
	//dvh.enable_debug_value_visualization(projected_point_handle);
	//dvh.enable_debug_value_visualization(direction_at_grab_handle);
	//dvh.enable_debug_value_visualization(direction_currently_handle);
}

void cgv::nui::rotation_gizmo::on_handle_released()
{
	release_handles();
}

void cgv::nui::rotation_gizmo::on_handle_drag()
{
	vec3 anchor_obj_parent_global_translation;
	quat anchor_obj_parent_global_rotation;
	vec3 anchor_obj_parent_global_scale;
	transforming::extract_transform_components(transforming::get_global_model_transform(anchor_obj->get_parent()),
		anchor_obj_parent_global_translation, anchor_obj_parent_global_rotation, anchor_obj_parent_global_scale);


	vec3 axis_origin = vec3(0.0f);
	vec3 axis;
	axis = axes_directions[prim_idx];

	// Get actual ring radius from the already calculated splines
	float radius = ring_splines[prim_idx].first.front().length();
	vec3 closest_point;
	if (ii_at_grab.is_pointing) {
		cgv::math::closest_point_on_line_to_circle(ii_during_focus[activating_hid_id].hid_position, ii_during_focus[activating_hid_id].hid_direction,
			axis_origin, axis, radius, closest_point);
	}
	else {
		if (!cgv::math::closest_point_on_circle_to_point(axis_origin, axis, radius,
			ii_during_focus[activating_hid_id].hid_position, closest_point))
			return;
	}

	vec3 direction_at_grab = cross(cross(axis, ii_at_grab.query_point - axis_origin), axis);
	vec3 direction_currently = cross(cross(axis, closest_point - axis_origin), axis);

	if (get_functionality_absolute_axes_rotation() &&
		_functionality_absolute_axes_rotation->get_use_absolute_rotation()) {
		direction_at_grab = anchor_obj_parent_global_rotation.apply(direction_at_grab);
		direction_currently = anchor_obj_parent_global_rotation.apply(direction_currently);
		axis = anchor_obj_parent_global_rotation.apply(axis);
	}
	else
	{
		vec3 anchor_obj_global_translation;
		quat anchor_obj_global_rotation;
		vec3 anchor_obj_global_scale;
		transforming::extract_transform_components(transforming::get_global_model_transform(anchor_obj),
			anchor_obj_global_translation, anchor_obj_global_rotation, anchor_obj_global_scale);
		quat anchor_obj_local_rotation = anchor_obj_parent_global_rotation.inverse() * anchor_obj_global_rotation;
		direction_at_grab = anchor_obj_local_rotation.apply(direction_at_grab);
		direction_currently = anchor_obj_local_rotation.apply(direction_currently);
		axis = anchor_obj_local_rotation.apply(axis);
	}

	float s = dot(cross(direction_at_grab, direction_currently), axis);
	float c = dot(direction_at_grab, direction_currently);
	float da = atan2(s, c);
	quat new_rotation = quat(axis, da);
	if (is_anchor_influenced_by_gizmo)
		set_rotation(new_rotation * get_rotation());
	else
		set_rotation(new_rotation * rotation_at_grab);
}

void cgv::nui::rotation_gizmo::set_axes_directions(std::vector<vec3> axes)
{
	gizmo_functionality_configurable_axes::set_axes_directions(axes);

	// Default configuration
	configure_axes_geometry(1.2f, 0.02f);
}

void cgv::nui::rotation_gizmo::configure_axes_geometry(float ring_radius, float ring_tube_radius)
{
	strs.radius = ring_tube_radius;
	this->ring_spline_radius = ring_tube_radius;
	this->ring_radius = ring_radius;
	// TODO: Update rendering, Handle switching during use
}

bool cgv::nui::rotation_gizmo::_compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx,
	const vec3& scale, const mat4& view_matrix)
{
	compute_geometry(scale);

	size_t idx = -1;
	vec3 p, n;
	float dist_min = std::numeric_limits<float>::max();
	// Get actual ring radius from calculated spline points
	float radius = ring_splines[prim_idx].first.front().length();

	for (size_t i = 0; i < axes_directions.size(); ++i) {
		vec3 origin = vec3(0.0f);
		vec3 d_xz = point - origin;
		d_xz.y() = 0;

		vec3 q0;
		float r0 = d_xz.length();
		if (r0 < 1e-6)
			q0 = origin + (vec3(radius, 0, 0));
		else
			q0 = origin + (radius / r0) * d_xz;
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

bool cgv::nui::rotation_gizmo::_compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal,
	size_t& primitive_idx, const vec3& scale, const mat4& view_matrix)
{
	compute_geometry(scale);

	// Get actual ring radius from calculated spline points
	float radius = ring_splines[prim_idx].first.front().length();
	size_t idx = -1;
	float t = std::numeric_limits<float>::max();
	vec3 n;
	for (size_t i = 0; i < axes_directions.size(); ++i) {
		vec3 n0;
		float t0 = cgv::math::ray_torus_intersection(ray_start, ray_direction,
			vec3(0.0),
			axes_directions[i],
			vec2(radius, ring_spline_radius), n0);
		if (t0 < t) {
			t = t0;
			n = n0;
			idx = i;
		}
	}
	if (t == std::numeric_limits<float>::max()) {
		dehighlight_handles();
		return false;
	}
	hit_param = t;
	hit_normal = n;
	primitive_idx = idx;
	highlight_handle(idx);
	return true;
}

bool cgv::nui::rotation_gizmo::init(cgv::render::context& ctx)
{
	if (!gizmo::init(ctx))
		return false;
	cgv::render::ref_spline_tube_renderer(ctx, 1);
	cgv::render::ref_box_renderer(ctx, 1);

	auto& dvh = cgv::nui::ref_debug_visualization_helper();
	projected_point_handle = dvh.register_debug_value_position(vec3(0.0));
	{
		auto config = dvh.get_config_debug_value_position(projected_point_handle);
		config.color = rgb(1.0, 0.0, 0.0);
		dvh.set_config_debug_value_position(projected_point_handle, config);
	}
	direction_at_grab_handle = dvh.register_debug_value_vector(vec3(1.0));
	direction_currently_handle = dvh.register_debug_value_vector(vec3(1.0));
	dvh.disable_debug_value_visualization(projected_point_handle);
	{
		auto config = dvh.get_config_debug_value_vector(direction_at_grab_handle);
		config.color = rgb(0.0, 1.0, 0.0);
		dvh.set_config_debug_value_vector(direction_at_grab_handle, config);
		config.color = rgb(0.0, 0.0, 1.0);
		dvh.set_config_debug_value_vector(direction_currently_handle, config);
	}
	dvh.disable_debug_value_visualization(direction_at_grab_handle);
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

void cgv::nui::rotation_gizmo::_draw(cgv::render::context& ctx, const vec3& scale, const mat4& view_matrix)
{
	compute_geometry(scale);

	auto& str = cgv::render::ref_spline_tube_renderer(ctx);
	str.set_render_style(strs);
	int i = 0;
	for (auto ring_spline : ring_splines) {
		if (ring_spline.first.empty())
			continue;
		str.set_position_array(ctx, ring_spline.first);
		str.set_tangent_array(ctx, ring_spline.second);
		std::vector<rgb> colors(ring_spline.first.size(), handle_colors[i]);
		str.set_color_array(ctx, colors);
		str.render(ctx, 0, ring_spline.first.size(), true);
		++i;
	}
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
