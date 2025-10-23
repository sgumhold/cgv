#pragma once

namespace cgv {
namespace math {

template<typename PointT, typename ParamT = float>
static PointT interpolate_linear(const PointT& p0, const PointT& p1, ParamT t) {
	PointT v0 = (ParamT(1) - t) * p0;
	PointT v1 = t * p1;
	return v0 + v1;
}

template<typename PointT, typename ParamT = float>
static PointT interpolate_quadratic_bezier(const PointT& p0, const PointT& p1, const PointT& p2, ParamT t) {
	ParamT mt = ParamT(1) - t;
	PointT v0 = mt * mt * p0;
	PointT v1 = ParamT(2) * mt * t * p1;
	PointT v2 = t * t * p2;
	return v0 + v1 + v2;
}

template<typename PointT, typename ParamT = float>
static PointT interpolate_cubic_bezier(const PointT& p0, const PointT& p1, const PointT& p2, const PointT& p3, ParamT t) {
	ParamT mt = ParamT(1) - t;
	PointT v0 = mt * mt * mt * p0;
	PointT v1 = ParamT(3) * mt * mt * t * p1;
	PointT v2 = ParamT(3) * mt * t * t * p2;
	PointT v3 = t * t * t * p3;
	return v0 + v1 + v2 + v3;
}

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

template<typename ParamT = float, typename OutputIt, typename UnaryOp>
static void sample_steps_transform(OutputIt output_first, UnaryOp operation, size_t num_segments) {
	num_segments = std::max(num_segments, size_t(1));
	ParamT step = ParamT(1) / static_cast<ParamT>(num_segments);
	for(size_t i = 0; i <= num_segments; ++i) {
		ParamT t = step * static_cast<ParamT>(i);
		*output_first = operation(t);
		++output_first;
	}
}

} // namespace math
} // namespace cgv
