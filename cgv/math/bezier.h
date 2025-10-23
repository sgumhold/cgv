#pragma once

#include "fvec.h"
#include "fmat.h"
#include "interpolate.h"
#include "parametric_curve.h"

namespace cgv {
namespace math {

template<typename PointT>
class bezier_curve : public parametric_curve<bezier_curve<PointT>> {
public:
	/// the control points
	std::vector<PointT> points;

	bezier_curve() {}
	bezier_curve(std::initializer_list<PointT> points) : points(points) {}

	template<typename ParamT = float>
	PointT evaluate(ParamT t) const {
		// TODO: throw an exception
		if(points.empty())
			return {};

		if(points.size() == 1)
			return points.front();

		size_t degree = points.size() - 1;
		std::vector<PointT> d = points;

		for(size_t i = 1; i <= degree; ++i) {
			for(size_t j = degree; j > i - 1; --j)
				d[j] = interpolate_linear(d[j - 1], d[j], t);
		}

		return d[degree];
	}
};

template<typename PointT>
class quadratic_bezier_curve : public parametric_curve<quadratic_bezier_curve<PointT>> {
public:
	/// the start control point
	PointT p0 = { 0 };
	/// the middle control point
	PointT p1 = { 0 };
	/// the end control point
	PointT p2 = { 0 };

	quadratic_bezier_curve() {}
	quadratic_bezier_curve(const PointT& p0, const PointT& p1, const PointT& p2) : p0(p0), p1(p1), p2(p2) {}

	template<typename ParamT = float>
	PointT evaluate(ParamT t) const {
		return interpolate_quadratic_bezier(p0, p1, p2, t);
	}

	template<typename ParamT = float>
	PointT derivative(ParamT t) const {
		return interpolate_linear(PointT(2) * (p1 - p0), PointT(2) * (p2 - p1), t);
	}

	std::pair<PointT, PointT> axis_aligned_bounding_box() const {
		PointT mi = per_component_min(p0, p2);
		PointT ma = per_component_max(p0, p2);

		if(is_outside_bounds(p1, mi, ma)) {
			PointT t = clamp((p0 - p1) / (p0 - PointT(2) * p1 + p2), PointT(0), PointT(1));
			PointT s = PointT(1) - t;
			PointT q = s * s * p0 + PointT(2) * s * t * p1 + t * t * p2;
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

template<typename PointT>
class cubic_bezier_curve : public parametric_curve<cubic_bezier_curve<PointT>> {
public:
	/// the first control point
	PointT p0 = { 0 };
	/// the second control point
	PointT p1 = { 0 };
	/// the third control point
	PointT p2 = { 0 };
	/// the fourth control point
	PointT p3 = { 0 };

	cubic_bezier_curve() {}
	cubic_bezier_curve(const PointT& p0, const PointT& p1, const PointT& p2, const PointT& p3) : p0(p0), p1(p1), p2(p2), p3(p3) {}

	template<typename ParamT = float>
	PointT evaluate(ParamT t) const {
		return interpolate_cubic_bezier(p0, p1, p2, p3, t);
	}

	template<typename ParamT = float>
	PointT derivative(ParamT t) const {
		return interpolate_quadratic_bezier(PointT(3) * (p1 - p0), PointT(3) * (p2 - p1), PointT(3) * (p3 - p2), t);
	}

	std::pair<PointT, PointT> axis_aligned_bounding_box() const {
		return compute_bounds(p0, p1, p2, p3);
	}
private:
	// Altered from original version. (Copyright 2013 Inigo Quilez: https://iquilezles.org/articles/bezierbbox/ and https://www.shadertoy.com/view/MdKBWt)
	template<typename T>
	std::pair<T, T> compute_bounds(const T& p0, const T& p1, const T& p2, const T& p3) const {
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
