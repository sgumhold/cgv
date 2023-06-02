#pragma once

#include <cgv/math/interpolator.h>

namespace easing_functions {

static inline float mix(float a, float b, float t) {
	
	return (1.0f - t) * a + t * b;
}

static inline float first(float t) {

	return 0.0f;
}

static inline float last(float t) {

	return 1.0f;
}

static inline float nearest(float t) {

	return t < 0.5f ? 0.0f : 1.0f;
}

static inline float linear(float t) {

	return t;
}

static inline float smoothstart(float t) {

	return t * t;
}

static inline float smootherstart(float t) {

	return t * t * t;
}

static inline float smoothstop(float t) {

	t = 1.0f - t;
	return 1.0f - t * t;
}

static inline float smootherstop(float t) {

	t = 1.0f - t;
	return 1.0f - t * t * t;
}

static inline float smoothstep(float t) {

	return t * t * (3.0f - 2.0f*t);
}

static inline float smootherstep(float t) {

	return t * t * t * (t * (6.0f*t - 15.0f) + 10.0f);
}

static inline float smootheststep(float t) {

	return t * t * t * t * (t * (t * (70.0f - 20.0f*t) - 84.0f) + 35.0f);
}

enum class Id {
	kNone,
	kFirst,
	kLast,
	kNearest,
	kLinear,
	kSmoothStart,
	kSmootherStart,
	kSmoothStop,
	kSmootherStop,
	kSmoothStep,
	kSmootherStep,
	kSmoothestStep
};

static std::function<float(float)> from_id(Id id) {

	switch(id) {
	case Id::kFirst: return &easing_functions::first;
	case Id::kLast: return &easing_functions::last;
	case Id::kNearest: return &easing_functions::nearest;
	case Id::kLinear: return &easing_functions::linear;
	case Id::kSmoothStart: return &easing_functions::smoothstart;
	case Id::kSmootherStart: return &easing_functions::smootherstart;
	case Id::kSmoothStop: return &easing_functions::smoothstop;
	case Id::kSmootherStop: return &easing_functions::smootherstop;
	case Id::kSmoothStep: return &easing_functions::smoothstep;
	case Id::kSmootherStep: return &easing_functions::smootherstep;
	case Id::kSmoothestStep: return &easing_functions::smootheststep;
	case Id::kNone:
	default: return nullptr;
	}
}

static std::string names_string() {

	return
		"None,First,Last,Nearest,Linear,"
		"SmoothStart,SmootherStart,"
		"SmoothStop,SmootherStop,"
		"SmoothStep,SmootherStep,SmoothestStep";
}

}
