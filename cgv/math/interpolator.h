#pragma once

#include <algorithm>
#include <vector>

#include "functions.h"

namespace cgv {
namespace math {

template<typename T>
class control_point_container {
public:
	typedef std::pair<float, T> control_point;
	using const_iterator = typename std::vector<control_point>::const_iterator;

private:
	std::vector<control_point> control_points;

public:
	control_point_container() {}
	~control_point_container() {}

	void clear() {
		control_points.clear();
	}

	bool empty() const { return control_points.empty(); }

	size_t size() const {
		return control_points.size();
	}

	void push_back(float t, const T& v) {
		control_points.push_back(std::make_pair(t, v));
		std::sort(control_points.begin(), control_points.end(), [](const control_point& a, const control_point& b) { return a.first < b.first; });
	}

	control_point operator [](size_t i) const {
		return control_points[i];
	}

	control_point& operator [](size_t i) {
		return control_points[i];
	}

	const std::vector<control_point>& ref_points() const {
		return control_points;
	}

	const_iterator begin() const { return control_points.cbegin(); }
	const_iterator end() const { return control_points.cend(); }

	control_point min() const {
		if(size() > 0)
			return control_points.front();
		return std::make_pair(0.0f, (T)0);
	}

	control_point max() const {
		if(size() > 0)
			return control_points.back();
		return std::make_pair(0.0f, (T)0);
	}
};

template<typename T>
class interpolator {
public:
	virtual T interpolate(control_point_container<T> control_points, float t) const = 0;
	
	virtual std::vector<T> interpolate(control_point_container<T> control_points, size_t n) const = 0;
};

}
}
