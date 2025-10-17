#pragma once

#include <vector>

#include "interval.h"

namespace cgv {
namespace math {

/// Template class representing a regularly (equidistant breakpoints) sampled piecewise linear function/approximation.
template<typename T>
struct regular_piecewise_linear_function {
	/// The sample values of the function evenly spread over the domain.
	std::vector<T> values;
	/// The domain (range of input values) of the function.
	interval<T> domain = { T(0), T(1) };

	/// Return the function value at position x.
	T evaluate(T x) const {
		x = (x - domain.lower_bound) / domain.size();

		if(x <= T(0))
			return values.front();

		if(x >= T(1))
			return values.back();

		T num_segments = static_cast<T>(values.size() - 1);
		int64_t index = static_cast<int64_t>(x * num_segments);

		T step = T(1) / num_segments;
		T prev = static_cast<T>(index) * step;
		T curr = static_cast<T>(index + 1) * step;
		T t = (x - prev) / (curr - prev);
		return interpolate_linear(values[index], values[index + 1], t);
	}
};

} // namespace math
} // namespace cgv
