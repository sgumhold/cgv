#pragma once

#include "functions.h"
#include "fvec.h"
#include "pose.h"
#include "ray.h"
#include <limits>

/// This header provides implementation for common ray-primitive intersection routines.
namespace cgv {
namespace math {

/// @brief Computes the intersection between a ray and axis aligned box located at the origin and returns the number of intersections.
/// Differentiates between 0 or 2 intersections.
/// 
/// @tparam T the numeric type.
/// @param [in] ray the incomming ray.
/// @param [in] extent the total box extent/size.
/// @param [out] out_ts the distances to the intersection points.
/// @param [out] out_normal optional surface normal at the first intersection point.
/// @return the number of intersections.
template <typename T>
int ray_box_intersection(const ray<T, 3>& ray, fvec<T, 3> extent, fvec<T, 2>& out_ts, fvec<T, 3>* out_normal = nullptr) {

	fvec<T, 3> m = fvec<T, 3>(T(1)) / ray.direction; // could be precomputed if traversing a set of aligned boxes
	fvec<T, 3> n = m * ray.origin;   // could be precomputed if traversing a set of aligned boxes
	fvec<T, 3> k = abs(m) * extent;
	fvec<T, 3> t1 = -n - k;
	fvec<T, 3> t2 = -n + k;
	T t_near = std::max(std::max(t1.x(), t1.y()), t1.z());
	T t_far = std::min(std::min(t2.x(), t2.y()), t2.z());

	if(t_near > t_far || t_far < T(0))
		return 0;

	out_ts[0] = t_near;
	out_ts[1] = t_far;

	if(out_normal)
		*out_normal = -sign(ray.direction)
		* step(fvec<T, 3>(t1.y(), t1.z(), t1.x()), fvec<T, 3>(t1.x(), t1.y(), t1.z()))
		* step(fvec<T, 3>(t1.z(), t1.x(), t1.y()), fvec<T, 3>(t1.x(), t1.y(), t1.z()));

	return 2;
}

/// @brief Computes the intersection between a ray and axis aligned box and returns the number of intersections.
/// Differentiates between 0 or 2 intersections.
/// 
/// @tparam T the numeric type.
/// @param [in] ray the incomming ray.
/// @param [in] min the minimum box point.
/// @param [in] max the maximum box point.
/// @param [out] out_ts the distances to the intersection points.
/// @return the number of intersections.
template <typename T>
int ray_box_intersection(const ray<T, 3> &ray, const fvec<T, 3> &min, const fvec<T, 3> &max, fvec<T, 2>& out_ts) {

	fvec<T, 3> t0 = (min - ray.origin) / ray.direction;
	fvec<T, 3> t1 = (max - ray.origin) / ray.direction;

	if(t0.x() > t1.x())
		std::swap(t0.x(), t1.x());

	if(t0.y() > t1.y())
		std::swap(t0.y(), t1.y());

	if(t0.z() > t1.z())
		std::swap(t0.z(), t1.z());

	if(t0.x() > t1.y() || t0.y() > t1.x() ||
		t0.x() > t1.z() || t0.z() > t1.x() ||
		t0.z() > t1.y() || t0.y() > t1.z())
		return 0;

	T t_near = std::max(std::max(t0.x(), t0.y()), t0.z());
	T t_far = std::min(std::min(t1.x(), t1.y()), t1.z());

	if(t_near > t_far)
		std::swap(t_near, t_far);

	out_ts[0] = t_near;
	out_ts[1] = t_far;

	return 2;
}

/// @brief Computes the intersection between a ray and oriented cylinder defined by base center and axis and returns the number of intersections.
/// Differentiates between 0 or 1 intersections.
/// 
/// @tparam T the numeric type.
/// @param [in] ray the incomming ray.
/// @param [in] position the cylinder center position.
/// @param [in] axis the cylinder main axis and length.
/// @param [in] radius the cylinder base radius.
/// @param [out] out_t the distance to the intersection point.
/// @param [out] out_normal optional surface normal at the intersection point.
/// @return the number of intersections.
template <typename T>
int ray_cylinder_intersection(const ray<T, 3>& ray, const fvec<T, 3>& position, const fvec<T, 3>& axis, T radius, T& out_t, fvec<T, 3>* out_normal = nullptr) {

	fvec<T, 3> oc = ray.origin - position;
	T caca = dot(axis, axis);
	T card = dot(axis, ray.direction);
	T caoc = dot(axis, oc);
	T a = caca - card * card;
	T b = caca * dot(oc, ray.direction) - caoc * card;
	T c = caca * dot(oc, oc) - caoc * caoc - radius * radius * caca;
	T h = b * b - a * c;

	if(h < T(0))
		return 0;

	h = std::sqrt(h);
	out_t = (-b - h) / a;

	// body
	T y = caoc + out_t * card;
	if(y > T(0) && y < caca) {
		if(out_normal)
			*out_normal = (oc + out_t * ray.direction - axis * y / caca) / radius;
		return 1;
	}

	// caps
	out_t = ((y < T(0) ? T(0) : caca) - caoc) / card;
	if(std::abs(b + a * out_t) < h) {
		if(out_normal)
			*out_normal = axis * sign(y) / caca;
		return 1;
	}

	return 0;
}

/// @brief Computes the intersection between a ray and oriented cylinder defined by start and end position and returns the number of intersections.
/// Differentiates between 0 or 1 intersections.
/// @param [in] ray the incomming ray.
/// @param [in] start_position the cylinder base center position.
/// @param [in] end_position the cylinder top center position.
/// @param [in] radius the cylinder radius.
/// @param [out] out_t the distance to the intersection point.
/// @param [out] out_normal optional surface normal at the intersection point.
/// @return the number of intersections.
template <typename T>
int ray_cylinder_intersection2(const ray<T, 3>& ray, const fvec<T, 3>& start_position, const fvec<T, 3>& end_position, T radius, T& out_t, fvec<T, 3>* out_normal = nullptr) {

	return ray_cylinder_intersection(ray, start_position, end_position - start_position, radius, out_t, out_normal);
}

/// @brief Computes the intersection between a ray and infinite plane and returns the number of intersections.
/// Differentiates between 0 or 1 intersections.
/// @param [in] ray the incomming ray.
/// @param [in] origin the plane origin position.
/// @param [in] normal the plane surface normal.
/// @param [out] out_t the distance to the intersection point.
/// @return the number of intersections.
template <typename T>
int ray_plane_intersection(const ray<T, 3>& ray, const fvec<T, 3>& origin, const fvec<T, 3>& normal, T& out_t) {

	float denom = dot(normal, ray.direction);
	if(std::abs(denom) < std::numeric_limits<T>::epsilon())
		return 0;

	out_t = dot(origin - ray.origin, normal) / denom;
	return 1;
};

/// @brief Computes the intersection between a ray and sphere and returns the number of intersections.
/// Differentiates between 0, 1 or 2 intersections.
/// @param [in] ray the incomming ray.
/// @param [in] center the sphere center position.
/// @param [in] radius the sphere radius.
/// @param [out] out_ts the distances to the intersection points.
/// @return the number of intersections.
template <typename T>
int ray_sphere_intersection(const ray<T, 3>& ray, const fvec<T, 3>& center, T radius, fvec<T, 2>& out_ts) {

	fvec<T, 3> d = ray.origin - center;
	T il = T(1) / dot(ray.direction, ray.direction);
	T b = il * dot(d, ray.direction);
	T c = il * (dot(d, d) - radius * radius);
	T D = b * b - c;

	if(D < T(0))
		return 0;

	if(D < std::numeric_limits<T>::epsilon()) {
		out_ts = -b;
		return 1;
	}

	D = std::sqrt(D);
	out_ts[0] = -b - D;
	out_ts[1] = -b + D;

	return 2;
}

/// @brief Computes the first intersection between a ray and sphere and returns the number of intersections.
/// Differentiates between 0 or 1 intersections.
/// @param [in] ray the incomming ray.
/// @param [in] center the sphere center position.
/// @param [in] radius the sphere radius.
/// @param [out] out_ts the distances to the intersection points.
/// @param [out] out_normal optional surface normal at the intersection point.
/// @return The number of intersections.
template <typename T>
int first_ray_sphere_intersection(const ray<T, 3>& ray, const fvec<T, 3>& center, T radius, T& out_t, fvec<T, 3>* out_normal = nullptr) {

	fvec<T, 2> ts;
	int k = ray_sphere_intersection(ray, center, radius, ts);

	if(k == 1 || (k == 2 && ts[0] > T(0)))
		out_t = ts[0];
	else if(k == 2 && ts[1] > T(0))
		out_t = ts[1];
	else
		return 0;

	if(out_normal)
		*out_normal = normalize(ray.position(out_t) - center);

	return 1;
}

/// @brief Computes the intersection between a ray and axis aligned torus with medial axis equal to the y-axis and returns the number of intersections.
/// Differentiates between 0 or 1 intersections.
/// @param [in] ray the incomming ray.
/// @param [in] large_radius the torus ring radius (R).
/// @param [in] small_radius the radial torus tube radius (r).
/// @param [out] out_t the distance to the intersection point.
/// @param [out] out_normal optional surface normal at the intersection point.
/// @return the number of intersections.
template <typename T>
int ray_torus_intersection(const ray<T, 3>& ray, T large_radius, T small_radius, T& out_t, fvec<T, 3>* out_normal = nullptr) {

	T po = T(1);
	T Ra2 = large_radius * large_radius;
	T ra2 = small_radius * small_radius;
	T m = dot(ray.origin, ray.origin);
	T n = dot(ray.origin, ray.direction);
	T k = (m + Ra2 - ra2) / T(2);
	T k3 = n;
	const fvec<T, 2>& ro_xy = reinterpret_cast<const fvec<T, 2>&>(ray.origin);
	const fvec<T, 2>& rd_xy = reinterpret_cast<const fvec<T, 2>&>(ray.direction);
	T k2 = n * n - Ra2 * dot(rd_xy, rd_xy) + k;
	T k1 = n * k - Ra2 * dot(rd_xy, ro_xy);
	T k0 = k * k - Ra2 * dot(ro_xy, ro_xy);

	if(std::abs(k3 * (k3 * k3 - k2) + k1) < T(0.01)) {
		po = T(-1);
		T tmp = k1; k1 = k3; k3 = tmp;
		k0 = T(1) / k0;
		k1 = k1 * k0;
		k2 = k2 * k0;
		k3 = k3 * k0;
	}

	T c2 = k2 * T(2) - T(3) * k3 * k3;
	T c1 = k3 * (k3 * k3 - k2) + k1;
	T c0 = k3 * (k3 * (c2 + T(2) * k2) - T(8) * k1) + T(4) * k0;
	c2 /= T(3);
	c1 *= T(2);
	c0 /= T(3);
	T Q = c2 * c2 + c0;
	T R = c2 * c2 * c2 - T(3) * c2 * c0 + c1 * c1;
	T h = R * R - Q * Q * Q;

	if(h >= T(0)) {
		h = std::sqrt(h);
		T v = sign(R + h) * std::pow(std::abs(R + h), T(1) / T(3)); // cube root
		T u = sign(R - h) * std::pow(std::abs(R - h), T(1) / T(3)); // cube root
		fvec<T, 2> s = fvec<T, 2>((v + u) + T(4) * c2, (v - u) * std::sqrt(T(3)));
		T y = std::sqrt(T(0.5) * (length(s) + s.x()));
		T x = T(0.5) * s.y() / y;
		T r = T(2) * c1 / (x * x + y * y);
		T t1 = x - r - k3; t1 = (po < T(0)) ? T(2) / t1 : t1;
		T t2 = -x - r - k3; t2 = (po < T(0)) ? T(2) / t2 : t2;

		if(t1 > T(0)) out_t = t1;
		if(t2 > T(0)) out_t = std::min(out_t, t2);

		if(out_normal) {
			fvec<T, 3> pos = ray.position(out_t);
			*out_normal = normalize(pos * ((dot(pos, pos) - ra2) * fvec<T, 3>(T(1)) - Ra2 * fvec<T, 3>(T(1), T(1), T(-1))));
		}

		return 1; // 2
	}

	T sQ = std::sqrt(Q);
	T w = sQ * cos(acos(-R / (sQ * Q)) / T(3));
	T d2 = -(w + c2);

	if (d2 < T(0))
		return 0;

	T d1 = std::sqrt(d2);
	T h1 = std::sqrt(w - T(2) * c2 + c1 / d1);
	T h2 = std::sqrt(w - T(2) * c2 - c1 / d1);
	T t1 = -d1 - h1 - k3; t1 = (po < T(0)) ? T(2) / t1 : t1;
	T t2 = -d1 + h1 - k3; t2 = (po < T(0)) ? T(2) / t2 : t2;
	T t3 = d1 - h2 - k3;  t3 = (po < T(0)) ? T(2) / t3 : t3;
	T t4 = d1 + h2 - k3;  t4 = (po < T(0)) ? T(2) / t4 : t4;
			
	if (t1 > T(0)) out_t = t1;
	if (t2 > T(0)) out_t = std::min(out_t, t2);
	if (t3 > T(0)) out_t = std::min(out_t, t3);
	if (t4 > T(0)) out_t = std::min(out_t, t4);

	if(out_normal) {
		fvec<T, 3> pos = ray.position(out_t);
		*out_normal = normalize(pos * ((dot(pos, pos) - ra2) * fvec<T, 3>(T(1)) - Ra2 * fvec<T, 3>(T(1), T(1), T(-1))));
	}

	return 1; // 4
}

/// @brief Computes the intersection between a ray and oriented torus defined by origin and medial axis and returns the number of intersections.
/// Differentiates between 0 or 1 intersections.
/// @param [in] ray the incomming ray.
/// @param [in] center the torus center position.
/// @param [in] normal the torus medial axis direction.
/// @param [in] large_radius the torus ring radius (R).
/// @param [in] small_radius the radial torus tube radius (r).
/// @param [out] out_t the distance to the intersection point.
/// @param [out] out_normal optional surface normal at the intersection point.
/// @return the number of intersections.
template <typename T>
int ray_torus_intersection(const ray<T, 3>& ray, const fvec<T, 3>& center, const fvec<T, 3>& normal, T large_radius, T small_radius, T& out_t, fvec<T, 3>* out_normal = nullptr) {

	// compute pose transformation
	fmat<T, 3, 4> pose;
	cgv::math::pose_position(pose) = center;
	fvec<T, 3>& x = reinterpret_cast<fvec<T, 3>&>(pose[0]);
	fvec<T, 3>& y = reinterpret_cast<fvec<T, 3>&>(pose[3]);
	fvec<T, 3>& z = reinterpret_cast<fvec<T, 3>&>(pose[6]);
	z = normal;
	x = normal;
	int i = std::abs(normal[0]) < std::abs(normal[1]) ? 0 : 1;
	i = std::abs(normal[i]) < std::abs(normal[2]) ? i : 2;
	x[i] = T(1);
	y = normalize(cross(normal, x));
	x = cross(y, normal);

	cgv::math::ray<T, 3> transformed_ray;
	transformed_ray.origin = cgv::math::inverse_pose_transform_point(pose, ray.origin);
	transformed_ray.direction = cgv::math::inverse_pose_transform_vector(pose, ray.direction);

	// transform ray into torus pose
	int res = ray_torus_intersection(transformed_ray, large_radius, small_radius, out_t, out_normal);

	// in case of intersection, transform normal back to world space
	if(res)
		*out_normal = cgv::math::pose_transform_vector(pose, *out_normal);

	return res;
}

} // namespace math
} // namespace cgv
