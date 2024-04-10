#pragma once

#include <cgv/math/fvec.h>
#include <cgv/media/axis_aligned_box.h>

namespace cgv {
namespace render {

/// This type provides a simple helper class to store rectangles with texture coordinates.
struct textured_rectangle {
	box2 rectangle;
	vec4 texcoords;
};

} // namespace render
} // namespace cgv