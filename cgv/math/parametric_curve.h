#pragma once

#include "fvec.h"
#include "fmat.h"
#include "interpolate.h"
#include "piecewise_linear_function.h"

namespace cgv {
namespace math {

/// @brief CRTP base class that provides arc length information for a parametric_curve.
/// @tparam DerivedT The implemented class type.
/// @tparam ParamT The parameter type.
template<typename DerivedT, typename ParamT>
struct curve_arc_length {
public:
	/// @brief Evaluate the arc length of the curve.
	/// @param t The curve parameter.
	/// @return The arc length at curve parameter t.
	ParamT evaluate(ParamT t) const {
		return static_cast<DerivedT const&>(*this).evaluate(t);
	}

	/// @brief Return the arc length of the complete curve for t from 0 to 1.
	ParamT total_length() const {
		return static_cast<DerivedT const&>(*this).total_length();
	}
};

/// @brief CRTP base class that provides arc length parameterization information for a parametric_curve.
/// @tparam DerivedT The implemented class type.
/// @tparam ParamT The parameter type.
template<typename DerivedT, typename ParamT>
struct curve_parameterization {
public:
	/// @brief Evaluate the arc length parameterization of the curve.
	/// @param d The arc length.
	/// @return The curve parameter where the curve has an arc length of d.
	ParamT evaluate(ParamT d) const {
		return static_cast<DerivedT const&>(*this).evaluate(d);
	}

	/// @brief Return the arc length of the complete curve for t from 0 to 1.
	ParamT total_length() const {
		return static_cast<DerivedT const&>(*this).total_length();
	}
};

/// @brief CRTP base class for parametric curves. Incomplete type that takes varying template types and allows for specialization.
/// @tparam ...Ts
template<typename... Ts>
class parametric_curve;

/// @brief CRTP base class for parametric curves. Specialization that allows to expose the curve point type.
/// @tparam DerivedT The implemented class type.
/// @tparam PointT The implemented curve class point type.
template<template<typename> typename DerivedT, typename PointT>
class parametric_curve<DerivedT<PointT>> {
public:
	using point_type = PointT;

	/// @brief Evalaute the curve.
	/// @tparam ParamT The curve parameter type. Typically float or double.
	/// @param t The curve parameter.
	/// @return The point of the curve at parameter t.
	template<typename ParamT = float>
	PointT evaluate(ParamT t) const {
		return static_cast<DerivedT<PointT> const&>(*this).evaluate(t);
	}

	/// @brief Sample (aka. flatten) the curve into consecutive points forming a polyline.
	/// 
	/// The curve is sampled at regular intervals of the parameter t.
	/// 
	/// @tparam ParamT The curve parameter type. Typically float or double.
	/// @param num_segments The number of segments of the resultng polyline.
	/// @return A vector of num_sample points.
	template<typename ParamT = float>
	std::vector<PointT> sample(size_t num_segments) const {
		std::vector<PointT> points;
		points.reserve(num_segments + 1);
		sample_steps_transform<ParamT>(std::back_inserter(points), [this](ParamT t) { return evaluate(t); }, num_segments);
		return points;
	}

	/// @brief Sample (aka. flatten) the curve into consecutive points forming a polyline while using the given arc length parameterization.
	/// 
	/// The arc length is sampled at regular intervals and transformed to the curve parameter t using the given parameterization.
	/// 
	/// @tparam ParameterizationT The type of the used parameterization.
	/// @tparam ParamT The curve parameter type. Typically float or double.
	/// @param num_segments The number of segments of the resultng polyline.
	/// @param parameterization 
	/// @return A vector of num_sample points.
	template<template<typename> typename ParameterizationT, typename ParamT = float>
	std::vector<PointT> sample(size_t num_segments, const curve_parameterization<ParameterizationT<ParamT>, ParamT>& parameterization) const {
		std::vector<PointT> points;
		points.reserve(num_segments + 1);
		sample_steps_transform<ParamT>(std::back_inserter(points), [this, &parameterization](ParamT t) {
			ParamT d = t * parameterization.total_length();
			return evaluate(parameterization.evaluate(d));
		}, num_segments);
		return points;
	}
};

/// @brief Return the approximate arc length of the given curve up to t.
/// The curve is divided into evenly spaced segments whose straight line distances are summed up.
/// Useful for retrieving the arc length once but inefficient for many successive evaluations.
template<template<class> class CurveT, typename T, cgv::type::uint32_type N>
T arc_length(const parametric_curve<CurveT<fvec<T, N>>>& curve, T t = T(1), int num_segments = 128) {
	num_segments = std::max(num_segments, 1);
	T step = T(1) / static_cast<T>(num_segments);
	T result = T(0);

	fvec<T, N> prev_point = curve.evaluate(T(0));
	for(int i = 0; i < num_segments; ++i) {
		;
		T t_ = static_cast<T>(i + 1) * step;
		if(t_ >= t) {
			fvec<T, N> next_point = curve.evaluate(t);
			result += length(next_point - prev_point);
			break;
		}

		fvec<T, N> next_point = curve.evaluate(t_);
		result += length(next_point - prev_point);
		prev_point = next_point;
	}

	return result;
}

/// @brief Provide arc length information of a parametric curve using a piecewise linear approximation.
/// Faster than arc_length_bezier_approximation with typically higher memory consumption.
template<typename T>
class arc_length_linear_approximation : public curve_arc_length<arc_length_linear_approximation<T>, T> {
public:
	template<template<class> class CurveT, cgv::type::uint32_type N>
	arc_length_linear_approximation(const parametric_curve<CurveT<fvec<T, N>>>& curve, int num_samples = 128) {
		num_samples = std::max(num_samples, 2);
		_lengths.values.reserve(num_samples);
		_lengths.values.push_back(T(0));

		T step = T(1) / static_cast<T>(num_samples - 1);

		fvec<T, N> prev_point = curve.evaluate(T(0));
		for(int i = 1; i < num_samples; ++i) {
			T t_ = static_cast<T>(i) * step;
			fvec<T, N> next_point = curve.evaluate(t_);
			_lengths.values.push_back(_lengths.values.back() + length(next_point - prev_point));
			prev_point = next_point;
		}

		_lengths.domain = { T(0), T(1) };
	}

	T evaluate(T t) const {
		return _lengths.evaluate(t);
	}

	T total_length() const {
		return _lengths.values.back();
	}

	const std::vector<T>& lengths() const {
		return _lengths.values;
	}

private:
	regular_piecewise_linear_function<T> _lengths;
};

/// @brief Provide arc length information of a parametric curve using an approximation via multiple cubic bezier segments.
/// Slower than arc_length_linear_approximation with typically lower memory consumption.
template<typename T>
class arc_length_bezier_approximation : public curve_arc_length<arc_length_bezier_approximation<T>, T> {
public:
	template<template<class> class CurveT, cgv::type::uint32_type N>
	arc_length_bezier_approximation(const parametric_curve<CurveT<fvec<T, N>>>& curve, int num_segments = 4, int num_segment_subdivisions = 8) {
		num_segments = std::max(num_segments, 1);
		num_segment_subdivisions = std::max(num_segment_subdivisions, 1);
		int num_samples = 3 * num_segments * num_segment_subdivisions;

		std::vector<T> arc_lengths;
		T t_step = T(1) / static_cast<T>(num_samples);
		int next_save_index = num_segment_subdivisions - 1;

		fvec<T, N> prev_point = curve.evaluate(T(0));
		for(int i = 0; i < num_samples; ++i) {
			T t = static_cast<T>(i + 1) * t_step;
			fvec<T, N> next_point = curve.evaluate(t);
			_total_length += length(next_point - prev_point);

			if(i == next_save_index) {
				arc_lengths.push_back(_total_length);
				next_save_index += num_segment_subdivisions;
			}

			prev_point = next_point;
		}

		_lengths.push_back(T(0));

		for(int i = 0; i < num_segments; ++i) {
			auto d_prev = _lengths.back();
			auto d_curr = arc_lengths[3 * i + 2];
			auto d_diff = d_curr - d_prev;

			if(std::abs(d_diff) < std::numeric_limits<T>::epsilon()) {
				_y1.push_back(T(0));
				_y2.push_back(T(0));
			} else {
				auto s1_over_3 = arc_lengths[3 * i];
				auto s1_over_3_scaled = (s1_over_3 - d_prev) / d_diff;

				auto s2_over_3 = arc_lengths[3 * i + 1];
				auto s2_over_3_scaled = (s2_over_3 - d_prev) / d_diff;

				auto y1 = (T(18) * s1_over_3_scaled - T(9) * s2_over_3_scaled + T(2)) / T(6);
				auto y2 = (T(-9) * s1_over_3_scaled + T(18) * s2_over_3_scaled - T(5)) / T(6);

				_y1.push_back(y1);
				_y2.push_back(y2);
			}
			_lengths.push_back(d_curr);
		}
	}

	T evaluate(T t) const {
		if(t <= T(0))
			return T(0);

		if(t >= T(1))
			return _total_length;

		T num_segments = static_cast<T>(_y1.size());
		size_t index = static_cast<size_t>(t * num_segments);
		auto t_step = T(1) / num_segments;
		auto t_prev = static_cast<T>(index) * t_step;
		auto t_curr = static_cast<T>(index + 1) * t_step;
		auto t_diff = t_curr - t_prev;

		auto x = (t - t_prev) / t_diff;
		auto y = interpolate_cubic_bezier(T(0), _y1[index], _y2[index], T(1), x);

		auto d_prev = _lengths[index];
		auto d_curr = _lengths[index + 1];
		auto d_diff = d_curr - d_prev;

		return static_cast<T>(y * d_diff + d_prev);
	}

	T total_length() const {
		return _total_length;
	}

private:
	std::vector<T> _y1;
	std::vector<T> _y2;
	std::vector<T> _lengths;
	T _total_length = T(0);
};

/// @brief Provide arc length parameterization of a parametric curve using binary search on a piecewise linear approximation of the arc length.
/// Slower than fast_linear_approximation but very accurate.
template<typename T>
class arc_length_parameterization_linear_approximation : public curve_parameterization<arc_length_parameterization_linear_approximation<T>, T> {
public:
	arc_length_parameterization_linear_approximation(const arc_length_linear_approximation<T>& arc_length) : _lengths(arc_length.lengths()) {}

	T evaluate(T d) const {
		if(d <= 0)
			return T(0);

		auto it = std::lower_bound(_lengths.begin(), _lengths.end(), d);
		if(it == _lengths.end())
			return T(1);

		// "it" points to the first length that is larger than d
		T d_prev = *(it - 1);
		T d_curr = *it;
		T x = (d - d_prev) / (d_curr - d_prev);

		T num_segments = static_cast<T>(_lengths.size() - 1);
		size_t index = std::distance(_lengths.begin(), it);
		T t_step = T(1) / num_segments;
		T t_prev = static_cast<T>(index - 1) * t_step;
		T t_curr = static_cast<T>(index) * t_step;
		return interpolate_linear(t_prev, t_curr, x);
	}

	T total_length() const {
		return _lengths.back();
	}

private:
	std::vector<T> _lengths;
};

/// @brief Provide arc length parameterization of a parametric curve using a piecewise linear approximation of the parameterization computed from arc length information.
/// Faster than linear_approximation but potentially less accurate.
template<typename T>
class arc_length_parameterization_fast_linear_approximation : public curve_parameterization<arc_length_parameterization_fast_linear_approximation<T>, T> {
public:
	arc_length_parameterization_fast_linear_approximation(const arc_length_linear_approximation<T>& arc_length, int num_samples = 128) {
		num_samples = std::max(num_samples, 2);
		_ts.values.reserve(num_samples);

		int num_segments = num_samples - 1;

		arc_length_parameterization_linear_approximation<T> param(arc_length);
		sample_steps_transform(std::back_inserter(_ts.values), [&param](T x) {
			T d = param.total_length() * x;
			return param.evaluate(d);
		}, num_segments);

		_ts.domain = { T(0), param.total_length() };
	}

	T evaluate(T d) const {
		return _ts.evaluate(d);
	}

	T total_length() const {
		return _ts.domain.upper_bound;
	}

private:
	regular_piecewise_linear_function<T> _ts;
};

/// @brief Provide arc length parameterization of a parametric curve using binary search on a bezier approximation of the arc length.
/// Potentially less accurate and slower than linear approximations but with lower memory consumption.
template<typename T>
struct arc_length_parameterization_bezier_approximation : public curve_parameterization<arc_length_parameterization_bezier_approximation<T>, T> {
public:
	arc_length_parameterization_bezier_approximation(const arc_length_bezier_approximation<T>& arc_length, int depth = 8) : _arc_length(arc_length), _depth(std::max(depth, 1)) {}

	T evaluate(T d) const {
		T t0 = T(0);
		T t1 = T(1);
		for(int i = 0; i < _depth; ++i) {
			T t = (t0 + t1) / T(2);
			auto l = _arc_length.evaluate(t);
			if(d < l)
				t1 = t;
			else if(d > l)
				t0 = t;
			else
				break;
		}

		T l0 = _arc_length.evaluate(t0);
		T l1 = _arc_length.evaluate(t1);
		T x = (d - l0) / (l1 - l0);

		return interpolate_linear(t0, t1, x);
	}

	T total_length() const {
		return _arc_length.total_length();
	}

private:
	arc_length_bezier_approximation<T> _arc_length;
	int _depth;
};

} // namespace math
} // namespace cgv
