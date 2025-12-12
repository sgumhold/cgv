#pragma once

#include "fvec.h"

/// This header provides common signed and unsigned distance implementations between points and primitives or primitives and primitives.
namespace cgv {
namespace math {

/// @brief Computes the point on the line given by direction and going through p0 which is closest to the query point.
/// 
/// @tparam T the numeric type.
/// @tparam N the dimensionality.
/// @param point the query point.
/// @param p0 the reference point on the line.
/// @param direction the line direction.
/// @return the unsigned distance.
template <typename T, cgv::type::uint32_type N>
T closest_point_on_line_to_point(const fvec<T, N>& point, const fvec<T, N>& p0, const fvec<T, N>& direction) {
	return p0 + (dot(point - p0, direction) / dot(direction, direction)) * direction;
}

/// @brief Computes the unsigned distance between a point and infinite line given by direction going through p0.
/// 
/// @tparam T the numeric type.
/// @tparam N the dimensionality.
/// @param point the query point.
/// @param p0 the reference point on the line.
/// @param direction the line direction.
/// @return the unsigned distance.
template <typename T, cgv::type::uint32_type N>
T point_line_distance(const fvec<T, N>& point, const fvec<T, N>& p0, const fvec<T, N>& direction) {
	fvec<T, N> closest_point = closest_point_on_line_to_point(line, point);
	return length(closest_point - point);
}

/// @brief Computes the signed distance between a point and axis aligned box located at the origin.
/// Negative distances are inside of the box.
/// 
/// @tparam T the numeric type.
/// @param [in] point the query point.
/// @param [in] extent the total box extent/size.
/// @return the signed distance.
template <typename T>
T point_box_distance(const fvec<T, 3>& point, const fvec<T, 3>& extent) {
	fvec<T, 3> q = abs(point) - T(0.5) * extent;
	return length(max(q, T(0))) + std::min(std::max(q.x(), std::max(q.y(), q.z())), T(0));
}

/// @brief Computes the signed distance between a point and infinite plane.??????????????????????????????????????????????????????????
/// Negative distances are opposite to the plane normal direction.
/// 
/// @tparam T the numeric type.
/// @param [in] point the query point.
/// @param [in] origin the plane origin position.
/// @param [in] normal the plane surface normal (must be normalized).
/// @return the signed distance.
template <typename T>
T point_plane_distance(const fvec<T, 3>& point, const fvec<T, 3>& origin, const fvec<T, 3>& normal) {
	return dot(point - origin, normal);
};

/// @brief Computes the signed distance between a point and sphere.
/// Negative distances are inside of the sphere.
/// 
/// @tparam T the numeric type.
/// @tparam N the number of dimensions.
/// @param [in] point the query point.
/// @param [in] center the sphere center position.
/// @param [in] radius the sphere radius.
/// @return the signed distance.
template <typename T, cgv::type::uint32_type N>
T point_sphere_distance(const fvec<T, N>& point, const fvec<T, N>& center, T radius) {
	return length(center - point) - radius;
}

/// @brief Computes the unsigned distance between a point and quadratic bezier curve given by three control points.
/// 
/// @tparam T the numeric type.
/// @param [in] point the query point.
/// @param [in] p0 the first bezier control point.
/// @param [in] p1 the second bezier control point.
/// @param [in] p2 the third bezier control point.
/// @return a pair { unsigned distance, curve parameter at closest point }.
template<typename T>
static std::pair<T, T> point_quadratic_bezier_distance(const fvec<T, 3>& point, const fvec<T, 3>& p0, const fvec<T, 3>& p1, const fvec<T, 3>& p2) {
	// (Copyright 2013 Inigo Quilez: https://iquilezles.org/articles/bezierbbox/ and https://www.shadertoy.com/view/ldj3Wh)
	using vec_type = fvec<T, 3>;
	vec_type a = p1 - p0;
	vec_type b = p0 - T(2) * p1 + p2;
	vec_type c = a * T(2);
	vec_type d = p0 - point;

	T kk = T(1) / dot(b, b);
	T kx = kk * dot(a, b);
	T ky = kk * (T(2) * dot(a, a) + dot(d, b)) / T(3);
	T kz = kk * dot(d, a);

	T dist = T(0);
	T t = T(0);

	T p = ky - kx * kx;
	T p3 = p * p * p;
	T q = kx * (T(2) * kx * kx - T(3) * ky) + kz;
	T h = q * q + T(4) * p3;

	if(h >= T(0)) {
		h = std::sqrt(h);
		fvec<T, 2> x = (vec2(h, -h) - q) / T(2);
		fvec<T, 2> uv = sign(x) * pow(abs(x), vec2(T(1) / T(3)));
		t = clamp(uv.x() + uv.y() - kx, T(0), T(1));

		// 1 root
		dist = sqr_length(d + (c + b * t) * t);
	} else {
		T z = std::sqrt(-p);
		T v = std::acos(q / (p * z * T(2))) / T(3);
		T m = std::cos(v);
		T n = std::sin(v) * T(1.732050808);
		vec_type ts = clamp(vec_type(m + m, -n - m, n - m) * z - kx, T(0), T(1));

		// 3 roots, but only need two
		T dt = sqr_length(d + (c + b * ts.x()) * ts.x());
		dist = dt;
		t = ts.x();

		dt = sqr_length(d + (c + b * ts.y()) * ts.y());
		if(dt < dist) {
			dist = dt;
			t = ts.y();
		}
	}

	dist = sqrt(dist);

	return { dist, t };
}

} // namespace math
} // namespace cgv
