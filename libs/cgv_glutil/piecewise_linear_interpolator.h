#pragma once

#include <cgv/math/functions.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil {

template<typename T>
class piecewise_linear_interpolator {
public:
	typedef std::pair<float, T> control_point;

protected:
	float min_t = 0.0f;
	float max_t = 0.0f;
	std::vector<control_point> control_points;

public:
	size_t size() { return control_points.size(); }

	const std::vector<control_point>& ref_control_points() { return control_points; }

	void clear() {
		min_t = 0.0f;
		max_t = 0.0f;
		control_points.clear();
	}

	void add_control_point(float t, T v) {	
		// keep track of the interval of t over all control points
		min_t = std::min(min_t, t);
		max_t = std::max(max_t, t);
		control_points.push_back(std::make_pair(t, v));
		// sorting the control points is necessary for efficiently finding the correct pair when interpolating
		std::sort(control_points.begin(), control_points.end(), [](const control_point& a, const control_point& b) { return a.first < b.first; });
	}

	T interpolate(float t) const {
		unsigned count = control_points.size();

		if(count == 0)
			return (T)0;

		if(count == 1)
			return control_points[0].second;

		t = cgv::math::clamp(t, min_t, max_t);

		if(t > control_points[0].first) {
			unsigned idx = 0;

			for(unsigned i = 1; i < count; ++i) {
				if(control_points[idx].first <= t && control_points[i].first > t)
					break;
				idx = i;
			}

			if(idx < count - 1) {
				std::pair<float, T> n0 = control_points[idx];
				std::pair<float, T> n1 = control_points[idx + 1];

				float t0 = n0.first;
				float t1 = n1.first;

				float a = (t - t0) / (t1 - t0);

				return (T)((1.0f - a) * n0.second + a * n1.second);
			} else {
				return control_points[idx].second;
			}
		} else {
			return control_points[0].second;
		}
	}
};

}
}

#include <cgv/config/lib_end.h>
