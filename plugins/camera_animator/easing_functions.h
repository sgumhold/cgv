#pragma once

#include <cgv/math/interpolator.h>

namespace easing_functions {

static inline float mix(float a, float b, float t) {
	
	return (1.0f - t) * a + t * b;
}

static inline float linear(float t) {

	return t;
}

static inline float smoothstart2(float t) {

	return t * t;
}

static inline float smoothstart3(float t) {

	return t * t * t;
}

static inline float smoothstart5(float t) {

	float t2 = t * t;
	return t2 * t2 * t;
}

static inline float smoothstart7(float t) {

	float t2 = t * t;
	float t4 = t2 * t2;
	return t4 * t2 * t;
}

static inline float smoothstop2(float t) {

	t = 1.0f - t;
	return 1.0f - t * t;
}

static inline float smoothstop3(float t) {

	t = 1.0f - t;
	return 1.0f - t * t * t;
}

static inline float smoothstop5(float t) {

	t = 1.0f - t;
	float t2 = t * t;
	return 1.0f - t2 * t2 * t;
}

static inline float smoothstop7(float t) {

	t = 1.0f - t;
	float t2 = t * t;
	float t4 = t2 * t2;
	return 1.0f - t4 * t2 * t;
}

static inline float smoothstep2(float t) {

	return mix(smoothstart2(t), smoothstop2(t), t);
}

static inline float smoothstep3(float t) {

	return mix(smoothstart3(t), smoothstop3(t), t);
}

static inline float smoothstep5(float t) {

	return mix(smoothstart5(t), smoothstop5(t), t);
}

static inline float smoothstep7(float t) {

	return mix(smoothstart7(t), smoothstop7(t), t);
}

}
