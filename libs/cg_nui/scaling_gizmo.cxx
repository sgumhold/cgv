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
	int handle_count = axes_directions.size();
	cube_positions.resize(handle_count);
	handle_positions.resize(handle_count);
	for (int i = 0; i < handle_count; ++i) {
		splines.push_back(spline_data_t());
		splines.back().first.resize(2);
		splines.back().second.resize(2);
		handle_directions.push_back(axes_directions[i] * scaling_axes_length);
		quat rot;
		vec3 axis = axes_directions[i];
		if (axis == vec3(-1.0, 0.0, 0.0))
			axis = axis * -1.0f;
		rot.set_normal(axis);
		cube_rotations.push_back(rot);
	}
}

void cgv::nui::scaling_gizmo::compute_geometry(const vec3& scale)
{
	for (int i = 0; i < axes_directions.size(); ++i) {
		vec3 start_point = scale_dependent_axes_positions[i] * scale + scale_independent_axes_positions[i];
		vec3 end_point = start_point + axes_directions[i] * scaling_axes_length;
		vec3 axis = axes_directions[i];
		splines[i].first[0] = start_point;
		splines[i].second[0] = vec4(axis, 0.0);
		splines[i].first[1] = end_point;
		splines[i].second[1] = vec4(axis, 0.0);
		cube_positions[i] = end_point;
		handle_positions[i] = start_point;
	}
}

void cgv::nui::scaling_gizmo::set_scale_reference(vec3* _scale_ptr, cgv::base::base_ptr _on_set_obj)
{
	scale_ptr = _scale_ptr;
	gizmo::set_on_set_object(_on_set_obj);
}

void cgv::nui::scaling_gizmo::set_scale_reference(vec3** _scale_ptr_ptr, cgv::base::base_ptr _on_set_obj)
{
	scale_ptr_ptr = _scale_ptr_ptr;
	gizmo::set_on_set_object(_on_set_obj);
}

void cgv::nui::scaling_gizmo::set_scale_reference(scalable* _scalable_obj)
{
	scalable_obj = _scalable_obj;
}

bool cgv::nui::scaling_gizmo::validate_configuration()
{
	bool configuration_valid = true;

	if (!(
		scale_ptr ||
		(scale_ptr_ptr && *scale_ptr_ptr) ||
		scalable_obj
		)) {
		std::cout << "Scaling gizmo requires a valid pointer to a scale or a pointer to a pointer to a scale or a reference to an object implementing scalable" << std::endl;
		configuration_valid = false;
	}

	configuration_valid = configuration_valid && validate_axes();
	configuration_valid = configuration_valid && validate_handles(axes_directions.size());

	return configuration_valid && gizmo::validate_configuration();
}

void cgv::nui::scaling_gizmo::on_handle_grabbed()
{
	scale_at_grab = get_scale();
	vec3 obj_translation;
	quat obj_rotation;
	vec3 obj_scale;
	transforming::extract_transform_components(transforming::get_global_model_transform(anchor_obj), obj_translation, obj_rotation, obj_scale);
	distance_at_grab = (ii_at_grab.query_point - obj_translation).length();
	grab_handle(prim_idx);
}

void cgv::nui::scaling_gizmo::on_handle_released()
{
	release_handles();
}

void cgv::nui::scaling_gizmo::on_handle_drag()
{
	vec3 obj_translation;
	quat obj_rotation;
	vec3 obj_scale;
	transforming::extract_transform_components(transforming::get_global_model_transform(anchor_obj), obj_translation, obj_rotation, obj_scale);

	vec3 axis;
	if (use_root_rotation) {
		axis = axes_directions[prim_idx];
	}
	else {
		axis = obj_rotation.inverse().get_homogeneous_matrix() * vec4(axes_directions[prim_idx], 0);
	}
	if (anchor_rotation_ptr)
		axis = anchor_rotation_ptr->get_homogeneous_matrix() * vec4(axis, 0.0f);
	else if (anchor_rotation_ptr_ptr)
		axis = (*anchor_rotation_ptr_ptr)->get_homogeneous_matrix() * vec4(axis, 0.0f);

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
	vec3 scale_ratio = scaling_axes_scale_ratios[prim_idx];
	float current_distance = (closest_point - obj_translation).length();
	if (is_anchor_influenced_by_gizmo)
	{
		distance_at_grab = (ii_at_grab.query_point - obj_translation).length();
		set_scale(get_scale() * (vec3(1.0f) - scale_ratio) + (get_scale() * current_distance / distance_at_grab) * scale_ratio);
	}
	else
	{
		set_scale(scale_at_grab * (vec3(1.0f) - scale_ratio) + (scale_at_grab * current_distance / distance_at_grab) * scale_ratio);
	}

	//vec3 current_position = accumulate_transforming_hierarchy().col(3);
	//float distance_at_grab = (ii_at_grab.query_point - current_position).length();
	//float current_distance = (closest_point - current_position).length();
	//
	//vec3 scale_ratio = scaling_axes_scale_ratios[prim_idx];
	//*scale_ptr = scale_at_grab * (vec3(1.0) - scale_ratio) +  (scale_at_grab * current_distance / distance_at_grab) * scale_ratio;
}

void cgv::nui::scaling_gizmo::set_axes_directions(std::vector<vec3> axes_directions)
{
	gizmo_functionality_configurable_axes::set_axes_directions(axes_directions);
	// Default configuration
	for (int i = 0; i < this->axes_directions.size(); ++i) {
		scaling_axes_scale_ratios.push_back(abs(axes_directions[i]));
	}
	configure_axes_geometry(0.015f, 0.2f, 0.035f);
}

void cgv::nui::scaling_gizmo::configure_axes_geometry(float radius, float length, float cube_size)
{
	this->spline_tube_radius = radius;
	strs.radius = radius;
	this->scaling_axes_length = length;
	this->cube_size = cube_size;
}

void cgv::nui::scaling_gizmo::configure_axes_scale_ratios(std::vector<vec3> scale_ratios)
{
	scaling_axes_scale_ratios.clear();
	for (auto scale_ratio : scale_ratios) {
		scaling_axes_scale_ratios.push_back(abs(scale_ratio));
	}
}

bool cgv::nui::scaling_gizmo::_compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx, const vec3& scale,
                                                     const mat4& view_matrix)
{
	compute_geometry(scale);

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

bool cgv::nui::scaling_gizmo::_compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal,
                                                    size_t& primitive_idx, const vec3& scale, const mat4& view_matrix)
{
	compute_geometry(scale);

	size_t idx = -1;
	float t = std::numeric_limits<float>::max();
	vec3 n;
	for (size_t i = 0; i < axes_directions.size(); ++i) {
		vec3 n0;
		float t0 = cgv::math::ray_cylinder_intersection(
			ray_start, ray_direction,
			handle_positions[i],
			handle_directions[i],
			spline_tube_radius, n0);
		if (t0 < t) {
			t = t0;
			n = n0;
			idx = i;
		}
	}
	
	if (t == std::numeric_limits<float>::max())
	{
		dehighlight_handles();
		return false;
	}
	hit_param = t;
	hit_normal = n;
	primitive_idx = idx;
	highlight_handle(idx);
	return true;
}

cgv::render::render_types::vec3 cgv::nui::scaling_gizmo::get_scale()
{
	if (scalable_obj)
		return scalable_obj->get_scale();
	if (scale_ptr_ptr)
		return **scale_ptr_ptr;
	return *scale_ptr;
}

void cgv::nui::scaling_gizmo::set_scale(const vec3& scale)
{
	if (scalable_obj) {
		scalable_obj->set_scale(scale);
	}
	else {
		if (scale_ptr_ptr)
			**scale_ptr_ptr = scale;
		else
			*scale_ptr = scale;
		if (on_set_obj)
			on_set_obj->on_set(scale_ptr);
	}
}

bool cgv::nui::scaling_gizmo::init(cgv::render::context& ctx)
{
	if (!gizmo::init(ctx))
		return false;
	cgv::render::ref_spline_tube_renderer(ctx, 1);
	cgv::render::ref_box_renderer(ctx, 1);
	return true;
}

void cgv::nui::scaling_gizmo::clear(cgv::render::context& ctx)
{
	cgv::render::ref_box_renderer(ctx, -1);
	cgv::render::ref_spline_tube_renderer(ctx, -1);
	gizmo::clear(ctx);
}

void cgv::nui::scaling_gizmo::_draw(cgv::render::context& ctx, const vec3& scale, const mat4& view_matrix)
{
	compute_geometry(scale);

	auto& str = cgv::render::ref_spline_tube_renderer(ctx);
	str.set_render_style(strs);
	int i = 0;
	for (auto spline : splines) {
		if (spline.first.empty())
			continue;
		str.set_position_array(ctx, spline.first);
		str.set_tangent_array(ctx, spline.second);
		std::vector<rgb> colors(spline.first.size(), handle_colors[i]);
		str.set_color_array(ctx, colors);
		str.render(ctx, 0, spline.first.size(), true);
		++i;
	}

	auto& br = cgv::render::ref_box_renderer(ctx);
	br.set_render_style(brs);
	br.set_position_array(ctx, cube_positions);
	br.set_color_array(ctx, &handle_colors[0], handle_colors.size());
	br.set_extent(ctx, vec3(cube_size));
	br.set_rotation_array(ctx, &cube_rotations[0], cube_rotations.size());
	br.render(ctx, 0, cube_positions.size());
}

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
