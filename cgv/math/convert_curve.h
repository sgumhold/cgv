#pragma once

#include <array>

#include "bezier.h"
#include "hermite.h"

namespace cgv {
namespace math {

/// @brief Split a cubic hermite segment into two quadratic bezier segments that approximate the original curve.
/// 
/// @tparam T The control point type.
/// @param h0 The segment start node.
/// @param h1 The segment end node.
/// @return An array of 5 bezier control points, with the first 3 defining the first segment and the last 3 defining the second segment.
template<typename T>
static std::array<T, 5> split_hermite_to_quadratic_beziers(const hermite_node<T>& n0, const hermite_node<T>& n1) {
	std::array<T, 5> bez;
	bez[0] = n0.val;
	bez[1] = n0.val + n0.tan * T(0.2667);
	auto h = n1.val - n1.tan * T(0.2667);
	bez[2] = (bez[1] + h) * T(0.5);
	bez[3] = h;
	bez[4] = n1.val;
	return bez;
}

template<typename T>
static std::array<T, 4> hermite_to_cubic_bezier(const hermite_node<T>& n0, const hermite_node<T>& n1) {
	return {
		n0.val,
		n0.val + n0.tan / T(3),
		n1.val - n1.tan / T(3),
		n1.val
	};
}

template<typename T>
static std::pair<hermite_node<T>, hermite_node<T>> cubic_bezier_to_hermite(const T& p0, const T& p1, const T& p2, const T& p3) {
	T t0 = T(3) * (p1 - p0);
	T t1 = T(3) * (p3 - p2);
	return { { p0, t0 }, { p3, t1 } };
}

} // namespace math
} // namespace cgv
