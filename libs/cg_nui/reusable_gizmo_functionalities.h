#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		using namespace render;

// Common helper functions

/// Fill given vector up with last value if too few values are contained in it
template<typename T>
void fill_with_last_value_if_not_full(std::vector<T>& to_fill, size_t required_size)
{
	const int size_diff = required_size - to_fill.size();
	if (size_diff > 0) {
		T last_value = to_fill.back();
		for (int i = 0; i < size_diff; ++i) {
			to_fill.push_back(last_value);
		}
	}
}

/// Abstraction for axes that can be positioned in the gizmo coordinate system.
/// This can be used as a basis for any handles that point in a specific direction.
class CGV_API gizmo_functionality_configurable_axes
{
protected:
	// configuration
	/// Directions of the axes, also defines the number of axes.
	std::vector<vec3> axes_directions;
	/// Scale dependent positions are multiplied by the anchor scale.
	std::vector<vec3> scale_dependent_axes_positions;
	/// Scale independent axes used as is.
	std::vector<vec3> scale_independent_axes_positions;

	/// Validate the configuration of the axes. Has to be called during validation at attach. Requires the number of axes that are supposed to exist.
	bool validate_axes();
public:
	// Configuration functions
	/// Set axes directions
	virtual void set_axes_directions(std::vector<vec3> axes_directions);
	/// Set axes positions. Scale dependent positions are multiplied by the anchor scale, scale independent positions are added as is.
	void set_axes_positions(std::vector<vec3> scale_dependent_axis_positions, std::vector<vec3> scale_independent_axis_positions);
	/// Set the scale dependent axis positions, which are multiplied by the anchor scale.
	void set_axes_scale_dependent_positions(std::vector<vec3> scale_dependent_axis_positions);
	/// Set the scale independent axis positions, which are used as is.
	void set_axes_scale_independent_positions(std::vector<vec3> scale_independent_axis_positions);
};

/// Abstraction that keeps the states of a list of handles that change color if highlighted or grabbed.
///	The colors can be set individually for each handle or once for all handles.
class CGV_API gizmo_functionality_handle_states
{
protected:
	// internal state
	/// The current color for each handle
	std::vector<rgb> handle_colors;
	/// Flag for each handle set if it is highlighted
	std::vector<bool> handles_highlighted;
	/// Flag for each handle set if it is grabbed
	std::vector<bool> handles_grabbed;

	// configuration
	/// Neutral state color for each handle (or one for all handles)
	std::vector<rgb> handle_base_colors{ rgb(0.11f, 0.35f, 0.62f) };
	/// Highlighted state color for each handle (or one for all handles)
	std::vector<rgb> handle_highlight_colors{ rgb(0.255f, 0.980f, 0.714f) };
	/// Grabbed state color for each handle (or one for all handles)
	std::vector<rgb> handle_grab_colors{ rgb(0.255f, 0.980f, 0.3f) };

	/// Validate the configuration of the axes. Has to be called during validation at attach. Requires the number of handles that are supposed to exist.
	bool validate_handles(int handle_count);

	/// Set handle as grabbed
	void grab_handle(int handle_idx);
	/// Set handle as not grabbed
	void release_handle(int handle_idx);
	/// Set all handles as not grabbed
	void release_handles();
	/// Set handle as highlighted
	void highlight_handle(int handle_idx);
	/// Set handle as not highlighted
	void dehighlight_handle(int handle_idx);
	/// Set all handles as not highlighted
	void dehighlight_handles();
public:
	// Configuration functions
	/// Set one base color to be used by all handles
	void set_handle_base_color(rgb color) { handle_base_colors = { color }; }
	/// Set different base colors for each handle. If less colors then handles are given then the last color will be repeated.
	void set_handle_base_colors(std::vector<rgb> colors) { handle_base_colors = colors; }
	/// Set one highlight color to be used by all handles
	void set_handle_highlight_color(rgb color) { handle_highlight_colors = { color }; }
	/// Set different highlight colors for each handle. If less colors then handles are given then the last color will be repeated.
	void set_handle_highlight_colors(std::vector<rgb> colors) { handle_highlight_colors = colors; }
	/// Set one grab color to be used by all handles
	void set_handle_grab_color(rgb color) { handle_highlight_colors = { color }; }
	/// Set different grab colors for each handle. If less colors then handles are given then the last color will be repeated.
	void set_handle_grab_colors(std::vector<rgb> colors) { handle_highlight_colors = colors; }
};

	}
}

#include <cgv/config/lib_end.h>