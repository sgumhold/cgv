#pragma once

#include "fvec.h"
#include "fmat.h"
#include "oriented_box.h"

namespace cgv {
namespace math {

template<typename point_type, typename param_type = float>
class quadratic_bezier {
public:
	using matrix_type = fmat<param_type, 3, 3>;

	static const matrix_type& characteristic_matrix() {
		return M;
	}

	static point_type interpolate(const point_type& p0, const point_type& p1, const point_type& p2, param_type t) {
		fvec<param_type, 3> w = { param_type(1), t, t * t };
		w = w * M;
		return w[0] * p0 + w[1] * p1 + w[2] * p2;
	}

	template<typename OutputIt>
	static void sample(const point_type& p0, const point_type& p1, const point_type& p2, size_t num_segments, OutputIt output_first) {
		num_segments = std::max(num_segments, size_t(1));
		param_type step = param_type(1) / static_cast<param_type>(num_segments);
		for(size_t i = 0; i <= num_segments; ++i) {
			param_type t = step * static_cast<param_type>(i);
			*output_first = interpolate(p0, p1, p2, t);
			++output_first;
		}
	}

	static std::vector<point_type> sample(const point_type& p0, const point_type& p1, const point_type& p2, size_t num_segments) {
		std::vector<point_type> points;
		points.reserve(num_segments);
		sample(p0, p1, p2, num_segments, std::back_inserter(points));
		return points;
	}

private:
	// the characteristic matrix
	inline static const matrix_type M = {
		1, -2, 1,
		0, 2, -2,
		0, 0, 1
	};
};

template<typename point_type>
class quadratic_bezier_curve {
public:
	/// the start point
	point_type p0 = { 0 };
	/// the middle point
	point_type p1 = { 0 };
	/// the end point
	point_type p2 = { 0 };

	quadratic_bezier_curve() {}
	quadratic_bezier_curve(const point_type& p0, const point_type& p1, const point_type& p2) : p0(p0), p1(p1), p2(p2) {}

	template<typename param_type = float>
	point_type interpolate(param_type t) const {
		return quadratic_bezier<point_type, param_type>::interpolate(p0, p1, p2, t);
	}

	template<typename param_type = float>
	std::vector<point_type> sample(size_t num_segments) const {
		return quadratic_bezier<point_type, param_type>::sample(p0, p1, p2, num_segments);
	}

	std::pair<point_type, point_type> axis_aligned_bounding_box() const {
		point_type mi = per_component_min(p0, p2);
		point_type ma = per_component_max(p0, p2);

		if(is_outside_bounds(p1, mi, ma)) {
			point_type t = clamp((p0 - p1) / (p0 - point_type(2) * p1 + p2), point_type(0), point_type(1));
			point_type s = point_type(1) - t;
			point_type q = s * s * p0 + point_type(2) * s * t * p1 + t * t * p2;
			mi = per_component_min(mi, q);
			ma = per_component_max(ma, q);
		}

		return { mi, ma };
	}

private:
	template<typename T>
	T per_component_min(const T& a, const T& b) const {
		return std::min(a, b);
	}

	template<typename T, cgv::type::uint32_type N>
	fvec<T, N> per_component_min(const fvec<T, N>& a, const fvec<T, N>& b) const {
		return min(a, b);
	}

	template<typename T>
	T per_component_max(const T& a, const T& b) const {
		return std::max(a, b);
	}

	template<typename T, cgv::type::uint32_type N>
	fvec<T, N> per_component_max(const fvec<T, N>& a, const fvec<T, N>& b) const {
		return max(a, b);
	}

	template<typename T>
	bool is_outside_bounds(const T& p, const T& min, const T& max) const {
		return p < min || p > max;
	}

	template<typename T, cgv::type::uint32_type N>
	bool is_outside_bounds(const fvec<T, N>& p, const fvec<T, N>& min, const fvec<T, N>& max) const {
		for(cgv::type::uint32_type i = 0; i < N; ++i) {
			if(p[i] < min[i] || p[i] > max[i])
				return true;
		}
		return false;
	}
};

// (Copyright 2013 Inigo Quilez: https://iquilezles.org/articles/bezierbbox/ and https://www.shadertoy.com/view/ldj3Wh)
template<typename T>
static std::pair<T, T> quadratic_bezier_signed_distance(const quadratic_bezier_curve<fvec<T, 3>>& curve, const fvec<T, 3>& pos) {
	using vec_type = fvec<T, 3>;
	vec_type a = curve.p1 - curve.p0;
	vec_type b = curve.p0 - T(2) * curve.p1 + curve.p2;
	vec_type c = a * T(2);
	vec_type d = curve.p0 - pos;

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
