#pragma once

#include "interpolator.h"

namespace cgv {
namespace math {

template<typename T>
class piecewise_nearest_interpolator : public interpolator<T> {
public:
	T interpolate(control_point_container<T> control_points, float t) const {
		size_t count = control_points.size();

		if(count == 0)
			return (T)0;

		if(count == 1)
			return control_points[0].second;

		t = cgv::math::clamp(t, control_points.min().first, control_points.max().first);

		if(t > control_points[0].first) {
			size_t idx = 0;

			for(size_t i = 1; i < count; ++i) {
				if(control_points[idx].first <= t && control_points[i].first > t)
					break;
				idx = i;
			}

			if(idx < count - 1) {
				std::pair<float, T> n0 = control_points[idx];
				std::pair<float, T> n1 = control_points[idx + 1];

				float t0 = n0.first;
				float t1 = n1.first;

				return (t - t0) < (t1 - t) ? n0.second : n1.second;
			} else {
				return control_points[idx].second;
			}
		} else {
			return control_points[0].second;
		}
	}

	std::vector<T> interpolate(control_point_container<T> control_points, size_t n) const {
		size_t count = control_points.size();

		std::vector<T> data(n, (T)0);

		if(n == 0 || count == 0)
			return data;

		if(count == 1) {
			std::fill(data.begin(), data.end(), control_points[0].second);
			return data;
		}

		size_t l = 0;
		size_t r = 1;

		if(control_points.min().first > 0.0f)
			r = 0;

		float step = 1.0f / static_cast<float>(n - 1);

		for(size_t i = 0; i < n; ++i) {
			float t = static_cast<float>(i) * step;

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

				data[i] = (t - t0) < (t1 - t) ? n0.second : n1.second;
			}
		}

		return data;
	}
};

}
}
