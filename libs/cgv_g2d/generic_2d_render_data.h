#pragma once

#include <cgv_gl/generic_render_data.h>

/* Define some presets */
namespace cgv {
namespace g2d {

/// Defines a generic render data class using attributes:
/// vec2 position
DEFINE_GENERIC_RENDER_DATA_CLASS(generic_render_data_vec2, 1, vec2, position);

/// Defines a generic render data class using attributes:
/// vec2 position
/// rgb color
DEFINE_GENERIC_RENDER_DATA_CLASS(generic_render_data_vec2_rgb, 2, vec2, position, rgb, color);

/// Defines a generic render data class using attributes:
/// vec2 position
/// rgba color
DEFINE_GENERIC_RENDER_DATA_CLASS(generic_render_data_vec2_rgba, 2, vec2, position, rgba, color);

/// Defines a generic render data class using attributes:
/// vec2 position
/// vec2 size
DEFINE_GENERIC_RENDER_DATA_CLASS(generic_render_data_vec2_vec2, 2, vec2, position, vec2, size);

/// Defines a generic render data class using attributes:
/// vec2 position
/// vec2 size
/// rgb color
DEFINE_GENERIC_RENDER_DATA_CLASS(generic_render_data_vec2_vec2_rgb, 3, vec2, position, vec2, size, rgb, color);

/// Defines a generic render data class using attributes:
/// vec2 position
/// vec2 size
/// rgba color
DEFINE_GENERIC_RENDER_DATA_CLASS(generic_render_data_vec2_vec2_rgba, 3, vec2, position, vec2, size, rgba, color);

} // namespace cgv
} // namespace g2d
