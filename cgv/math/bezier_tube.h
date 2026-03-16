#pragma once

#include "bezier.h"
#include "distance.h"
#include "oriented_box.h"

namespace cgv {
namespace math {

template<typename T>
struct bezier_tube_node {
	fvec<T, 3> pos = T(0);
	T rad = T(0);
};

template<typename T>
class quadratic_bezier_tube {
public:
	using vec_type = fvec<T, 3>;
	using matrix_type = fmat<T, 4, 4>;
	using node_type = bezier_tube_node<T>;

	// the start node
	node_type n0;
	// the middle node
	node_type n1;
	// the end node
	node_type n2;

	template<typename ParamT = float>
	node_type evaluate(ParamT t) const {
		node_type n;
		n.pos = interpolate_quadratic_bezier(n0.pos, n1.pos, n2.pos, t);
		n.rad = interpolate_quadratic_bezier(n0.rad, n1.rad, n2.rad, t);
		return n;
	}

	template<typename ParamT = float>
	node_type derivative(ParamT t) const {
		node_type n;
		n.pos = interpolate_linear(point_type(2) * (n1.pos - n0.pos), point_type(2) * (n2.pos - n1.pos), t);
		n.rad = interpolate_linear(point_type(2) * (n1.rad - n0.rad), point_type(2) * (n2.rad - n1.rad), t);
		return n;
	}

	template<typename ParamT = float>
	std::vector<node_type> sample(size_t num_segments) const {
		std::vector<node_type> points;
		points.reserve(num_segments + 1);
		sequence_transform<ParamT>(std::back_inserter(points), [this](ParamT t) { return evaluate(t); }, num_segments + 1);
		return points;
	}

	std::pair<vec_type, vec_type> axis_aligned_bounding_box() const {
		quadratic_bezier_curve<vec_type> curve;
		curve.p0 = n0.pos - n0.rad;
		curve.p1 = n1.pos - n1.rad;
		curve.p2 = n2.pos - n2.rad;

		auto box_min = curve.axis_aligned_bounding_box();

		curve.p0 = n0.pos + n0.rad;
		curve.p1 = n1.pos + n1.rad;
		curve.p2 = n2.pos + n2.rad;
		auto box_max = curve.axis_aligned_bounding_box();

		return { min(box_min.first, box_max.first), max(box_min.second, box_max.second) };
	}

	oriented_box3<T> oriented_bounding_box() const {
		const fvec<T, 4> corners[4] = {
			{ T(-0.5), T(-0.5), T(-0.5), T(1.0) },
			{ T(+0.5), T(-0.5), T(-0.5), T(1.0) },
			{ T(-0.5), T(+0.5), T(-0.5), T(1.0) },
			{ T(-0.5), T(-0.5), T(+0.5), T(1.0) }
		};

		cgv::mat4 M = calculate_transformation_matrix();

		vec_type p000 = vec_type(M * corners[0]);
		vec_type p100 = vec_type(M * corners[1]);
		vec_type p010 = vec_type(M * corners[2]);
		vec_type p001 = vec_type(M * corners[3]);

		vec_type dx = p100 - p000;
		vec_type dy = p010 - p000;
		vec_type dz = p001 - p000;

		T lx = length(dx);
		T ly = length(dy);
		T lz = length(dz);

		fmat<T, 3, 3> R({ dx / lx, dy / ly, dz / lz });

		oriented_box3<T> box;
		box.center = M.col(3);
		box.extent = cgv::vec3(lx, ly, lz);
		box.rotation = cgv::quat(R);
		return box;
	}

	std::pair<T, T> signed_distance(const vec_type& pos) const {
		quadratic_bezier_curve<vec_type> curve = { n0.pos, n1.pos, n2.pos };
		std::pair<T, T> res = point_quadratic_bezier_distance(pos, n0.pos, n1.pos, n2.pos);

		T rc[3];
		control_points_to_poly_coeffs(n0.rad, n1.rad, n2.rad, rc);
		T radius = eval_poly_d0(res.second, rc);

		res.first -= radius;
		return res;
	}

	matrix_type calculate_transformation_matrix()  const {
		vec_type x, y, z;

		T xl, yl;
		bool xq = false;
		bool yq = false;
		{
			x = n2.pos - n0.pos;
			xl = length(x);

			if(xl < T(0.0001)) {
				y = n1.pos - n0.pos;
				yl = length(y);

				if(yl < T(0.0001)) {
					x = vec_type(T(1), T(0), T(0));
					y = vec_type(T(0), T(1), T(0));
					z = vec_type(T(0), T(0), T(1));

					xl = T(1); xq = true;
					yl = T(1); yq = true;
				} else {
					x = normalize(ortho(x));
					xl = T(1); xq = true;

					z = cross(x, y);
				}
			} else {
				y = cgv::math::project_to_plane(n1.pos - n0.pos, x);
				yl = length(y);

				if(yl < T(0.0001)) {
					y = normalize(ortho(x));
					yl = T(1); yq = true;
				}

				z = cross(x, y);
			}
		}

		vec_type xd = x / xl;
		vec_type yd = y / yl;
		vec_type zd = normalize(z);

		T xm, xp, ym, yp, zm;
		{
			T xyl = dot(n1.pos - n0.pos, xd);

			T cx[3];
			control_points_to_poly_coeffs(T(0), xyl, xl, cx);

			T cy[3];
			control_points_to_poly_coeffs(T(0), yl, T(0), cy);

			T rc[3];
			control_points_to_poly_coeffs(n0.rad, n1.rad, n2.rad, rc);

			T c_xm[3];
			c_xm[0] = cx[0] - rc[0]; c_xm[1] = cx[1] - rc[1]; c_xm[2] = cx[2] - rc[2];

			T c_xp[3];
			c_xp[0] = cx[0] + rc[0]; c_xp[1] = cx[1] + rc[1]; c_xp[2] = cx[2] + rc[2];

			xm = std::min(-n0.rad, std::min(xl - n2.rad, eval_poly_d0(saturate(-c_xm[1] / c_xm[2] * T(0.5)), c_xm)));
			xp = std::max(+n0.rad, std::max(xl + n2.rad, eval_poly_d0(saturate(-c_xp[1] / c_xp[2] * T(0.5)), c_xp)));

			T c_ym[3];
			c_ym[0] = cy[0] - rc[0]; c_ym[1] = cy[1] - rc[1]; c_ym[2] = cy[2] - rc[2];

			T c_yp[3];
			c_yp[0] = cy[0] + rc[0]; c_yp[1] = cy[1] + rc[1]; c_yp[2] = cy[2] + rc[2];

			ym = std::min(-n0.rad, std::min(-n2.rad, eval_poly_d0(saturate(-c_ym[1] / c_ym[2] * T(0.5)), c_ym)));
			yp = std::max(+n0.rad, std::max(+n2.rad, eval_poly_d0(saturate(-c_yp[1] / c_yp[2] * T(0.5)), c_yp)));

			zm = std::max(n0.rad, std::max(n2.rad, eval_poly_d0(saturate(-rc[1] / rc[2] * T(0.5)), rc)));

			if(xq) { xm = -zm; xp = zm; }
			if(yq) { ym = -zm; yp = zm; }
		}

		vec_type center = n0.pos + 0.5f * (xd * (xm + xp) + yd * (ym + yp));

		return {
			{ (xp - xm) * xd, T(0) },
			{ (yp - ym) * yd, T(0) },
			{ T(2) * zm * zd, T(0) },
			{ center, T(1) }
		};
	}

private:
	void control_points_to_poly_coeffs(T p0, T h, T p1, T o_c[3]) const {
		o_c[0] = p0;
		o_c[1] = T(-2) * p0 + T(2) * h;
		o_c[2] = p0 + p1 - T(2) * h;
	}

	T eval_poly_d0(T x, T c[3])  const {
		return x * (x * c[2] + c[1]) + c[0];
	}
};

} // namespace math
} // namespace cgv
