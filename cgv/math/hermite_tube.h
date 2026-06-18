#pragma once

#include <array>

#include "bezier_tube.h"
#include "convert_curve.h"
#include "hermite.h"

namespace cgv {
namespace math {

template<typename T>
struct hermite_tube_node {
	hermite_node<fvec<T, 3>> pos;
	hermite_node<T> rad;
};

template<typename T>
class cubic_hermite_tube {
public:
	using vec_type = fvec<T, 3>;
	using matrix_type = fmat<T, 4, 4>;
	using node_type = hermite_tube_node<T>;
	
	// the start node
	node_type n0;
	// the end node
	node_type n1;

	template<typename ParamT = float>
	node_type evaluate(ParamT t) const {
		node_type n;
		n.pos = interpolate_cubic_hermite(n0.val, n0.tan, n1.val, n1.tan, t);
		n.rad = interpolate_cubic_hermite(n0.val, n0.tan, n1.val, n1.tan, t);
		return n;
	}

	template<typename ParamT = float>
	std::vector<node_type> sample(size_t num_segments) const {
		std::vector<node_type> points;
		points.reserve(num_segments + 1);
		sequence_transform<ParamT>(std::back_inserter(points), [this](ParamT t) { return evaluate(t); }, num_segments + 1);
		return points;
	}

	std::pair<vec_type, vec_type> approximate_axis_aligned_bounding_box() const {
		std::pair<vec_type, vec_type> box;
		std::array<quadratic_bezier_tube<T>, 2> qtubes = split_to_quadratic_bezier_tubes();

		std::pair<vec_type, vec_type> qbox = qtubes[0].axis_aligned_bounding_box();
		box.first = qbox.first;
		box.second = qbox.second;

		qbox = qtubes[1].axis_aligned_bounding_box();
		box.first = min(box.first, qbox.first);
		box.second = max(box.second, qbox.second);

		return box;
	}

	std::array<quadratic_bezier_tube<T>, 2> split_to_quadratic_bezier_tubes() const {
		std::array<vec_type, 5> ps = split_hermite_to_quadratic_beziers(n0.pos, n1.pos);
		std::array<T, 5> rs = split_hermite_to_quadratic_beziers(n0.rad, n1.rad);

		std::array<quadratic_bezier_tube<T>, 2> qtubes;
		qtubes[0].n0 = { ps[0], rs[0] };
		qtubes[0].n1 = { ps[1], rs[1] };
		qtubes[0].n2 = { ps[2], rs[2] };
		qtubes[1].n0 = { ps[2], rs[2] };
		qtubes[1].n1 = { ps[3], rs[3] };
		qtubes[1].n2 = { ps[4], rs[4] };
		return qtubes;
	}
};


} // namespace math
} // namespace cgv
