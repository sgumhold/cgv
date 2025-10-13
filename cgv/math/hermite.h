#pragma once

#include "fvec.h"
#include "fmat.h"
#include "interpolate.h"
#include "parametric_curve.h"

namespace cgv {
namespace math {

template<typename T>
struct hermite_node {
	T val; // the point value (e.g. position)
	T tan; // the tangent
};

template<typename PointT>
struct cubic_hermite_curve : public parametric_curve<cubic_hermite_curve<PointT>> {
	/// the start node
	hermite_node<PointT> n0;
	/// the end node
	hermite_node<PointT> n1;

	cubic_hermite_curve() {}
	cubic_hermite_curve(const PointT& p0, const PointT& m0, const PointT& p1, const PointT& m1) : n0{ p1, m0 }, n1{ p1, m1 } {}
	cubic_hermite_curve(const hermite_node<PointT>& n0, const hermite_node<PointT>& n1) : n0(n0), n1(n1) {}

	template<typename ParamT = float>
	PointT evaluate(ParamT t) const {
		return interpolate_cubic_hermite(n0.val, n0.tan, n1.val, n1.tan, t);
	}
};

} // namespace math
} // namespace cgv
