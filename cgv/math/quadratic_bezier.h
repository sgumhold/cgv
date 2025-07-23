#pragma once

#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>

namespace cgv {
namespace math {

template<typename point_type, typename param_type = float>
class quadratic_bezier_interpolation {
public:
	using matrix_type = cgv::math::fmat<param_type, 3, 3>;

	static const matrix_type& characteristic_matrix() {
		return M;
	}

	static point_type interpolate(const point_type& p0, const point_type& p1, const point_type& p2, param_type t) {
		cgv::math::fvec<param_type, 3> w = { param_type(1), t, t * t };
		w = w * M;
		return w[0] * p0 + w[1] * p1 + w[2] * p2;
	}

	template<typename OutputIt>
	static void sample(const point_type& p0, const point_type& p1, const point_type& p2, size_t num_segments, OutputIt output_first) {
		num_segments = std::max(num_segments, size_t(1));
		param_type step = 1.0f / static_cast<param_type>(num_segments);
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
struct quadratic_bezier_curve {
public:
	/// the start point
	point_type p0 = { 0 };
	/// the middle point
	point_type p1 = { 0 };
	/// the end point
	point_type p2 = { 0 };

	template<typename param_type = float>
	point_type interpolate(param_type t) const {
		return quadratic_bezier_interpolation<point_type, param_type>::interpolate(p0, p1, p2, t);
	}

	template<typename param_type = float>
	std::vector<point_type> sample(size_t num_segments) const {
		return quadratic_bezier_interpolation<point_type, param_type>::sample(p0, p1, p2, num_segments);
	}
};

} // namespace math
} // namespace cgv
