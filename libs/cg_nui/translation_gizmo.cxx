#include "translation_gizmo.h"

#include <cgv/math/intersection.h>
#include <cgv/math/proximity.h>
#include <cg_nui/translatable.h>

#include <cg_nui/debug_visualization_helper.h>

void cgv::nui::translation_gizmo::precompute_geometry()
{
	arrow_positions.clear();
	arrow_directions.clear();
	for (int i = 0; i < axes_directions.size(); ++i) {
		arrow_positions.push_back(vec3(0.0f));
		arrow_directions.push_back(axes_directions[i] * translation_axes_length);
	}
}

void cgv::nui::translation_gizmo::compute_geometry(const vec3& scale)
{
	for (int i = 0; i < axes_directions.size(); ++i) {
		arrow_positions[i] = scale_dependent_axes_positions[i] * scale + scale_independent_axes_positions[i];
	}
}

bool cgv::nui::translation_gizmo::validate_configuration()
{
	bool configuration_valid = true;

	if (!(
		position_ptr ||
		(position_ptr_ptr && *position_ptr_ptr) ||
		translatable_obj
		)) {
		std::cout << "Translation gizmo requires a valid pointer to a position, or a pointer to a pointer to a position, or a reference to an object implementing translatable" << std::endl;
		configuration_valid = false;
	}

	configuration_valid = configuration_valid && validate_axes();
	configuration_valid = configuration_valid && validate_handles(axes_directions.size());

	return configuration_valid && gizmo::validate_configuration();
}

void cgv::nui::translation_gizmo::on_handle_grabbed()
{
	position_at_grab = get_position();
	grab_handle(prim_idx);
}

void cgv::nui::translation_gizmo::on_handle_released()
{
	release_handles();
}

void cgv::nui::translation_gizmo::on_handle_drag()
{
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

	vec3 movement = closest_point - ii_at_grab.query_point;

	// Transform movement into value object' parent coordinate system
	movement = gizmo_to_value_parent_transform_vector(movement);

	// If the position that this gizmo changes influences the anchor of this gizmo, then the movement is an incremental update.
	// Otherwise the movement is relative to the original position of the anchor at the time of grabbing.
	if (is_anchor_influenced_by_gizmo)
		set_position(get_position() + movement);
	else
		set_position(position_at_grab + movement);
}

cgv::render::render_types::vec3 cgv::nui::translation_gizmo::get_position()
{
	if (translatable_obj)
		return translatable_obj->get_position();
	if (position_ptr_ptr)
		return **position_ptr_ptr;
	return *position_ptr;
}

void cgv::nui::translation_gizmo::set_position(const vec3& position)
{
	if (translatable_obj) {
		translatable_obj->set_position(position);
	}
	else {
		if (position_ptr_ptr)
			**position_ptr_ptr = position;
		else
			*position_ptr = position;
		if (on_set_obj)
			on_set_obj->on_set(position_ptr);
	}
}

void cgv::nui::translation_gizmo::set_position_reference(vec3* _position_ptr, cgv::base::base_ptr _on_set_obj)
{
	position_ptr = _position_ptr;
	gizmo::set_on_set_object(_on_set_obj);
}

void cgv::nui::translation_gizmo::set_position_reference(vec3** _position_ptr_ptr, cgv::base::base_ptr _on_set_obj)
{
	position_ptr_ptr = _position_ptr_ptr;
	gizmo::set_on_set_object(_on_set_obj);
}

void cgv::nui::translation_gizmo::set_position_reference(translatable* _translatable_obj)
{
	translatable_obj = _translatable_obj;
}

void cgv::nui::translation_gizmo::set_axes_directions(std::vector<vec3> axes)
{
	gizmo_functionality_configurable_axes::set_axes_directions(axes);

	// Default configuration
	configure_axes_geometry(0.015f, 0.2f);
}

void cgv::nui::translation_gizmo::configure_axes_geometry(float radius, float length)
{
	ars.radius_relative_to_length = 0;
	ars.radius_lower_bound = radius;
	arrow_radius = radius;
	translation_axes_length = length;
}

bool cgv::nui::translation_gizmo::_compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal,
	size_t& primitive_idx, const vec3& scale, const mat4& view_matrix)
{
	compute_geometry(scale);

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

bool cgv::nui::translation_gizmo::_compute_intersection(const vec3& ray_start, const vec3& ray_direction,
	float& hit_param, vec3& hit_normal, size_t& primitive_idx, const vec3& scale, const mat4& view_matrix)
{
	compute_geometry(scale);

	size_t idx = -1;
	float t = std::numeric_limits<float>::max();
	vec3 n;
	for (size_t i = 0; i < arrow_positions.size(); ++i) {
		vec3 n0;
		// DEBUG TO REMOVE - Simple box version for testing purposes (working)
		vec3 ro = ray_start - arrow_positions[i] - (arrow_directions[i] / 2.0f);
		vec3 norm_arrow_direction = arrow_directions[i];
		norm_arrow_direction.normalize();
		vec3 orth_arrow_direction_mask = vec3(1.0f) - norm_arrow_direction;
		vec3 box_half_extent = (vec3(arrow_radius) * orth_arrow_direction_mask + arrow_directions[i]) * 0.5f;
		vec2 res;
		vec3 normal;
		int n_intersections = cgv::math::ray_box_intersection(cgv::math::ray<float, 3>(ro, ray_direction), box_half_extent, res, &normal);
		if (n_intersections > 0 && res[0] < t) {
			t = res[0];
			n = normal;
			idx = i;
		}

		// Using simplified ray cylinder intersection (not working at all for unknown reasons)
		/*
		quat arrow_rot;
		arrow_rot.set_normal(normalize(arrow_directions[i]));
		quat correction_rot = arrow_rot.inverse();
		vec3 ro = ray_start - arrow_positions[i];
		correction_rot.rotate(ro);
		vec3 rd = ray_direction;
		correction_rot.rotate(rd);
		if (i == 0) {
			dvh.update_debug_value_cylinder(debug_cylinder_handle0, intersection_debug_position, vec3(arrow_directions[i].length(), 0.0, 0.0), arrow_radius);
			dvh.update_debug_value_ray(debug_ray_handle0, ro + intersection_debug_position, rd);
		}
		auto res = cgv::math::ray_cylinder_intersection(ro, ray_direction, arrow_directions[i].length(), arrow_radius);
		if (res.hit && res.t_near < t) {
			t = res.t_near;
			n = vec3(1.0f, 0.0f, 0.0f);
			idx = i;
		}
		*/

		// Old ray cylinder intersection (not working for the case of root = table for unknown reasons)
		/*
		float t0 = cgv::math::ray_cylinder_intersection(ray_start, ray_direction, arrow_positions[i], arrow_directions[i], arrow_radius, n0);
		if (t0 < t) {
			t = t0;
			n = n0;
			idx = i;
		}
		*/
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

bool cgv::nui::translation_gizmo::init(cgv::render::context& ctx)
{
	if (!gizmo::init(ctx))
		return false;
	cgv::render::ref_arrow_renderer(ctx, 1);

	// Example of using the debug visualization helper
	// Retrieve the debug visualization helper singleton instance. Increase ref count (same as with primitive renderer instances).
	auto& dvh = cgv::nui::ref_debug_visualization_helper(ctx, 1);
	// Register a new debug primitive, in this case a coordinate frame. Save its handle for later.
	debug_coord_system_handle = dvh.register_debug_value_coordinate_system();
	{
		// Retrieve the current config of the primitive using its handle. This can be done at any time, here it is done for the initial configuration.
		auto config = dvh.get_config_debug_value_coordinate_system(debug_coord_system_handle);
		// Change any values of the config as needed.
		config.show_translation = false;
		config.position = vec3(0.0f, 2.0f, 0.0f);
		// Write back the modified configuration to make it active.
		dvh.set_config_debug_value_coordinate_system(debug_coord_system_handle, config);
		// This is only to hide the debug primitive as this is only meant as an example. This line should be removed for an actual use of the helper.
		// The corresponding enable function would make the debug primitive visible again.
		dvh.disable_debug_value_visualization(debug_coord_system_handle);
	}

	return true;
}

void cgv::nui::translation_gizmo::clear(cgv::render::context& ctx)
{
	// Example of using the debug visualization helper
	// Deregister debug primitive. Also decrement ref count of debug visualization helper (same as with renderer instances).
	auto& dvh = ref_debug_visualization_helper();
	dvh.deregister_debug_value(debug_coord_system_handle);
	cgv::nui::ref_debug_visualization_helper(ctx, -1);

	cgv::render::ref_arrow_renderer(ctx, -1);
	gizmo::clear(ctx);
}

void cgv::nui::translation_gizmo::_draw(cgv::render::context& ctx, const vec3& scale, const mat4& view_matrix)
{
	compute_geometry(scale);

	// Example of using the debug visualization helper
	// Set a new value for the debug primitive. This can be done at any time (does not have to happen every frame).
	ref_debug_visualization_helper().update_debug_value_coordinate_system(debug_coord_system_handle, get_global_model_transform(anchor_obj));

	if (!arrow_directions.empty()) {
		auto& ar = cgv::render::ref_arrow_renderer(ctx);
		ar.set_render_style(ars);

		ar.set_position_array(ctx, arrow_positions);
		ar.set_direction_array(ctx, arrow_directions);
		ar.set_color_array(ctx, handle_colors);
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
