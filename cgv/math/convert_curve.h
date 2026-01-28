#pragma once

#include <array>

#include "bezier.h"
#include "hermite.h"

namespace cgv {
namespace math {

/// @brief Split a cubic hermite curve into two quadratic bezier curve that approximate the original curve.
/// 
/// @tparam T The control point type.
/// @tparam S The scalar type. Must be compatible with the control point type.
/// @param n0 The curve start node.
/// @param n1 The curve end node.
/// @return An array of 5 bezier control points, with the first 3 defining the first segment and the last 3 defining the second segment.
template<typename T, typename S = T>
static std::array<T, 5> split_hermite_to_quadratic_beziers(const hermite_node<T>& n0, const hermite_node<T>& n1) {
	std::array<T, 5> bez;
	bez[0] = n0.val;
	bez[1] = n0.val + n0.tan * S(0.2667);
	auto h = n1.val - n1.tan * S(0.2667);
	bez[2] = (bez[1] + h) * S(0.5);
	bez[3] = h;
	bez[4] = n1.val;
	return bez;
}

/// @brief Convert a cubic hermite curve to a cubic bezier curve.
/// 
/// @tparam T The control point type.
/// @tparam S The scalar type. Must be compatible with the control point type.
/// @param n0 The hermite curve start node.
/// @param n1 The hermite curve end node.
/// @return An array of 4 bezier control points.
template<typename T, typename S = T>
static std::array<T, 4> hermite_to_cubic_bezier(const hermite_node<T>& n0, const hermite_node<T>& n1) {
	return {
		n0.val,
		n0.val + n0.tan / S(3),
		n1.val - n1.tan / S(3),
		n1.val
	};
}

/// @brief Convert a cubic bezier curve to a cubic hermite curve.
/// 
/// @tparam T The control point type.
/// @tparam S The scalar type. Must be compatible with the control point type.
/// @param p0 The first bezier control point.
/// @param p1 The second bezier control point.
/// @param p2 The third bezier control point.
/// @param p3 The fourth bezier control point.
/// @return A pair of hermite start and end nodes.
template<typename T, typename S = T>
static std::pair<hermite_node<T>, hermite_node<T>> cubic_bezier_to_hermite(const T& p0, const T& p1, const T& p2, const T& p3) {
	T t0 = S(3) * (p1 - p0);
	T t1 = S(3) * (p3 - p2);
	return { { p0, t0 }, { p3, t1 } };
}

/// @brief Convert a cubic B-spline curve to a cubic bezier curve.
/// 
/// @tparam T The control point type.
/// @tparam S The scalar type. Must be compatible with the control point type.
/// @param p0 The first B-spline control point.
/// @param p1 The second B-spline control point.
/// @param p2 The third B-spline control point.
/// @param p3 The fourth B-spline control point.
/// @return An array of 4 bezier control points.
template<typename T, typename S = T>
static std::array<T, 4> cubic_basis_to_bezier(const T& b0, const T& b1, const T& b2, const T& b3) {
	constexpr S one_over_three = S(1) / S(3);
	constexpr S one_over_six = S(1) / S(6);
	T p0 = one_over_six * (b0 + S(4) * b1 + b2);
	T p1 = one_over_three * (S(2) * b1 + b2);
	T p2 = one_over_three * (b1 + S(2) * b2);
	T p3 = one_over_six * (b1 + S(4) * b2 + b3);
	return { p0, p1, p2, p3 };
}

} // namespace math
} // namespace cgv
