#pragma once

#include <array>

#include <cgv/math/fvec.h>
#include "rectangle.h"

using namespace cgv::math;
using namespace cgv::data;

namespace trajectory {
namespace util {
	template <typename T> class ray_rect_intersection {
	  private:
		static constexpr T epsilon = T(1e-6);
		using vec3 = cgv::math::fvec<T, 3>;

	  public:
		/*
		0-------1
		|       |
		|       |
		|       |
		2-------3
		*/

		struct plane_intersection_result {
			vec3 pos;
			T t;
			bool hit;
		};

		struct rect_intersection_result {
			vec3 pos;
			// percentage inside rect in relative coordinates [0 - 1], starting upper-left
			T px;
			T py;
			bool hit;
		};

		// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection
		static plane_intersection_result intersect_plane(const vec3 &point,
		                                                 const vec3 &normal,
		                                                 const vec3 &origin,
		                                                 const vec3 &dir)
		{
			auto n = normalize(normal);
			auto d = normalize(dir);

			plane_intersection_result res{};
			res.pos = vec3(T(0));

			auto denom = dot(n, d);
			if (denom > epsilon) {
				vec3 op = point - origin;
				res.t = dot(op, n) / denom;
				res.hit = (res.t >= T(0));
				if (res.hit) res.pos = origin + res.t * d;
			}

			return res;
		}

		// https://stackoverflow.com/a/21114992
		static rect_intersection_result intersect_rect(const rectangle &rect,
		                                               const vec3 &origin,
		                                               const vec3 &dir,
		                                               bool clockwise = false)
		{
			auto r10 = rect[1] - rect[0];
			auto r20 = rect[2] - rect[0];

			auto normal = cross(r10, r20);
			if (clockwise) normal = -normal;
			auto center = (rect[0] + rect[1] + rect[2] + rect[3]) / T(4);

			auto plane = intersect_plane(center, normal, origin, dir);
			rect_intersection_result res{plane.pos, T(-1), T(-1), false};

			if (plane.hit) {
				auto proj = res.pos - rect[0];
				auto u = dot(proj, r10);
				auto v = dot(proj, r20);
				auto r10sqr = dot(r10, r10);
				auto r20sqr = dot(r20, r20);

				res.hit = (u >= T(0) && u <= r10sqr && v >= T(0) && v <= r20sqr);
				res.px = u / r10sqr;
				res.py = v / r20sqr;
			}

			return res;
		}
	};
} // namespace util
} // namespace trajectory