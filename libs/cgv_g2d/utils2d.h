#pragma once

#include <cgv/render/render_types.h>

#include "rect.h"

namespace cgv {
namespace g2d {

/** Returns the mouse position transformed from FLTK window to OpenGL
	viewport space defined by viewport_size. The OpenGL viewport
	origin is in the bottom left while the FLTK origin is in the top left.
*/
inline cgv::render::ivec2 get_transformed_mouse_pos(const cgv::render::ivec2& mouse_pos, const cgv::render::ivec2& viewport_size) {

	return cgv::render::ivec2(mouse_pos.x(), viewport_size.y() - mouse_pos.y() - 1);
	//return cgv::render::ivec2(mouse_pos.x(), mouse_pos.y());
}

/** Returns the mouse position in OpenGL viewport space local to the
	given container.
*/
inline cgv::render::ivec2 get_local_mouse_pos(const cgv::render::ivec2& mouse_pos, const cgv::render::ivec2& viewport_size, const cgv::g2d::irect& container) {

	return get_transformed_mouse_pos(mouse_pos, viewport_size) - container.position;
}

}
}
