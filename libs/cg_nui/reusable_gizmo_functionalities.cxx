#include "reusable_gizmo_functionalities.h"

void cgv::nui::gizmo_functionality_local_world_orientation::use_world_coordinate_system()
{
	use_local_coords = false;
	// TODO: Handle switching during use, update rendering?
}

void cgv::nui::gizmo_functionality_local_world_orientation::use_local_coordinate_system()
{
	use_local_coords = true;
	// TODO: Handle switching during use, update rendering?
}

void cgv::nui::gizmo_functionality_configurable_axes::compute_absolute_axis_parameters(vec3 anchor_position,
	quat anchor_rotation, vec3 anchor_scale, bool consider_local_rotation)
{
	absolute_axes_directions.clear();
	absolute_axes_positions.clear();
	for (int i = 0; i < axes_directions.size(); ++i) {
		if (consider_local_rotation)
		{
			absolute_axes_positions.push_back(anchor_position + anchor_rotation.apply(
				scale_dependent_axes_positions[i] * anchor_scale + scale_independent_axes_positions[i]
			));
			absolute_axes_directions.push_back(anchor_rotation.apply(axes_directions[i]));
		}
		else
		{
			absolute_axes_positions.push_back(anchor_position + scale_dependent_axes_positions[i] * anchor_scale + scale_independent_axes_positions[i]);
			absolute_axes_directions.push_back(axes_directions[i]);
		}
	}
}

void cgv::nui::gizmo_functionality_configurable_axes::configure_axes_directions(std::vector<vec3> axes_directions)
{
	this->axes_directions = axes_directions;
	for (int i = 0; i < this->axes_directions.size(); ++i) {
		this->axes_directions[i].normalize();
	}
	// Default configuration
	for (int i = 0; i < this->axes_directions.size(); ++i) {
		this->scale_dependent_axes_positions.push_back(vec3(0.0));
		this->scale_independent_axes_positions.push_back(vec3(0.0));
	}
}

void cgv::nui::gizmo_functionality_configurable_axes::configure_axes_positioning(
	std::vector<vec3> scale_dependent_axis_positions, std::vector<vec3> scale_independent_axis_positions)
{
	configure_axes_positioning_scale_dependent(scale_dependent_axis_positions);
	configure_axes_positioning_scale_independent(scale_independent_axis_positions);
}

void cgv::nui::gizmo_functionality_configurable_axes::configure_axes_positioning_scale_dependent(
	std::vector<vec3> scale_dependent_axis_positions)
{
	this->scale_dependent_axes_positions = scale_dependent_axis_positions;
	fill_with_last_value_if_not_full<vec3>(this->scale_dependent_axes_positions, axes_directions.size());
}

void cgv::nui::gizmo_functionality_configurable_axes::configure_axes_positioning_scale_independent(
	std::vector<vec3> scale_independent_axis_positions)
{
	this->scale_independent_axes_positions = scale_independent_axis_positions;
	fill_with_last_value_if_not_full<vec3>(this->scale_independent_axes_positions, axes_directions.size());
}
