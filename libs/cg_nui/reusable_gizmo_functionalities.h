#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

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


class CGV_API gizmo_functionality_configurable_axes : public render::render_types
{
protected:
	// configuration
	std::vector<vec3> axes_directions;
	std::vector<vec3> scale_dependent_axes_positions;
	std::vector<vec3> scale_independent_axes_positions;

	/// Validate the configuration of the axes. Has to be called during validation at attach.
	bool validate_axes();
public:
	// Configuration functions
	/// Set axes directions
	virtual void configure_axes_directions(std::vector<vec3> axes_directions);
	/// Set axes positions. Scale dependent positions are multiplied by the anchor scale, scale independent positions are added as is.
	void configure_axes_positioning(std::vector<vec3> scale_dependent_axis_positions, std::vector<vec3> scale_independent_axis_positions);
	void configure_axes_positioning_scale_dependent(std::vector<vec3> scale_dependent_axis_positions);
	void configure_axes_positioning_scale_independent(std::vector<vec3> scale_independent_axis_positions);
};

	}
}

#include <cgv/config/lib_end.h>