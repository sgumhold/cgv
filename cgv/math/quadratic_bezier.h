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

// TODO: Rename interpolate to evaluate/eval.
// TODO: Check signed distance for tube.

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















template<typename T, cgv::type::uint32_type N>
T quadratic_bezier_arc_length_even_subdivision(const quadratic_bezier_curve<fvec<T, N>>& curve, T t = T(1), size_t num_segments = 100) {
	T step = T(1) / static_cast<T>(num_segments);
	T result = T(0);

	fvec<T, N> prev_point = curve.p0;
	for(size_t i = 0; i < num_segments; ++i) {
		T t_ = static_cast<T>(i + 1) * step;
		if(t_ >= t) {
			fvec<T, N> next_point = curve.interpolate(t);
			result += length(next_point - prev_point);
			break;
		}

		fvec<T, N> next_point = curve.interpolate(t_);
		result += length(next_point - prev_point);
		prev_point = next_point;
	}

	return result;
}

// TODO: change template type name
template <typename FLOAT_TYPE> struct ArcLengthBezierApproximation {
	std::vector<FLOAT_TYPE> y1 = {};
	std::vector<FLOAT_TYPE> y2 = {};
	std::vector<FLOAT_TYPE> lengths = {};
	FLOAT_TYPE totalLength = 0.0;

	FLOAT_TYPE evaluate(FLOAT_TYPE t) const {
		if(t <= 0) {
			return 0;
		}
		if(t >= 1) {
			return totalLength;
		}

		size_t segmentCount = y1.size();
		size_t index = size_t(FLOAT_TYPE(segmentCount) * t);
		auto tStep = 1.0 / static_cast<FLOAT_TYPE>(segmentCount);
		auto tPrev = static_cast<FLOAT_TYPE>(index) * tStep;
		auto tCur = static_cast<FLOAT_TYPE>(index + 1) * tStep;
		auto tDiff = tCur - tPrev;

		auto x = (t - tPrev) / tDiff;
		auto t1 = 3.0 * (1.0 - x) * (1.0 - x) * x * y1[index];
		auto t2 = 3.0 * (1.0 - x) * x * x * y2[index];
		auto t3 = x * x * x;
		auto y = t1 + t2 + t3;

		auto dPrev = lengths[index];
		auto dCur = lengths[index + 1];
		auto dDiff = dCur - dPrev;
		return (FLOAT_TYPE)(y * dDiff + dPrev);
	}
};

template<typename T, cgv::type::uint32_type N>
ArcLengthBezierApproximation<T> quadratic_bezier_arc_length_bezier_approximation(const quadratic_bezier_curve<fvec<T, N>>& curve, size_t num_segments, size_t num_samples = 100) {
	if(num_segments < 1)
		return {};

	auto result = ArcLengthBezierApproximation<T>();
	result.totalLength = quadratic_bezier_arc_length_even_subdivision(curve, T(1), num_samples);// arc_length_legendre_gauss(1.0, numSamples);
	result.lengths.push_back(T(0));

	auto dStep = T(1) / static_cast<T>(num_segments);
	for(size_t i = 0; i < num_segments; i++) {
		auto tPrev = static_cast<T>(i) * dStep;
		auto tCur = static_cast<T>(i + 1) * dStep;
		auto tDiff = tCur - tPrev;

		auto dPrev = result.lengths.back();
		auto dCur = quadratic_bezier_arc_length_even_subdivision(curve, tCur, num_samples);// arc_length_legendre_gauss(tCur, numSamples);
		auto dDiff = dCur - dPrev;

		if(std::abs(dDiff) < std::numeric_limits<T>::epsilon()) {
			result.y1.push_back(T(0));
			result.y2.push_back(T(0));
		} else {
			T sample1 = (tPrev + tDiff * (T(1) / T(3)));
			auto s1over3 = quadratic_bezier_arc_length_even_subdivision(curve, sample1, num_samples); //arc_length_legendre_gauss(sample1);
			auto s1over3Scaled = (s1over3 - dPrev) / dDiff;

			T sample2 = (tPrev + tDiff * (T(2) / T(3)));
			auto s2over3 = quadratic_bezier_arc_length_even_subdivision(curve, sample2, num_samples); //arc_length_legendre_gauss(sample2);
			auto s2over3Scaled = (s2over3 - dPrev) / dDiff;

			auto y1 = (T(18) * s1over3Scaled - T(9) * s2over3Scaled + T(2)) / T(6);
			auto y2 = (T(-9) * s1over3Scaled + T(18) * s2over3Scaled - T(5)) / T(6);

			result.y1.push_back(y1);
			result.y2.push_back(y2);
		}
		result.lengths.push_back(dCur);
	}

	return result;
}

template <typename FLOAT_TYPE> struct Parameterization {
	virtual FLOAT_TYPE evaluate(FLOAT_TYPE d) const = 0;
	virtual FLOAT_TYPE length() const = 0;
};

template <typename FLOAT_TYPE> struct ParameterizationBezierApproximation : public Parameterization<FLOAT_TYPE> {
	std::vector<FLOAT_TYPE> y1 = {};
	std::vector<FLOAT_TYPE> y2 = {};
	std::vector<FLOAT_TYPE> t = {};
	FLOAT_TYPE totalLength = 0.0;

	FLOAT_TYPE evaluate(FLOAT_TYPE d) const override {
		if(d <= 0) {
			return 0;
		}
		if(d >= totalLength) {
			return 1;
		}

		auto dNormalized = d / totalLength;
		size_t segmentCount = y1.size();
		size_t index = size_t(FLOAT_TYPE(segmentCount) * dNormalized);
		auto dStep = 1.0 / static_cast<FLOAT_TYPE>(segmentCount);
		auto dPrev = static_cast<FLOAT_TYPE>(index) * dStep;
		auto dCur = static_cast<FLOAT_TYPE>(index + 1) * dStep;
		auto dDiff = dCur - dPrev;

		auto x = (dNormalized - dPrev) / dDiff;
		auto t1 = 3.0 * (1.0 - x) * (1.0 - x) * x * y1[index];
		auto t2 = 3.0 * (1.0 - x) * x * x * y2[index];
		auto t3 = x * x * x;
		auto y = t1 + t2 + t3;

		auto tPrev = t[index];
		auto tCur = t[index + 1];
		auto tDiff = tCur - tPrev;
		return (FLOAT_TYPE)(y * tDiff + tPrev);
	}
	FLOAT_TYPE length() const override {
		return totalLength;
	}
};

template <typename FLOAT_TYPE>
struct ParameterizationSubdivisionBezierApproximation : public Parameterization<FLOAT_TYPE> {
	ArcLengthBezierApproximation<FLOAT_TYPE> arcLength;
	int depth;
	FLOAT_TYPE evaluate(FLOAT_TYPE d) const override {
		FLOAT_TYPE t0 = 0.0;
		FLOAT_TYPE t1 = 1.0;
		for(int i = 0; i < depth; i++) {
			FLOAT_TYPE t = (t0 + t1) / FLOAT_TYPE(2);
			auto l = arcLength.evaluate(t);
			if(d < l) {
				t1 = t;
			} else if(d > l) {
				t0 = t;
			} else {
				break;
			}
		}
		return (t0 + t1) / FLOAT_TYPE(2);
	}
	FLOAT_TYPE length() const override {
		return arcLength.totalLength;
	}
};

template<typename FLOAT_TYPE>
ParameterizationSubdivisionBezierApproximation<FLOAT_TYPE> parameterization_subdivision_bezier_approximation(const ArcLengthBezierApproximation<FLOAT_TYPE>& arcLength, int depth = 100) {
	ParameterizationSubdivisionBezierApproximation<FLOAT_TYPE> result;
	result.depth = depth;
	result.arcLength = arcLength;
	return result;
}

template<typename FLOAT_TYPE>
ParameterizationBezierApproximation<FLOAT_TYPE> parameterization_bezier_approximation(const ArcLengthBezierApproximation<FLOAT_TYPE>& arcLength, int numSamples = 100) {
	ParameterizationBezierApproximation<FLOAT_TYPE> result;
	result.totalLength = arcLength.totalLength;
	result.t.push_back(0.0);

	auto approx = parameterization_subdivision_bezier_approximation(arcLength, numSamples);

	const int numSegments = (unsigned)arcLength.y1.size();
	auto dStep = FLOAT_TYPE(1) / static_cast<FLOAT_TYPE>(numSegments);
	for(int i = 0; i < numSegments; i++) {
		auto dPrev = static_cast<FLOAT_TYPE>(i) * dStep;
		auto dCur = static_cast<FLOAT_TYPE>(i + 1) * dStep;
		auto dDiff = dCur - dPrev;

		auto tPrev = result.t.back();
		auto tCur = approx.evaluate(dCur * result.totalLength);
		auto tDiff = tCur - tPrev;

		FLOAT_TYPE sample1 = (dPrev + dDiff * (FLOAT_TYPE(1) / FLOAT_TYPE(3))) * result.totalLength;
		auto s1over3 = approx.evaluate(sample1);
		auto s1over3Scaled = (s1over3 - tPrev) / tDiff;

		FLOAT_TYPE sample2 = (dPrev + dDiff * (FLOAT_TYPE(2) / FLOAT_TYPE(3))) * result.totalLength;
		auto s2over3 = approx.evaluate(sample2);
		auto s2over3Scaled = (s2over3 - tPrev) / tDiff;

		auto y1 = (18.0 * s1over3Scaled - 9.0 * s2over3Scaled + 2.0) / 6.0;
		auto y2 = (-9.0 * s1over3Scaled + 18.0 * s2over3Scaled - 5.0) / 6.0;

		result.y1.push_back((FLOAT_TYPE)y1);
		result.y2.push_back((FLOAT_TYPE)y2);
		result.t.push_back((FLOAT_TYPE)tCur);
	}

	return result;
}






} // namespace math
} // namespace cgv
