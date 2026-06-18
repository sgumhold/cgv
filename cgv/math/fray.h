#pragma once

#include "fmat.h"
#include "fvec.h"

namespace cgv {
namespace math {

/// Struct template for fixed n-dimensional rays with arbitrary data type defined by origin and direction.
template <typename T, cgv::type::uint32_type N>
struct fray {
	fvec<T, N> origin;
	fvec<T, N> direction;

	/// Construct an invalid ray with origin at zero and undefined direction.
	fray() {
		origin = fvec<T, N>(0);
		direction = fvec<T, N>(0);
	}

	/// Construct a ray with given origin and direction.
	fray(const fvec<T, N>& origin, const fvec<T, N>& direction) : origin(origin), direction(direction) {}

	/// Return the position of the ray at the given distance (ray parameter t) from its origin.
	fvec<T, N> position(float t) const {
		return origin + t * direction;
	}
};

/// Partial specialization of fray class for 3 dimensions.
template <typename T>
struct fray<T, 3> {
	fvec<T, 3> origin;
	fvec<T, 3> direction;

	/// Construct an invalid ray with origin at zero and undefined direction.
	fray() {
		origin = fvec<T, 3>(0);
		direction = fvec<T, 3>(0);
	}

	/// Construct a ray with given origin and direction.
	fray(const fvec<T, 3>& origin, const fvec<T, 3>& direction) : origin(origin), direction(direction) {}

	/// @brief Construct a ray from a given screen coordinate and view parameters. This is typically used to create a
	/// viewing ray that originates at the eye and goes through a pixel on the screen.
	/// 
	/// @param screen_coord The screen pixel coordinate with (0,0) in the bottom left corner. 
	/// @param viewport_size The screen size in pixels.
	/// @param eye_position The location of the eye/camera.
	/// @param view_projection_matrix The current view and projection matrix with no (or identity) model transformation.
	fray(const fvec<T, 2>& screen_coord, const fvec<unsigned, 2>& viewport_size, const fvec<T, 3>& eye_position, const fmat<T, 4, 4>& view_projection_matrix) {
		fvec<T, 2> window_coord = T(2) * screen_coord / static_cast<fvec<T, 2>>(viewport_size) - T(1);

		fvec<T, 4> world_coord(window_coord.x(), window_coord.y(), T(1), T(1));
		world_coord = inverse(view_projection_matrix) * world_coord;
		world_coord /= world_coord.w();

		origin = eye_position;
		direction = normalize(fvec<T, 3>(world_coord) - eye_position);
	}

	/// Return the position of the ray at the given distance (ray parameter t) from its origin.
	fvec<T, 3> position(T t) const {
		return origin + t * direction;
	}
};

// Predefined ray types
/// 2d ray using 32 bit unsigned integer type
typedef fray<unsigned, 2> uray2;
/// 3d ray using 32 bit unsigned integer type
typedef fray<unsigned, 3> uray3;
/// 4d ray using 32 bit unsigned integer type
typedef fray<unsigned, 3> uray4;
/// 2d ray using 32 bit signed integer type
typedef fray<int, 2> iray2;
/// 3d ray using 32 bit signed integer type
typedef fray<int, 3> iray3;
/// 4d ray using 32 bit signed integer type
typedef fray<int, 3> iray4;
/// 2d ray using 32 bit single precision floating point type
typedef fray<float, 2> ray2;
/// 3d ray using 32 bit single precision floating point type
typedef fray<float, 3> ray3;
/// 4d ray using 32 bit single precision floating point type
typedef fray<float, 3> ray4;
/// 2d ray using 32 bit double precision floating point type
typedef fray<double, 2> dray2;
/// 3d ray using 32 bit double precision floating point type
typedef fray<double, 3> dray3;
/// 4d ray using 32 bit double precision floating point type
typedef fray<double, 3> dray4;

} // namespace math
} // namespace cgv
