#include "ticks.h"

#include <cmath>
#include <cgv/math/compare_float.h>

namespace cgv {
namespace media {

std::tuple<int, int, int>  get_tick_specification(float start, float stop, int count) {
	const float e10 = std::sqrt(50.0f);
	const float e5 = std::sqrt(10.0f);
	const float e2 = std::sqrt(2.0f);

	const float step = (stop - start) / std::max(0, count);
	const float power = std::floor(std::log10(step));
	const float	error = step / std::pow(10.0f, power);
	const float factor = error >= e10 ? 10.0f : error >= e5 ? 5.0f : error >= e2 ? 2.0f : 1.0f;
	int i1, i2;
	float inc;
	if(power < 0) {
		inc = std::pow(10.0f, -power) / factor;
		i1 = static_cast<int>(std::round(start * inc));
		i2 = static_cast<int>(std::round(stop * inc));
		if(i1 / inc < start) ++i1;
		if(i2 / inc > stop) --i2;
		inc = -inc;
	} else {
		inc = std::pow(10.0f, power) * factor;
		i1 = static_cast<int>(std::round(start / inc));
		i2 = static_cast<int>(std::round(stop / inc));
		if(i1 * inc < start) ++i1;
		if(i2 * inc > stop) --i2;
	}
	if(i2 < i1 && 0.5 <= count && count < 2)
		return get_tick_specification(start, stop, count * 2);
	return { i1, i2, static_cast<int>(inc) };
};

std::vector<float> compute_ticks(float start, float stop, int count) {
	if(count <= 0)
		return {};

	if(start == stop)
		return { start };

	const bool reverse = stop < start;
	if(reverse)
		std::swap(start, stop);

	float i1, i2, inc;
	std::tie(i1, i2, inc) = get_tick_specification(start, stop, count);

	if(i2 < i1)
		return {};

	const int n = i2 - i1 + 1;
	std::vector<float> ticks(n);
	if(reverse) {
		if(inc < 0)
			for(int i = 0; i < n; ++i)
				ticks[i] = static_cast<float>(i2 - i) / static_cast<float>(-inc);
		else
			for(int i = 0; i < n; ++i)
				ticks[i] = static_cast<float>(i2 - i) * static_cast<float>(inc);
	} else {
		if(inc < 0)
			for(int i = 0; i < n; ++i)
				ticks[i] = static_cast<float>(i1 + i) / static_cast<float>(-inc);
		else
			for(int i = 0; i < n; ++i)
				ticks[i] = static_cast<float>(i1 + i) * static_cast<float>(inc);
	}
	return ticks;
};

std::vector<float> compute_ticks_log(float start, float stop, float base, int count) {
	const bool crosses_zero = std::signbit(start) != std::signbit(stop);

	if(count <= 0 || crosses_zero || cgv::math::is_zero(start) || cgv::math::is_zero(stop))
		return {};

	if(start == stop)
		return { start };

	const bool reverse = stop < start;
	if(reverse)
		std::swap(start, stop);

	const float log_start = std::log(start) / std::log(base);
	const float log_stop = std::log(stop) / std::log(base);
	const bool base_is_integral = cgv::math::is_equal(base, std::floor(base));

	std::vector<float> ticks;
	if(base_is_integral && static_cast<int>(std::ceil(log_stop - log_start)) < count) {
		int int_base = static_cast<int>(base);
		int i = static_cast<int>(std::floor(log_start));
		int j = static_cast<int>(std::ceil(log_stop));
		if(start > 0) {
			for(; i <= j; ++i) {
				for(int k = 1; k < int_base; ++k) {
					float t = i < 0 ? k / std::pow(base, -i) : k * std::pow(base, i);
					if(t < start) continue;
					if(t > stop) break;
					ticks.push_back(t);
				}
			}
		} else {
			for(; i <= j; ++i) {
				for(int k = int_base - 1; k >= 1; --k) {
					float t = i > 0 ? k / std::pow(base, -i) : k * std::pow(base, i);
					if(t < start) continue;
					if(t > stop) break;
					ticks.push_back(t);
				}
			}
		}
		if(ticks.size() * 2 < count)
			ticks = compute_ticks(start, stop, count);
	} else {
		ticks = compute_ticks(log_start, log_stop, std::min(static_cast<int>(std::ceil(log_stop - log_start)), count));
		for(float& tick : ticks)
			tick = std::pow(base, tick);
	}

	if(reverse)
		std::reverse(ticks.begin(), ticks.end());

	return ticks;
}

} // namespace media
} // namespace cgv
