#pragma once

#include "fvec.h"
#include "pose.h"
#include "functions.h"
#include <limits>

#include "lib_begin.h"

namespace cgv {
	namespace math {
		template <typename T>
		T ray_cylinder_intersection(const fvec<T, 3>& ro, const fvec<T, 3>& rd, const fvec<T, 3>& pa, const fvec<T, 3>& ca, T ra, fvec<T, 3>& n)
		{
			fvec<T, 3> oc = ro - pa;
			T caca = dot(ca, ca);
			T card = dot(ca, rd);
			T caoc = dot(ca, oc);
			T a = caca - card * card;
			T b = caca * dot(oc, rd) - caoc * card;
			T c = caca * dot(oc, oc) - caoc * caoc - ra * ra * caca;
			T h = b * b - a * c;
			if (h < 0.0)
				return std::numeric_limits<T>::max();
			h = sqrt(h);
			T t = (-b - h) / a;
			// body
			T y = caoc + t * card;
			if (y > 0.0f && y < caca) {
				n = (oc + t * rd - ca * y / caca) / ra;
				return t;
			}
			// caps
			t = (((y < 0.0f) ? 0.0f : caca) - caoc) / card;
			if (std::abs(b + a * t) < h) {
				n = ca * sign(y) / caca;
				return t;
			}
			return std::numeric_limits<T>::max();
		}
		template <typename T>
		T ray_torus_intersection(const fvec<T, 3>& ro, const fvec<T, 3>& rd, const fvec<T, 2> tor, fvec<T, 3>& n0)
		{
			//T torIntersect(in fvec<T,3> ro, in fvec<T,3> rd, in fvec<T,2> tor)
			T po = 1.0;
			T Ra2 = tor.x() * tor.x();
			T ra2 = tor.y() * tor.y();
			T m = dot(ro, ro);
			T n = dot(ro, rd);
			T k = (m + Ra2 - ra2) / 2.0f;
			T k3 = n;
			const fvec<T, 2>& ro_xy = reinterpret_cast<const fvec<T, 2>&>(ro);
			const fvec<T, 2>& rd_xy = reinterpret_cast<const fvec<T, 2>&>(rd);
			T k2 = n * n - Ra2 * dot(rd_xy, rd_xy) + k;
			T k1 = n * k - Ra2 * dot(rd_xy, ro_xy);
			T k0 = k * k - Ra2 * dot(ro_xy, ro_xy);

			if (std::abs(k3 * (k3 * k3 - k2) + k1) < 0.01f)
			{
				po = -1.0f;
				T tmp = k1; k1 = k3; k3 = tmp;
				k0 = 1.0f / k0;
				k1 = k1 * k0;
				k2 = k2 * k0;
				k3 = k3 * k0;
			}

			T c2 = k2 * 2.0f - 3.0f * k3 * k3;
			T c1 = k3 * (k3 * k3 - k2) + k1;
			T c0 = k3 * (k3 * (c2 + 2.0f * k2) - 8.0f * k1) + 4.0f * k0;
			c2 /= 3.0f;
			c1 *= 2.0f;
			c0 /= 3.0f;
			T Q = c2 * c2 + c0;
			T R = c2 * c2 * c2 - 3.0f * c2 * c0 + c1 * c1;
			T h = R * R - Q * Q * Q;

			if (h >= 0.0)
			{
				h = sqrt(h);
				T v = sign(R + h) * std::pow(std::abs(R + h), 1.0f / 3.0f); // cube root
				T u = sign(R - h) * std::pow(std::abs(R - h), 1.0f / 3.0f); // cube root
				fvec<T, 2> s = fvec<T, 2>((v + u) + 4.0f * c2, (v - u) * sqrt(3.0f));
				T y = sqrt(0.5f * (length(s) + s.x()));
				T x = 0.5f * s.y() / y;
				T r = 2.0f * c1 / (x * x + y * y);
				T t1 = x - r - k3; t1 = (po < 0.0f) ? 2.0f / t1 : t1;
				T t2 = -x - r - k3; t2 = (po < 0.0f) ? 2.0f / t2 : t2;
				T t = 1e20f;
				if (t1 > 0.0f) t = t1;
				if (t2 > 0.0f) t = std::min(t, t2);
				fvec<T, 3> pos = ro + t * rd;
				n0 = normalize(pos * ((dot(pos, pos) - ra2) * fvec<T, 3>(1, 1, 1) - Ra2 * fvec<T, 3>(1, 1, -1)));
				return t;
			}

			T sQ = sqrt(Q);
			T w = sQ * cos(acos(-R / (sQ * Q)) / 3.0f);
			T d2 = -(w + c2);
			if (d2 < 0.0f)
				return std::numeric_limits<T>::max();
			T d1 = sqrt(d2);
			T h1 = sqrt(w - 2.0f * c2 + c1 / d1);
			T h2 = sqrt(w - 2.0f * c2 - c1 / d1);
			T t1 = -d1 - h1 - k3; t1 = (po < 0.0f) ? 2.0f / t1 : t1;
			T t2 = -d1 + h1 - k3; t2 = (po < 0.0f) ? 2.0f / t2 : t2;
			T t3 = d1 - h2 - k3;  t3 = (po < 0.0f) ? 2.0f / t3 : t3;
			T t4 = d1 + h2 - k3;  t4 = (po < 0.0f) ? 2.0f / t4 : t4;
			T t = 1e20f;
			if (t1 > 0.0f) t = t1;
			if (t2 > 0.0f) t = std::min(t, t2);
			if (t3 > 0.0f) t = std::min(t, t3);
			if (t4 > 0.0f) t = std::min(t, t4);
			fvec<T, 3> pos = ro + t * rd;
			n0 = normalize(pos * ((dot(pos, pos) - ra2) * fvec<T, 3>(1, 1, 1) - Ra2 * fvec<T, 3>(1, 1, -1)));
			return t;
		}
		template <typename T>
		T ray_torus_intersection(const fvec<T, 3>& ro, const fvec<T, 3>& rd, const fvec<T, 3>& to, const fvec<T, 3>& tn, const fvec<T, 2> tr, fvec<T, 3>& n)
		{
			// compute pose transformation
			fmat<T, 3, 4> pose;
			cgv::math::pose_position(pose) = to;
			fvec<T, 3>& x = reinterpret_cast<fvec<T, 3>&>(pose[0]);
			fvec<T, 3>& y = reinterpret_cast<fvec<T, 3>&>(pose[3]);
			fvec<T, 3>& z = reinterpret_cast<fvec<T, 3>&>(pose[6]);
			z = tn;
			x = tn;
			int i = std::abs(tn[0]) < std::abs(tn[1]) ? 0 : 1;
			i = std::abs(tn[i]) < std::abs(tn[2]) ? i : 2;
			x[i] = 1.0f;
			y = normalize(cross(tn, x));
			x = cross(y, tn);
			// transform ray into torus pose
			T t = ray_torus_intersection(
				cgv::math::inverse_pose_transform_point(pose, ro),
				cgv::math::inverse_pose_transform_vector(pose, rd), tr, n);
			// in case of intersection, transform normal back to world space
			if (t < std::numeric_limits<T>::max())
				n = cgv::math::pose_transform_vector(pose, n);
			return t;
		}
		template <typename T>
		int ray_sphere_intersection(const fvec<T,3>& ro, const fvec<T,3>& rd, const fvec<T,3>& ce, T ra, fvec<T, 2>& res)
		{
			fvec<T,3> d = ro - ce;
			T il = T(1)/dot(rd, rd);
			T b = il*dot(d, rd);
			T c = il*(dot(d, d) - ra * ra);
			T D = b * b - c;
			if (D < 0.0)
				return 0;
			if (D < std::numeric_limits<T>::epsilon()) {
				res = -b;
				return 1;
			}
			D = std::sqrt(D);
			res = fvec<T,2>(-b - D, -b + D);
			return 2;
		}
		template <typename T>
		int ray_box_intersection(const fvec<T,3>& ro, const fvec<T,3>& rd, fvec<T,3> boxSize, fvec<T,2>& res, fvec<T,3>& outNormal)
		{
			fvec<T,3> m = fvec<T,3>(T(1)) / rd; // can precompute if traversing a set of aligned boxes
			fvec<T,3> n = m * ro;   // can precompute if traversing a set of aligned boxes
			fvec<T,3> k = abs(m) * boxSize;
			fvec<T,3> t1 = -n - k;
			fvec<T,3> t2 = -n + k;
			T tN = std::max(std::max(t1.x(), t1.y()), t1.z());
			T tF = std::min(std::min(t2.x(), t2.y()), t2.z());
			if (tN > tF || tF < 0.0)
				return 0;
			outNormal = -sign(rd)
				* step(fvec<T, 3>(t1.y(), t1.z(), t1.x()), fvec<T, 3>(t1.x(), t1.y(), t1.z()))
				* step(fvec<T, 3>(t1.z(), t1.x(), t1.y()), fvec<T, 3>(t1.x(), t1.y(), t1.z()));
			res = fvec<T,2>(tN, tF);
			return 2;
		}
	}
}

#include <cgv/config/lib_end.h>