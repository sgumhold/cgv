#pragma once

#include "fmat.h"
#include "fvec.h"
#include "inv.h"

namespace cgv {
	namespace math {

/**
*	This class defines a template for n-dimensional rays with arbitrary data type defined by origin and direction.
*/
template <typename T, cgv::type::uint32_type N>
class ray {
public:
	fvec<T, N> origin;
	fvec<T, N> direction;

	/// Construct an invalid ray with origin at zero and undefined direction.
	ray() {
		origin = fvec<T, N>(0);
		direction = fvec<T, N>(0);
	}

	/// Construct a ray with given origin and direction.
	ray(const fvec<T, N>& origin, const fvec<T, N>& direction) : origin(origin), direction(direction) {}

	/// Returns the position of the ray at the given distance (ray parameter t) from its origin.
	fvec<T, N> position(float t) const {

		return origin + t * direction;
	}
};

/**
*	Partial specialization of ray class for 3 dimensions.
*/
template <typename T>
class ray<T, 3> {
public:
	fvec<T, 3> origin;
	fvec<T, 3> direction;

	/// Construct an invalid ray with origin at zero and undefined direction.
	ray() {
		origin = fvec<T, 3>(0);
		direction = fvec<T, 3>(0);
	}

	/// Construct a ray with given origin and direction.
	ray(const fvec<T, 3>& origin, const fvec<T, 3>& direction) : origin(origin), direction(direction) {}

	/**
	  Construct a ray from a given screen coordinate and view parameters. This is typically used to create a
	  viewing ray that originates at the eye and goes through a pixel on the screen.
	  \param screen_coord the screen pixel coordinate with (0,0) in the bottom left corner.
	  \param viewport_size the screen size in pixels.
	  \param eye_position the location of the eye/camera.
	  \param view_projection_matrix the current view and projection matrix with no (or identity) model transformation.
	*/
	ray(const fvec<T, 2>& screen_coord, const fvec<unsigned, 2>& viewport_size, const fvec<T, 3>& eye_position, const fmat<T, 4, 4>& view_projection_matrix) {

		fvec<T, 2> window_coord = T(2) * screen_coord / static_cast<fvec<T, 2>>(viewport_size) - T(1);

		fvec<T, 4> world_coord(window_coord.x(), window_coord.y(), T(1), T(1));
		world_coord = inv(view_projection_matrix) * world_coord;
		world_coord /= world_coord.w();

		origin = eye_position;
		direction = normalize(fvec<T, 3>(world_coord) - eye_position);
	}

	/// Returns the position of the ray at the given distance (ray parameter t) from its origin.
	fvec<T, 3> position(float t) const {

		return origin + t * direction;
	}
};

// Definition of some typical ray types
typedef ray<unsigned, 2> uray2;
typedef ray<unsigned, 3> uray3;
typedef ray<unsigned, 3> uray4;
typedef ray<int, 2> iray2;
typedef ray<int, 3> iray3;
typedef ray<int, 3> iray4;
typedef ray<float, 2> ray2;
typedef ray<float, 3> ray3;
typedef ray<float, 3> ray4;
typedef ray<double, 2> dray2;
typedef ray<double, 3> dray3;
typedef ray<double, 3> dray4;

	}
}
