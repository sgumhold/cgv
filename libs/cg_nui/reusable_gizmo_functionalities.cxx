#include "reusable_gizmo_functionalities.h"

bool cgv::nui::gizmo_functionality_configurable_axes::validate_axes()
{
	return axes_directions.size() == scale_dependent_axes_positions.size()
		&& axes_directions.size() == scale_independent_axes_positions.size();
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
