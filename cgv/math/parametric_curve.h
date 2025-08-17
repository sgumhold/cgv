#pragma once

#include "fvec.h"
#include "fmat.h"
#include "interpolate.h"

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
	/// @param t The arc length.
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

/// @brief CRTP base class for parametric curves. Specialization taht allows to expose the curve point type.
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

/// @brief Return the arc length of the given curve up to t.
/// The curve is divided into evenly spaced segments whose straight line distances are summed up.
template<template<class> class CurveT, typename T, cgv::type::uint32_type N>
T arc_length_even_subdivision(const parametric_curve<CurveT<fvec<T, N>>>& curve, T t = T(1), size_t num_segments = 128) {
	T step = T(1) / static_cast<T>(num_segments);
	T result = T(0);

	fvec<T, N> prev_point = curve.evaluate(T(0));
	for(size_t i = 0; i < num_segments; ++i) {
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


} // namespace math
} // namespace cgv
