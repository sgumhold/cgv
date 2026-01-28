#pragma once

#include "interval.h"

namespace cgv {
namespace math {

namespace detail {

/// @brief Return the interpolated value of the piecewise function defined by uniformly-spaced points at position t using the interpolation operation.
/// 
/// Points are assumed to be spaced equidistant over the parameter domain [0,1]. The function finds the piece of the function that contains
/// the parameter position t. The piece is defined by two points (a, b), with a being the point with the largest position smaller than t and
/// b being the point with the smallest position larger than t. If t is smaller than the first point's position, a is equal to b and defined
/// to be the first point. If t is larger than the last point's position, a is equal to b and defined to be the last point.
/// The interpolation operation is then invoked on (a, b) with the locally transformed interpolation parameter t'
/// in [0,1].
/// 
/// @tparam PointT The point type of the piecewise function.
/// @tparam InterpolationOp The interpolation operation type.
/// @tparam ParamT The parameter type.
/// @param points The breakpoints defining the piecewise function.
/// @param t The parameter position at which to evaluate the piecewise interpolation.
/// @param operation The operation applied to the piece belonging to t. Must be of form (PointT, PointT, ParamT) -> PointT.
/// @return The interpolated point at position t.
template<typename PointT, typename InterpolationOp, typename ParamT = float>
PointT interpolate_piece(const std::vector<PointT>& points, ParamT t, InterpolationOp operation) {
	if(t <= ParamT(0))
		return points.front();

	if(t >= ParamT(1))
		return points.back();

	size_t num_pairs = points.size() - 1;
	size_t index = static_cast<size_t>(t * static_cast<ParamT>(num_pairs));
	ParamT step = ParamT(1) / static_cast<ParamT>(num_pairs);
	ParamT prev = static_cast<ParamT>(index) * step;
	ParamT curr = static_cast<ParamT>(index + 1) * step;
	t = (t - prev) / (curr - prev);
	return operation(points[index], points[index + 1], t);
}

/// @brief Return the interpolated value of the piecewise function defined by points at position t using the interpolation operation.
/// 
/// Points are defined by their position and value and must be sorted in ascending order according to their position.
/// If the sequence of input points is not ordered, the result of the function is undefined.
/// The parameter domain is given by [points.front().first, points.last().first]. The function finds the piece of the function that contains
/// the parameter position t. The piece is defined by two points (a, b), with a being the point with the largest position smaller than t and
/// b being the point with the smallest position larger than t. If t is smaller than the first point's position, a is equal to b and defined
/// to be the first point. If t is larger than the last point's position, a is equal to b and defined to be the last point.
/// The interpolation operation is then invoked on (a, b) with the locally transformed interpolation parameter t'
/// in [0,1].
/// 
/// @tparam PointT The point type of the piecewise function.
/// @tparam InterpolationOp The interpolation operation type.
/// @tparam ParamT The parameter type.
/// @param points The breakpoints defining the piecewise function. Must be sorted ascending according to std::pair<ParamT, PointT>::first.
/// @param t The parameter position at which to evaluate the piecewise interpolation.
/// @param operation The operation applied to the piece belonging to t. Must be of form (PointT, PointT, ParamT) -> PointT.
/// @return The interpolated point at position t.
template<typename PointT, typename InterpolationOp, typename ParamT = float>
PointT interpolate_piece(const std::vector<std::pair<ParamT, PointT>>& points, ParamT t, InterpolationOp operation) {
	using pair_type = std::pair<ParamT, PointT>;
	
	if(t <= 0)
		return points.front().second;

	auto it = std::lower_bound(points.begin(), points.end(), t, [](const pair_type& point, float value) { return point.first < value; });
	if(it == points.end())
		return points.back().second;

	// "it" points to the first point whose parameter position is larger than t
	const pair_type& left = *(it == points.begin() ? it : it - 1);
	const pair_type& right = *it;
	t = (t - left.first) / (right.first - left.first);
	return operation(left.second, right.second, t);
}

/// @brief Return a sequence of n interpolated values of the piecewise function defined by points at position t using the interpolation operation.
/// 
/// Points are defined by their position and value and must be sorted in ascending order according to their position.
/// If the sequence of input points is not ordered, the result of the function is undefined.
/// The generated samples are spaced equidistant over the parameter domain given by [points.front().first, points.last().first].
/// The interpolation operation is invoked for every sample with the current piece defined by enclosing points (a, b) and the locally
/// transformed interpolation parameter t' in [0,1].
/// 
/// @tparam PointT The point type of the piecewise function.
/// @tparam InterpolationOp The interpolation operation type.
/// @tparam ParamT The parameter type.
/// @param points The breakpoints defining the piecewise function. Must be sorted ascending according to std::pair<ParamT, PointT>::first.
/// @param operation The operation applied to the piece belonging to t. Must be of form (PointT, PointT, ParamT) -> PointT.
/// @param n The number of samples.
/// @return The seqence of interpolated points.
template<typename PointT, typename InterpolationOp, typename ParamT = float>
std::vector<PointT> interpolate_n(const std::vector<std::pair<ParamT, PointT>>& points, InterpolationOp operation, size_t n) {
	using pair_type = std::pair<ParamT, PointT>;

	if(n == 0 || points.size() == 0)
		return {};

	if(points.size() == 1)
		return std::vector<PointT>(n, points.front().second);

	size_t l = 0;
	size_t r = 1;

	if(points.front().first > ParamT(0))
		r = 0;

	std::vector<PointT> res;
	res.reserve(n);

	ParamT step = ParamT(1) / static_cast<ParamT>(n - 1);

	for(size_t i = 0; i < n; ++i) {
		ParamT x = static_cast<ParamT>(i) * step;

		while(x > points[r].first && l < points.size() - 1) {
			l = r;
			r = std::min(l + 1, points.size() - 1);
		}

		if(l == r) {
			res.push_back(points[r].second);
		} else {
			const pair_type& p0 = points[l];
			const pair_type& p1 = points[r];
			ParamT t = (x - p0.first) / (p1.first - p0.first);
			res.push_back(operation(p0.second, p1.second, t));
		}
	}
	return res;
}

} // namespace detail

/// @brief Apply operation to a generated sequence of n uniformly-spaced values in [0,1] and store the result in an output range starting from output_first.
/// 
/// @tparam ParamT The sequence value type.
/// @tparam OutputIt The output range iterator type.
/// @tparam UnaryOp Unary operation type.
/// @param output_first The start of the output range.
/// @param operation The operation to transform the sequence. Takes one argument of type ParamT.
/// @param n The number of values in the generated sequence.
template<typename ParamT = float, typename OutputIt, typename UnaryOp>
static void sequence_transform(OutputIt output_first, UnaryOp operation, size_t n) {
	if(n == 1) {
		*output_first = operation(ParamT(0.5));
		++output_first;
	} else if(n > 1) {
		ParamT step = ParamT(1) / static_cast<ParamT>(n - 1);
		for(size_t i = 0; i < n; ++i) {
			ParamT t = step * static_cast<ParamT>(i);
			*output_first = operation(t);
			++output_first;
		}
	}
}

/// @brief Nearest-neighbor interpolation of two points.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param p0 The first point.
/// @param p1 The second point.
/// @param t The interpolation parameter in [0,1].
/// @return The interpolated point.
template<typename PointT, typename ParamT = float>
PointT interpolate_nearest(const PointT& p0, const PointT& p1, ParamT t) {
	return t < ParamT(0.5) ? p0 : p1;
};

/// @brief Return the result of a piecewise nearest-neighbor interpolation on a seqence of uniformly-spaced control points at position t.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param points The control points of the piecewise function.
/// @param t The interpolation parameter in domain.
/// @param domain The domain of the control points, defining the valid range of t.
/// @return The interpolated point.
template<typename PointT, typename ParamT = float>
PointT interpolate_nearest(const std::vector<PointT>& points, ParamT t, const interval<ParamT>& domain = { 0.0f, 1.0f }) {
	return detail::interpolate_piece(points, t, static_cast<PointT(*)(const PointT&, const PointT&, ParamT)>(&interpolate_nearest<PointT, ParamT>));
};

/// @brief Return the result of a piecewise nearest-neighbor interpolation on a seqence of control points at position t.
/// 
/// Points are defined by their position and value and must be sorted in ascending order according to their position.
/// If the sequence of input points is not ordered, the result of the function is undefined.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param points The control points of the piecewise function.
/// @param t The interpolation parameter in domain.
/// @param domain The domain of the control points, defining the valid range of t.
/// @return The interpolated point.
template<typename PointT, typename ParamT = float>
PointT interpolate_nearest(const std::vector<std::pair<ParamT, PointT>>& points, ParamT t) {
	return detail::interpolate_piece(points, t, static_cast<PointT(*)(const PointT&, const PointT&, ParamT)>(&interpolate_nearest<PointT, ParamT>));
}

/// @brief Return a sequence of n interpolated values of a piecewise nearest-neighbor interpolation on a seqence of uniformly-spaced control points.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param points The control points of the piecewise function.
/// @param n The number of samples.
/// @return The seqence of interpolated points.
template<typename PointT, typename ParamT = float>
std::vector<PointT> interpolate_nearest_n(const std::vector<PointT>& points, size_t n) {
	std::vector<PointT> res;
	res.reserve(n);
	sequence_transform(std::back_inserter(res), [&points](float t) { return cgv::math::interpolate_nearest(points, t); }, n);
	return res;
};

/// @brief Return a sequence of n interpolated values of a piecewise nearest-neighbor interpolation on a seqence of control points.
/// 
/// Points are defined by their position and value and must be sorted in ascending order according to their position.
/// If the sequence of input points is not ordered, the result of the function is undefined.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param points The control points of the piecewise function.
/// @param n The number of samples.
/// @return The seqence of interpolated points.
template<typename PointT, typename ParamT = float>
std::vector<PointT> interpolate_nearest_n(const std::vector<std::pair<ParamT, PointT>>& points, size_t n) {
	return detail::interpolate_n(points, static_cast<PointT(*)(const PointT&, const PointT&, ParamT)>(&interpolate_nearest<PointT, ParamT>), n);
}

/// @brief Linear interpolation of two points.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param p0 The first point.
/// @param p1 The second point.
/// @param t The interpolation parameter in [0,1].
/// @return The interpolated point.
template<typename PointT, typename ParamT = float>
static PointT interpolate_linear(const PointT& p0, const PointT& p1, ParamT t) {
	PointT v0 = (ParamT(1) - t) * p0;
	PointT v1 = t * p1;
	return v0 + v1;
}

/// @brief Return the result of a piecewise linear interpolation on a seqence of uniformly-spaced control points at position t.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param points The control points of the piecewise function.
/// @param t The interpolation parameter in domain.
/// @param domain The domain of the control points, defining the valid range of t.
/// @return The interpolated point.
template<typename PointT, typename ParamT = float>
PointT interpolate_linear(const std::vector<PointT>& points, ParamT t, const interval<ParamT>& domain = { 0.0f, 1.0f }) {
	t = (t - domain.lower_bound) / domain.size();
	return detail::interpolate_piece(points, t, static_cast<PointT(*)(const PointT&, const PointT&, ParamT)>(&interpolate_linear<PointT, ParamT>));
};

/// @brief Return the result of a piecewise linear interpolation on a seqence of control points at position t.
/// 
/// Points are defined by their position and value and must be sorted in ascending order according to their position.
/// If the sequence of input points is not ordered, the result of the function is undefined.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param points The control points of the piecewise function.
/// @param t The interpolation parameter in domain.
/// @param domain The domain of the control points, defining the valid range of t.
/// @return The interpolated point.
template<typename PointT, typename ParamT = float>
PointT interpolate_linear(const std::vector<std::pair<ParamT, PointT>>& points, ParamT t) {
	return detail::interpolate_piece(points, t, static_cast<PointT(*)(const PointT&, const PointT&, ParamT)>(&interpolate_linear<PointT, ParamT>));
}

/// @brief Return a sequence of n interpolated values of a piecewise linear interpolation on a seqence of uniformly-spaced control points.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param points The control points of the piecewise function.
/// @param n The number of samples.
/// @return The seqence of interpolated points.
template<typename PointT, typename ParamT = float>
std::vector<PointT> interpolate_linear_n(const std::vector<PointT>& points, size_t n) {
	std::vector<PointT> res;
	res.reserve(n);
	sequence_transform(std::back_inserter(res), [&points](float t) { return cgv::math::interpolate_linear(points, t); }, n);
	return res;
};

/// @brief Return a sequence of n interpolated values of a piecewise linear interpolation on a seqence of control points.
/// 
/// Points are defined by their position and value and must be sorted in ascending order according to their position.
/// If the sequence of input points is not ordered, the result of the function is undefined.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param points The control points of the piecewise function.
/// @param n The number of samples.
/// @return The seqence of interpolated points.
template<typename PointT, typename ParamT = float>
std::vector<PointT> interpolate_linear_n(const std::vector<std::pair<ParamT, PointT>>& points, size_t n) {
	return detail::interpolate_n(points, static_cast<PointT(*)(const PointT&, const PointT&, ParamT)>(&interpolate_linear<PointT, ParamT>), n);
}

/// @brief Interpolation of a quadratic bezier curve defined by three control points.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param p0 The first point.
/// @param p1 The second point.
/// @param p2 The third point.
/// @param t The interpolation parameter in [0,1].
/// @return The interpolated point.
template<typename PointT, typename ParamT = float>
static PointT interpolate_quadratic_bezier(const PointT& p0, const PointT& p1, const PointT& p2, ParamT t) {
	ParamT mt = ParamT(1) - t;
	PointT v0 = mt * mt * p0;
	PointT v1 = ParamT(2) * mt * t * p1;
	PointT v2 = t * t * p2;
	return v0 + v1 + v2;
}

/// @brief Interpolation of a cubic bezier curve defined by four control points.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param p0 The first point.
/// @param p1 The second point.
/// @param p2 The third point.
/// @param p3 The fourth point.
/// @param t The interpolation parameter in [0,1].
/// @return The interpolated point.
template<typename PointT, typename ParamT = float>
static PointT interpolate_cubic_bezier(const PointT& p0, const PointT& p1, const PointT& p2, const PointT& p3, ParamT t) {
	ParamT mt = ParamT(1) - t;
	PointT v0 = mt * mt * mt * p0;
	PointT v1 = ParamT(3) * mt * mt * t * p1;
	PointT v2 = ParamT(3) * mt * t * t * p2;
	PointT v3 = t * t * t * p3;
	return v0 + v1 + v2 + v3;
}

/// @brief Interpolation of a cubic hermite curve defined by control points and tangents.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param p0 The start point.
/// @param m0 The start tangent.
/// @param p1 The end point.
/// @param m1 The end tangent.
/// @param t The interpolation parameter in [0,1].
/// @return The interpolated point.
template<typename PointT, typename ParamT = float>
static PointT interpolate_cubic_hermite(const PointT& p0, const PointT& m0, const PointT& p1, const PointT& m1, ParamT t) {
	ParamT t2 = t * t;
	ParamT t3 = t * t * t;
	PointT w0 = ParamT(2) * t3 - ParamT(3) * t2 + ParamT(1);
	PointT w1 = t3 - ParamT(2) * t2 + t;
	PointT w2 = ParamT(-2) * t3 + ParamT(3) * t2;
	PointT w3 = t3 - t2;
	return w0 * p0 + w1 * m0 + w2 * p1 + w3 * m1;
}

/// @brief Interpolation of a cubic b-spline curve segment defined by four control points in basis representation.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param p0 The first point.
/// @param p1 The second point.
/// @param p2 The third point.
/// @param p3 The fourth point.
/// @param t The interpolation parameter in [0,1].
/// @return The interpolated point.
template<typename PointT, typename ParamT = float>
static PointT interpolate_cubic_basis(const PointT& b0, const PointT& b1, const PointT& b2, const PointT& b3, ParamT t) {
	ParamT t2 = t * t;
	ParamT t3 = t * t * t;
	PointT v0 = (ParamT(1) - ParamT(3) * t + ParamT(3) * t2 - t3) * b0;
	PointT v1 = (ParamT(4) - ParamT(6) * t2 + ParamT(3) * t3) * b1;
	PointT v2 = (ParamT(1) + ParamT(3) * t + ParamT(3) * t2 - ParamT(3) * t3) * b2;
	PointT v3 = t3 * b3;
	return (ParamT(1) / ParamT(6)) * (v0 + v1 + v2 + v3);
}

/// @brief Return the result of a piecewise cubic interpolation on a seqence of uniformly-spaced control points at position t.
/// 
/// The function assumes points to define a cubic b-spline and uses interpolate_cubic_basis to interpolate a tuple of four
/// consecutive points defining a segment around t. Internally, an extra point is extrapolated at the start and end of the input
/// sequence to guarantee perfect interpoaltion of the first and last point in the input sequence.
/// 
/// @tparam PointT The point type.
/// @tparam ParamT The parameter type.
/// @param points The control points of the piecewise function.
/// @param t The interpolation parameter in domain.
/// @param domain The domain of the control points, defining the valid range of t.
/// @return The interpolated point.
template<typename PointT, typename ParamT = float>
PointT interpolate_smooth_cubic(const std::vector<PointT>& points, ParamT t, const interval<ParamT>& domain = { 0.0f, 1.0f }) {
	t = (t - domain.lower_bound) / domain.size();

	size_t num_pairs = points.size() - 1;

	size_t i = 0;
	if(t <= ParamT(0)) {
		t = ParamT(0);
	} else if(t >= ParamT(1)) {
		t = ParamT(1);
		i = num_pairs - 1;
	} else {
		i = static_cast<size_t>(std::floor(t * static_cast<float>(num_pairs)));
	}

	PointT v1 = points[i];
	PointT v2 = points[i + 1];
	PointT v0 = i > 0 ? points[i - 1] : PointT(2) * v1 - v2;
	PointT v3 = i < num_pairs - 1 ? points[i + 2] : PointT(2) * v2 - v1;
	t = (t - static_cast<float>(i) / static_cast<float>(num_pairs)) * static_cast<float>(num_pairs);
	return interpolate_cubic_basis(v0, v1, v2, v3, t);
};

/// Template class representing a piecewise linear function with uniformly spaced breakpoints that maps from X to Y.
template<typename X, typename Y>
struct uniform_piecewise_linear_function {
	/// The input parameter domain.
	interval<X> domain = { X(0), X(1) };
	/// The points that define the piecewise intervals and are assumed to be spaced uniformly over the domain.
	std::vector<Y> breakpoints;
	
	/// Return the piecewise linear interpolated value at position x.
	Y evaluate(X x) const {
		x = (x - domain.lower_bound) / domain.size();

		if(x <= X(0))
			return breakpoints.front();

		if(x >= X(1))
			return breakpoints.back();

		size_t num_pieces = breakpoints.size() - 1;
		size_t index = static_cast<size_t>(x * static_cast<X>(num_pieces));

		X step = X(1) / static_cast<X>(num_pieces);
		X prev = static_cast<X>(index) * step;
		X curr = static_cast<X>(index + 1) * step;
		X t = (x - prev) / (curr - prev);
		return interpolate_linear(breakpoints[index], breakpoints[index + 1], t);
	}
};

} // namespace math
} // namespace cgv
