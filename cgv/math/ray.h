#pragma once

#include "vec.h"
#include "inv.h"

namespace cgv {
namespace math {

/// Struct template for varying n-dimensional rays with arbitrary data type defined by origin and direction.
template<class T>
struct ray {
	vec<T> origin;
	vec<T> direction;

	/// Construct an invalid ray with origin at zero and undefined direction.
	ray(unsigned int n = 3) {
		origin.resize(n);
		origin.fill(T(0));
		direction.resize(n);
		direction.fill(T(0));
	}

	/// Construct a ray with given origin and direction. The given orign and direction must have the same dimension.
	ray(const vec<T>& origin, const vec<T>& direction) : origin(origin), direction(direction) {
		assert(origin.dim() == direction.dim());
	}

	/// Return the position of the ray at the given distance (ray parameter t) from its origin.
	vec<T> position(T t) const {
		return origin + t * direction;
	}
};

/// variable n-dimensional ray using single precision floating point type
typedef ray<float> rayn;
/// variable n-dimensional ray using double precision floating point type
typedef ray<double> drayn;

} // namespace math
} // namespace cgv
