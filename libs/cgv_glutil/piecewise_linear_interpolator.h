#pragma once

#include <vector>

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

	const std::vector<control_point>& ref_control_points() const { return control_points; }

	void clear() {
		min_t = 0.0f;
		max_t = 0.0f;
		control_points.clear();
	}

	bool empty() { return control_points.empty(); }

	void add_control_point(float t, T v) {	
		// keep track of the interval of t over all control points
		min_t = std::min(min_t, t);
		max_t = std::max(max_t, t);
		control_points.push_back(std::make_pair(t, v));
		// sorting the control points is necessary for efficiently finding the correct pair when interpolating
		std::sort(control_points.begin(), control_points.end(), [](const control_point& a, const control_point& b) { return a.first < b.first; });
	}

	T interpolate(float t) const {
		unsigned count = (unsigned)control_points.size();

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

	std::vector<T> interpolate(size_t n) const {
		std::vector<T> data(n, (T)0);
		size_t count = control_points.size();

		if(n == 0 || count == 0)
			return data;

		if(count == 1) {
			std::fill(data.begin(), data.end(), control_points[0].second);
			return data;
		}

		size_t l = 0;
		size_t r = 1;

		if(min_t < control_points[l].first)
			r = 0;

		// TODO: add option to define interpolation range
		float step = 1.0f / static_cast<float>(n - 1);

		for(size_t i = 0; i < n; ++i) {
			float t = min_t + static_cast<float>(i) * step;

			while(t > control_points[r].first && l < count - 1) {
				l = r;
				r = std::min(l + 1, count - 1);
			}

			if(l == r) {
				data[i] = (T)(control_points[r].second);
			} else {
				std::pair<float, T> n0 = control_points[l];
				std::pair<float, T> n1 = control_points[r];

				float t0 = n0.first;
				float t1 = n1.first;

				float a = (t - t0) / (t1 - t0);
			
				data[i] = (T)((1.0f - a) * n0.second + a * n1.second);
			}
		}

		return data;
	}
};

}
}

#include <cgv/config/lib_end.h>
