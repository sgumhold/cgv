#pragma once

#include <memory>

#include <cgv/math/interval.h>
#include <cgv/math/interpolate.h>

namespace cgv {
namespace media {

enum class MappingTransform {
	kLinear,
	kLog,
	kPow
};

template<typename T>
class sequential_scale {
public:
	cgv::math::interval<float> domain = { 0.0f, 1.0f };

	bool clamp = false;
	bool reverse = false;
	MappingTransform transform = MappingTransform::kLinear;
	bool diverging = false;
	float center = 0.5f;
	float exponent = 1.0f;
	float base = 10.0f;

	T unknown_value = T(0);

	sequential_scale() : sequential_scale(cgv::math::identity_interpolator<T, float>()) {}

	sequential_scale(const cgv::math::interpolator<T, float>& interpolator) {
		set_interpolator(interpolator);
	}

	sequential_scale(cgv::math::interval<float> domain, const cgv::math::interpolator<T, float>& interpolator) : domain(domain) {
		set_interpolator(interpolator);
	}

	std::shared_ptr<const cgv::math::interpolator<T, float>> get_interpolator() const {
		return interpolator_;
	}

	void set_interpolator(const cgv::math::interpolator<T, float>& interpolator) {
		interpolator_ = interpolator.clone();
	}

	void set_interpolator(std::shared_ptr<const cgv::math::interpolator<T, float>> interpolator) {
		interpolator_ = interpolator;
	}

	T evaluate(float v) const {
		// Todo: Mapping for diverging and center position.

		if(!interpolator_)
			return unknown_value;

		if(clamp)
			v = cgv::math::clamp(v, domain.lower_bound, domain.upper_bound);
		else if(v < domain.lower_bound || v > domain.upper_bound)
			return unknown_value;

		float t = 0.0f;

		switch(transform) {
		case MappingTransform::kLog:
		{
			float log_base = std::log(base);
			float log_lower = std::log(domain.lower_bound) / log_base;
			float log_upper = std::log(domain.upper_bound) / log_base;
			t = std::log(v) / log_base;
			t = (t - log_lower) / (log_upper - log_lower);
			break;
		}
		case MappingTransform::kPow:
		/* {
			float pow_lower = std::pow(domain.lower_bound, exponent);
			float pow_upper = std::pow(domain.upper_bound, exponent);
			t = std::pow(v, exponent);
			t = (t - pow_lower) / (pow_upper - pow_lower);
			break;
		}*/
			t = (v - domain.lower_bound) / domain.size();
			t = std::pow(t, exponent);
			break;
		default:
			t = (v - domain.lower_bound) / domain.size();
			break;
		}

		if(reverse)
			t = 1.0f - t;

		return interpolator_->at(t);
	}

private:
	std::shared_ptr<const cgv::math::interpolator<T, float>> interpolator_;
};

} // namespace media
} // namespace cgv
