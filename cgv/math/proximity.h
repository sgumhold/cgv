#pragma once

#include "fvec.h"
#include <random>

#include "lib_begin.h"

namespace cgv {
	namespace math {
		/// <summary>
		/// find point on sphere closest to reference point 
		/// </summary>
		/// <typeparam name="T">coordinate type</typeparam>
		/// <param name="so">sphere center</param>
		/// <param name="sr">sphere radius</param>
		/// <param name="p">reference point</param>
		/// <param name="q">closest point</param>
		/// <param name="n">normal at closest point</param>
		template <typename T>
		void closest_point_on_sphere_to_point(const fvec<T, 3>& so, T sr, const fvec<T, 3>& p, fvec<T, 3>& q, fvec<T, 3>& n)
		{
			n = normalize(p - so);
			q = sr * n + so;
		}
		/// <summary>
		/// find point on box closest to reference point. If reference point is 
		/// in the interior, return reference point
		/// </summary>
		/// <typeparam name="T">coordinate type</typeparam>
		/// <param name="bo">box center</param>
		/// <param name="be">box extent</param>
		/// <param name="p">reference point</param>
		/// <param name="q">closest point</param>
		/// <param name="n">normal at closest point if this is on the box surfaces, otherwise the 0 vector</param>
		template <typename T>
		void closest_point_on_box_to_point(const fvec<T, 3>& bo, const fvec<T, 3>& be, const fvec<T, 3>& p, fvec<T, 3>& q, fvec<T, 3>& n)
		{
			int j = 0;
			for (int i = 0; i < 3; ++i) {
				q[i] = p[i] - bo[i];
				if (q[i] < 0) {
					n[i] = -1;
					if (q[i] < -0.5f * be[i]) {
						q[i] = -0.5f * be[i];
						++j;
					}
					else
						n[i] = 0;
				}
				else {
					n[i] = 1;
					if (q[i] > 0.5f * be[i]) {
						q[i] = 0.5f * be[i];
						++j;
					}
					else
						n[i] = 0;
				}
				q[i] += bo[i];
			}
			if (j > 1)
				n *= sqrt(T(2));
		}
		/// <summary>
		/// find point on box closest to reference point. If reference point is 
		/// in the interior, return reference point
		/// </summary>
		/// <typeparam name="T">coordinate type</typeparam>
		/// <param name="bo">box center</param>
		/// <param name="be">box extent</param>
		/// <param name="bq">box rotation as quaternion</param>
		/// <param name="p">reference point</param>
		/// <param name="q">closest point</param>
		/// <param name="n">normal at closest point if this is on the box surfaces, otherwise the 0 vector</param>
		template <typename T>
		void closest_point_on_box_to_point(const fvec<T, 3>& bo, const fvec<T, 3>& be, const quaternion<T>& bq, const fvec<T, 3>& p, fvec<T, 3>& q, fvec<T, 3>& n)
		{
			fvec<T, 3> p_loc = p-bo;
			bq.inverse_rotate(p_loc);
			int j = 0;
			for (int i = 0; i < 3; ++i) {
				q[i] = p[i];
				if (q[i] < 0) {
					n[i] = -1;
					if (q[i] < -0.5f * be[i]) {
						q[i] = -0.5f * be[i];
						++j;
					}
					else
						n[i] = 0;
				}
				else {
					n[i] = 1;
					if (q[i] > 0.5f * be[i]) {
						q[i] = 0.5f * be[i];
						++j;
					}
					else
						n[i] = 0;
				}
			}
			if (j > 1)
				n *= sqrt(T(2));
			bq.rotate(n);
			bq.rotate(q);
			q += bo;
		}
		/// <summary>
		/// find point on cylinder closest to reference point
		/// </summary>
		/// <typeparam name="T">coordinate type</typeparam>
		/// <param name="co">cylinder base center</param>
		/// <param name="cd">vector pointing from cylinder base center to cylinder top center</param>
		/// <param name="cr">cylinder radius</param>
		/// <param name="p">reference point</param>
		/// <param name="q">point on cylinder closest to reference point</param>
		/// <param name="n">cylinder normal at reference point</param>
		template <typename T>
		void closest_point_on_cylinder_to_point(const fvec<T, 3>& co, const fvec<T, 3>& cd, T cr, const fvec<T, 3>& p, fvec<T, 3>& q, fvec<T, 3>& n)
		{
			T l = dot(cd, cd);
			T a = dot(p - co, cd) / l;
			l = std::sqrt(l);
			fvec<T, 3> q0 = co + a * cd;
			T r0 = (p - q0).length();
			if (a < 0) {
				if (r0 > cr - a * l)
					n = (1 / r0) * (p - q0);
				else
					n = -cd / l;
				if (r0 < cr)
					q = co + p - q0;
				else
					q = co + cr / r0 * (p - q0);
			}
			else if (a > 1) {
				if (r0 > cr + (a - 1) * l)
					n = (1 / r0) * (p - q0);
				else
					n = cd / l;
				if (r0 < cr)
					q = co + cd + p - q0;
				else
					q = co + cd + cr / r0 * (p - q0);
			}
			else {
				q = q0 + (cr / r0) * (p - q0);
				n = (1 / r0) * (p - q0);
			}
		}
		/// <summary>
		/// find point on line closest to second line
		/// </summary>
		/// <typeparam name="T">coordinate type</typeparam>
		/// <param name="lo">point on line</param>
		/// <param name="ld">direction of line</param>
		/// <param name="lo2">point on second line</param>
		/// <param name="ld2">direction of second line</param>
		/// <param name="q">point on line closest to second line if this exists</param>
		/// <returns>returns whether closest point exists, what is not the case for parallel lines </returns>
		template <typename T>
		bool closest_point_on_line_to_line(const fvec<T, 3>& lo, const fvec<T, 3>& ld, const fvec<T, 3>& lo2, const fvec<T, 3>& ld2, fvec<T, 3>& q)
		{
			T a = dot(ld, ld);
			T b = dot(ld, ld2);
			T e = dot(ld2, ld2);

			T d = a * e - b * b;
			if (std::abs(d) < 1e-10f) // lines are parallel
				return false;
			fvec<T, 3> r = lo - lo2;
			T c = dot(ld, r);
			T f = dot(ld2, r);
			T s = (b * f - c * e) / d;
			q = lo + s * ld;
			//T t = (a * f - b * c) / d;
			return true;
		}
		/// <summary>
		/// find point on line closest to reference point
		/// </summary>
		/// <typeparam name="T">coordinate type</typeparam>
		/// <param name="lo">point on line</param>
		/// <param name="ld">direction of line</param>
		/// <param name="p">reference point</param>
		/// <returns>point on line closest to reference point</returns>
		template <typename T>
		fvec<T, 3> closest_point_on_line_to_point(const fvec<T, 3>& lo, const fvec<T, 3>& ld, const fvec<T, 3>& p)
		{
			return lo + (dot(p - lo, ld) / dot(ld, ld)) * ld;
		}
		/// <summary>
		/// find point on circle closest to reference point
		/// </summary>
		/// <typeparam name="T">coordinate type</typeparam>
		/// <param name="co">circle center</param>
		/// <param name="cn">circle plane normal</param>
		/// <param name="cr">circle radius</param>
		/// <param name="p">reference point</param>
		/// <param name="q">point on circle closest to reference point, if this exists</param>
		/// <returns>whether closest point on circle exists, what is not the case for points on line through circle center orthogonal to circle plane</returns>
		template <typename T>
		bool closest_point_on_circle_to_point(const fvec<T, 3>& co, const fvec<T, 3>& cn, T cr, const fvec<T, 3>& p, fvec<T, 3>& q)
		{
			fvec<T, 3> d = p - co;
			d -= (dot(d, cn) / dot(cn, cn)) * cn;
			T l = d.length();
			if (l < 1e-6f)
				return false;
			q = co + (cr / l) * d;
			return true;
		}
		template <typename T>
		T closest_point_on_line_to_circle_impl(const fvec<T, 3>& lo, const fvec<T, 3>& ld, const fvec<T, 3>& co, const fvec<T, 3>& cn, T cr, fvec<T, 3>& q, int* iter_ptr = 0)
		{
			std::default_random_engine e;
			std::uniform_real_distribution<T> d(-0.01f, 0.01f);
			q = lo;
			fvec<T, 3> q0;
			int iter = 0;
			T dist = std::numeric_limits<T>::max();
			do {
				q0 = q;
				int cnt = 0;
				fvec<T, 3> q1 = q;
				while (!closest_point_on_circle_to_point(co, cn, cr, q, q)) {
					q += fvec<T, 3>(d(e), d(e), d(e));
					if (++cnt > 10) {
						std::cerr << "jittering not working!" << std::endl;
						return std::numeric_limits<T>::max();
					}
				}
				q1 = q;
				q = closest_point_on_line_to_point(lo, ld, q);
				dist = (q - q1).length();
				if (++iter > 1000) {
					std::cerr << "no convergence!" << std::endl;
					if (iter_ptr)
						*iter_ptr = iter;
					return std::numeric_limits<T>::max();
				}
			} while ((q0 - q).length() > 1e-6f);
			if (iter_ptr)
				*iter_ptr = iter;
			return dist;
		}
		/// <summary>
		/// find point on line closest to circle
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="lo">line point</param>
		/// <param name="ld">line direction</param>
		/// <param name="co">circle center</param>
		/// <param name="cn">circle plane normal</param>
		/// <param name="cr">circle radius</param>
		/// <param name="q">point on line closest to circle, if exists</param>
		/// <param name="iter_ptr">pointer to receive iteration count</param>
		/// <returns>return distance to circle or std::numeric_limits<T>::max() is iterative procedure fails</returns>
		template <typename T> 
		T closest_point_on_line_to_circle(const fvec<T, 3>& lo, const fvec<T, 3>& ld, const fvec<T, 3>& co, const fvec<T, 3>& cn, T cr, fvec<T, 3>& q, int* iter_ptr = 0)
		{
			T dist = closest_point_on_line_to_circle_impl(lo, ld, co, cn, cr, q);
			if (dist == std::numeric_limits<T>::max())
				return dist;

			fvec<T, 3> lo2 = closest_point_on_line_to_point(lo, ld, co);
			lo2 = 2.0f * lo2 - q;
			fvec<T, 3> q2;
			T dist2 = cgv::math::closest_point_on_line_to_circle_impl(lo2, ld, co, cn, cr, q2);
			if (dist2 == std::numeric_limits<float>::max())
				return dist2;
			if (dist < dist2)
				return dist;
			q = q2;
			return dist2;
		}
	}
}

#include <cgv/config/lib_end.h>