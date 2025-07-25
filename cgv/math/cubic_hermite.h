#pragma once

#include "fvec.h"
#include "fmat.h"

namespace cgv {
namespace math {

template<typename point_type, typename param_type = float>
class cubic_hermite {
public:
	using matrix_type = fmat<param_type, 4, 4>;

	static const matrix_type& characteristic_matrix() {
		return M;
	}

	static point_type interpolate(const point_type& p0, const point_type& m0, const point_type& p1, const point_type& m1, param_type t) {
		fvec<param_type, 4> w = { param_type(1), t, t * t, t * t * t };
		w = w * M;
		return w[0] * p0 + w[1] * m0 + w[2] * p1 + w[3] * m1;
	}

	template<typename OutputIt>
	static void sample(const point_type& p0, const point_type& m0, const point_type& p1, const point_type& m1, size_t num_segments, OutputIt output_first) {
		num_segments = std::max(num_segments, size_t(1));
		param_type step = param_type(1) / static_cast<param_type>(num_segments);
		for(size_t i = 0; i <= num_segments; ++i) {
			param_type t = step * static_cast<param_type>(i);
			*output_first = interpolate(p0, m0, p1, m1, t);
			++output_first;
		}
	}

	static std::vector<point_type> sample(const point_type& p0, const point_type& m0, const point_type& p1, const point_type& m1, size_t num_segments) {
		std::vector<point_type> points;
		points.reserve(num_segments);
		sample(p0, m0, p1, m1, num_segments, std::back_inserter(points));
		return points;
	}

private:
	// the characteristic matrix
	inline static const matrix_type M = {
		1, 0, -3, 2,
		0, 1, -2, 1,
		0, 0, 3, -2,
		0, 0, -1, 1
	};
};

template<typename T>
struct hermite_node {
	T val; // the point value (e.g. position)
	T tan; // the tangent
};

template<typename point_type>
struct cubic_hermite_curve {
	/// the start node
	hermite_node<point_type> n0;
	/// the end node
	hermite_node<point_type> n1;

	cubic_hermite_curve() {}
	cubic_hermite_curve(const point_type& p0, const point_type& m0, const point_type& p1, const point_type& m1) : n0{ p1, m0 }, n1{ p1, m1 } {}
	cubic_hermite_curve(const hermite_node<point_type>& n0, const hermite_node<point_type>& n1) : n0(n0), n1(n1) {}

	template<typename param_type = float>
	point_type interpolate(param_type t) const {
		return cubic_hermite<point_type, param_type>::interpolate(n0.val, n0.tan, n1.val, n1.tan, t);
	}

	template<typename param_type = float>
	std::vector<point_type> sample(size_t num_segments) const {
		return cubic_hermite<point_type, param_type>::sample(n0.val, n0.tan, n1.val, n1.tan, num_segments);
	}
};

} // namespace math
} // namespace cgv
