#include "reusable_gizmo_functionalities.h"

bool cgv::nui::gizmo_functionality_configurable_axes::validate_axes()
{
	return axes_directions.size() == scale_dependent_axes_positions.size()
		&& axes_directions.size() == scale_independent_axes_positions.size();
}

void cgv::nui::gizmo_functionality_configurable_axes::set_axes_directions(std::vector<vec3> axes_directions)
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

void cgv::nui::gizmo_functionality_configurable_axes::set_axes_positions(
	std::vector<vec3> scale_dependent_axis_positions, std::vector<vec3> scale_independent_axis_positions)
{
	set_axes_scale_dependent_positions(scale_dependent_axis_positions);
	set_axes_scale_independent_positions(scale_independent_axis_positions);
}

void cgv::nui::gizmo_functionality_configurable_axes::set_axes_scale_dependent_positions(
	std::vector<vec3> scale_dependent_axis_positions)
{
	this->scale_dependent_axes_positions = scale_dependent_axis_positions;
	fill_with_last_value_if_not_full<vec3>(this->scale_dependent_axes_positions, axes_directions.size());
}

void cgv::nui::gizmo_functionality_configurable_axes::set_axes_scale_independent_positions(
	std::vector<vec3> scale_independent_axis_positions)
{
	this->scale_independent_axes_positions = scale_independent_axis_positions;
	fill_with_last_value_if_not_full<vec3>(this->scale_independent_axes_positions, axes_directions.size());
}

void cgv::nui::gizmo_functionality_absolute_axes_rotation::set_use_absolute_rotation(bool value)
{
	use_absolute_rotation = value;
}

bool cgv::nui::gizmo_functionality_absolute_axes_rotation::get_use_absolute_rotation()
{
	return use_absolute_rotation;
}

void cgv::nui::gizmo_functionality_absolute_axes_position::set_use_absolute_position(bool value)
{
	use_absolute_position = value;
}

bool cgv::nui::gizmo_functionality_absolute_axes_position::get_use_absolute_position()
{
	return use_absolute_position;
}

bool cgv::nui::gizmo_functionality_handle_states::validate_handles(int handle_count)
{
	// Make sure the color lists are filled with as many colors as there are handles
	fill_with_last_value_if_not_full(handle_base_colors, handle_count);
	if (handle_base_colors.size() < handle_count) return false;
	fill_with_last_value_if_not_full(handle_highlight_colors, handle_count);
	if (handle_highlight_colors.size() < handle_count) return false;
	fill_with_last_value_if_not_full(handle_grab_colors, handle_count);
	if (handle_grab_colors.size() < handle_count) return false;
	// Fill current handle colors with base colors
	handle_colors.clear();
	handles_highlighted.clear();
	handles_grabbed.clear();
	handle_colors.reserve(handle_count);
	handles_highlighted.reserve(handle_count);
	handles_grabbed.reserve(handle_count);
	for (int i = 0; i < handle_count; ++i) {
		handle_colors.push_back(handle_base_colors[i]);
		handles_highlighted.push_back(false);
		handles_grabbed.push_back(false);
	}

	return true;
}

void cgv::nui::gizmo_functionality_handle_states::grab_handle(int handle_idx)
{
	if (handle_idx < 0 || handle_idx >= handle_colors.size())
		return;

	handles_grabbed[handle_idx] = true;
	handle_colors[handle_idx] = handle_grab_colors[handle_idx];
}

void cgv::nui::gizmo_functionality_handle_states::release_handle(int handle_idx)
{
	if (handle_idx < 0 || handle_idx >= handle_colors.size())
		return;

	handles_grabbed[handle_idx] = false;
	if (handles_highlighted[handle_idx])
		handle_colors[handle_idx] = handle_highlight_colors[handle_idx];
	else
		handle_colors[handle_idx] = handle_base_colors[handle_idx];
}

void cgv::nui::gizmo_functionality_handle_states::release_handles()
{
	for (int i = 0; i < handle_colors.size(); ++i) {
		handles_grabbed[i] = false;
		if (handles_highlighted[i])
			handle_colors[i] = handle_highlight_colors[i];
		else
			handle_colors[i] = handle_base_colors[i];
	}
}

void cgv::nui::gizmo_functionality_handle_states::highlight_handle(int handle_idx)
{
	if (handle_idx < 0 || handle_idx >= handle_colors.size())
		return;

	handles_highlighted[handle_idx] = true;
	if (!handles_grabbed[handle_idx])
		handle_colors[handle_idx] = handle_highlight_colors[handle_idx];
}

void cgv::nui::gizmo_functionality_handle_states::dehighlight_handle(int handle_idx)
{
	if (handle_idx < 0 || handle_idx >= handle_colors.size())
		return;

	handles_highlighted[handle_idx] = false;
	if (!handles_grabbed[handle_idx])
		handle_colors[handle_idx] = handle_base_colors[handle_idx];
}

void cgv::nui::gizmo_functionality_handle_states::dehighlight_handles()
{
	for (int i = 0; i < handle_colors.size(); ++i) {
		handles_highlighted[i] = false;
		if (!handles_grabbed[i])
			handle_colors[i] = handle_base_colors[i];
	}
}
