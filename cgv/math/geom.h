#pragma once

#include "fmat.h"

namespace cgv {
	namespace math {
		//! rotate vector v around axis n by angle a (given in radian)
		/*! the cos and sin functions need to be implemented for type T.*/
		template <typename T>
		cgv::math::fvec<T, 3> rotate(const cgv::math::fvec<T, 3>& v, const cgv::math::fvec<T, 3>& n, T a)
		{
			cgv::math::fvec<T, 3> vn = dot(n, v)*n;
			return vn + cos(a)*(v - vn) + sin(a)*cross(n, v);
		}
		//! compute a rotation axis and a rotation angle in radian that rotates v0 onto v1.
		/*! An alternative solution is given by the negated axis with negated angle. */
		template <typename T>
		void compute_rotation_axis_and_angle_from_vector_pair(const cgv::math::fvec<T, 3>& v0, const cgv::math::fvec<T, 3>& v1, cgv::math::fvec<T, 3>& axis, T& angle)
		{
			axis = cross(v0, v1);
			T len = axis.length();
			if (len > 2 * std::numeric_limits<T>::epsilon())
				axis *= T(1) / len;
			else
				axis[1] = T(1);
			angle = atan2(len, dot(v0, v1));
		}
		//! decompose a rotation matrix into axis angle representation
		/*! The implementation assumes that R is orthonormal and that det(R) = 1, thus no reflections are handled.
		    Negation of axis and angle yield another solution.
		    The function returns three possible status values:
			- 0 ... axis and angle where unique up to joined negation
			- 1 ... angle is M_PI can be negated independently of axis yielding another solution
			- 2 ... angle is 0 and axis can be choosen arbitrarily.
			*/
		template <typename T>
		int decompose_rotation_to_axis_and_angle(const cgv::math::fmat<T, 3, 3>& R, cgv::math::fvec<T, 3>& axis, T& angle)
		{
			axis(0) = R(2, 1) - R(1, 2);
			axis(1) = R(0, 2) - R(2, 0);
			axis(2) = R(1, 0) - R(0, 1);
			T len = axis.length();
			T tra = R.trace();
			if (len < 2 * std::numeric_limits<T>::epsilon()) {
				if (tra < 0) {
					for (unsigned c = 0; c<3; ++c)
						if (R(c, c) > 0) {
							axis(c) = T(1);
							angle = M_PI;
							break;
						}
					return 1;
				}
				else {
					axis(1) = T(1);
					angle = 0;
					return 2;
				}
			}
			else {
				axis *= T(1) / len;
				angle = atan2(len, tra - T(1));
				return 0;
			}
		}
		//! Given two vectors v0 and v1 extend to orthonormal frame and return 3x3 matrix containing frame vectors in the columns.
		/*! The implementation has the following assumptions that are not checked:
		    - v0.length() > 0 
			- v1.length() > 0 
			- v0 and v1 are not parallel or anti-parallel
			If the result matrix has columns x,y, and z, 
			- x will point in direction of v0 
			- z will point orthogonal to x and v1
			- y will point orthogonal to x and z as good as possible in direction of v1. If v0 and v1 are orthogonal, y is in direction of v1. */
		template <typename T>
		cgv::math::fmat<T, 3, 3> build_orthogonal_frame(const cgv::math::fvec<T, 3>& v0, const cgv::math::fvec<T, 3>& v1)
		{
			cgv::math::fmat<T, 3, 3> O;

			cgv::math::fvec<T, 3>& x = (cgv::math::fvec<T, 3>&)O(0, 0);
			cgv::math::fvec<T, 3>& y = (cgv::math::fvec<T, 3>&)O(0, 1);
			cgv::math::fvec<T, 3>& z = (cgv::math::fvec<T, 3>&)O(0, 2);

			x = v0;
			x.normalize();
			z = cross(v0, v1);
			z.normalize();
			y = cross(z, x);

			return O;
		}

	}
}