#pragma once

#include "trect.h"

namespace cgv {
namespace g2d {

enum class Origin {
	kBottomLeft,
	kTopLeft
};

/** Returns the mouse position transformed from FLTK window space to OpenGL
	viewport space depending on the origin setting. The FLTK origin is in the top left
	The OpenGL viewport
	origin is in the bottom left while .
*/

/// @brief Transforms the mouse position according to the given origin.
/// 
/// Transforms the mouse position from FLTK window space to viewport space
/// depending on the origin setting. The FLTK mouse coordinates origin is in the top
/// left corner. If origin is Origin::kTopLeft the original position is returned. If
/// origin is Origin::kBottomLeft the y-coordinate is inverted.
/// 
/// @param mouse_pos The mouse input coordinates.
/// @param viewport_size The viewport size in pixel.
/// @param origin The viewport origin setting.
/// @return The transformed mouse position.
inline ivec2 get_transformed_mouse_pos(const ivec2& mouse_pos, const ivec2& viewport_size, Origin origin = Origin::kBottomLeft) {

	if(origin == Origin::kBottomLeft)
		return ivec2(mouse_pos.x(), viewport_size.y() - mouse_pos.y() - 1);
	else
		return mouse_pos;
}

/// @brief Transforms the mouse position into a coordinate system relative to the given container
/// according to the given origin.
/// 
/// @param mouse_pos The mouse input coordinates.
/// @param viewport_size The viewport size in pixel.
/// @param container The reference container rect.
/// @param origin The viewport origin setting.
/// @return The transformed mouse position.
inline ivec2 get_local_mouse_pos(const ivec2& mouse_pos, const ivec2& viewport_size, const cgv::g2d::irect& container, Origin origin = Origin::kBottomLeft) {

	return get_transformed_mouse_pos(mouse_pos, viewport_size, origin) - container.position;
}

}
}
