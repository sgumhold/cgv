#pragma once

#include "fvec.h"
#include "fmat.h"

namespace cgv {
namespace math {

template<typename point_type, typename param_type = float>
class cubic_bezier {
public:
	using matrix_type = fmat<param_type, 4, 4>;

	static const matrix_type& characteristic_matrix() {
		return M;
	}

	static point_type interpolate(const point_type& p0, const point_type& p1, const point_type& p2, const point_type& p3, param_type t) {
		fvec<param_type, 4> w = { param_type(1), t, t * t, t * t * t };
		w = w * M;
		return w[0] * p0 + w[1] * p1 + w[2] * p2 + w[3] * p3;
	}

	template<typename OutputIt>
	static void sample(const point_type& p0, const point_type& p1, const point_type& p2, const point_type& p3, size_t num_segments, OutputIt output_first) {
		num_segments = std::max(num_segments, size_t(1));
		param_type step = param_type(1) / static_cast<param_type>(num_segments);
		for(size_t i = 0; i <= num_segments; ++i) {
			param_type t = step * static_cast<param_type>(i);
			*output_first = interpolate(p0, p1, p2, p3, t);
			++output_first;
		}
	}

	static std::vector<point_type> sample(const point_type& p0, const point_type& p1, const point_type& p2, const point_type& p3, size_t num_segments) {
		std::vector<point_type> points;
		points.reserve(num_segments);
		sample(p0, p1, p2, p3, num_segments, std::back_inserter(points));
		return points;
	}

private:
	// the characteristic matrix
	inline static const matrix_type M = {
		1, -3, 3, -1,
		0, 3, -6, 3,
		0, 0, 3, -3,
		0, 0, 0, 1
	};
};

template<typename point_type>
class cubic_bezier_curve {
public:
	/// the first point
	point_type p0 = { 0 };
	/// the second point
	point_type p1 = { 0 };
	/// the third point
	point_type p2 = { 0 };
	/// the fourth point
	point_type p3 = { 0 };

	cubic_bezier_curve() {}
	cubic_bezier_curve(const point_type& p0, const point_type& p1, const point_type& p2, const point_type& p3) : p0(p0), p1(p1), p2(p2), p3(p3) {}

	template<typename param_type = float>
	point_type interpolate(param_type t) const {
		return quadratic_bezier<point_type, param_type>::interpolate(p0, p1, p2, p3, t);
	}

	template<typename param_type = float>
	std::vector<point_type> sample(size_t num_segments) const {
		return quadratic_bezier<point_type, param_type>::sample(p0, p1, p2, p3, num_segments);
	}

	std::pair<point_type, point_type> axis_aligned_bounding_box() const {
		return compute_bounds(p0, p1, p2, p3);
	}

private:
	// Altered from original version. (Copyright 2013 Inigo Quilez: https://iquilezles.org/articles/bezierbbox/ and https://www.shadertoy.com/view/MdKBWt)
	template<typename T>
	std::pair<T, T> compute_bounds(const T& p0, const T& p1, const T& p2, const T& p3) const{
		T mi = std::min(p0, p3);
		T ma = std::max(p0, p3);

		T c = T(-1) * p0 + T(1) * p1;
		T b = T(1) * p0 - T(2) * p1 + T(1) * p2;
		T a = T(-1) * p0 + T(3) * p1 - T(3) * p2 + T(1) * p3;

		T h = b * b - a * c;

		if(h > T(0)) {
			h = sqrt(h);
			T t = (-b - h) / a;
			if(t > T(0) && t < T(1)) {
				T s = T(1) - t;
				T q = s * s * s * p0 + T(3) * s * s * t * p1 + T(3) * s * t * t * p2 + t * t * t * p3;
				mi = std::min(mi, q);
				ma = std::max(ma, q);
			}
			t = (-b + h) / a;
			if(t > T(0) && t < T(1)) {
				T s = T(1) - t;
				T q = s * s * s * p0 + T(3) * s * s * t * p1 + T(3) * s * t * t * p2 + t * t * t * p3;
				mi = std::min(mi, q);
				ma = std::max(ma, q);
			}
		}

		return { mi, ma };
	}

	template<typename T, cgv::type::uint32_type N>
	std::pair<fvec<T, N>, fvec<T, N>> compute_bounds(const fvec<T, N>& p0, const fvec<T, N>& p1, const fvec<T, N>& p2, const fvec<T, N>& p3) const {
		std::pair<fvec<T, N>, fvec<T, N>> bounds;
		for(cgv::type::uint32_type i = 0; i < N; ++i) {
			std::pair<T, T> coord_bounds = compute_bounds(p0[i], p1[i], p2[i], p3[i]);
			bounds.first[i] = coord_bounds.first;
			bounds.second[i] = coord_bounds.second;
		}
		return bounds;
	}
};

} // namespace math
} // namespace cgv
