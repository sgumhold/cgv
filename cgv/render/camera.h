#pragma once

#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/inv.h>
#include <cgv/render/render_types.h>
#include "lib_begin.h"

namespace cgv {
	namespace render {

/// all members necessary for undistorted pinhole camera
struct CGV_API pinhole : public render_types
{
	// extent in pixels
	unsigned w, h;
	// focal length in pixel units for x and y
	vec2 s;
	// optical center in pixel units
	vec2 c;
	// skew strength;
	float skew = 0.0f;

	mat23 get_camera_matrix() const {
		return { s[0], 0.0f, skew, s[1], c[0], c[1] };
	}
	mat3 get_squared_camera_matrix() const {
		return { s[0], 0.0f, 0.0f, skew, s[1], 0.0f, c[0], c[1], 1.0f };
	}
	mat4 get_homogeneous_camera_matrix() const {
		return { s[0], 0.0f, 0.0f, 0.0f, skew, s[1], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, c[0], c[1], 0.0f, 1.0f };
	}
	vec2 image_to_pixel_coordinates(const vec2& x) {
		return vec2(s[0] * x[0] + h * x[1] + c[0], s[1] * x[1] + c[1]);
	}
	vec2 pixel_to_image_coordinates(const vec2& p) {
		float y = (p[1] - c[1]) / s[1]; return vec2((p[0] - c[0] - h*y)/s[0], y);
	}
};

/// extension of pinhole to distorted pinhole
struct CGV_API distorted_pinhole : public pinhole
{
	/// default epsilon used to check for zero denominator or in case of inversion zero enumerator
	static float standard_epsilon;
	/// default maximum number of iterations used for inversion of distortion models
	static unsigned standard_max_nr_iterations;
	// distortion center
	vec2 dc;
	// internal calibration
	float k[6], p[2];
	// maximum radius allowed for projection
	float max_radius_for_projection = 10.0f;
	//! apply distortion model from distorted to undistorted image coordinates used in projection direction and return whether successful
	/*! Failure cases are zero denominator in distortion formula or radius larger than max projection radius. */
	bool apply_distortion_model(const vec2& xd, vec2& xu, float epsilon = standard_epsilon, mat2* J_ptr = 0);
	//! invert model for image coordinate inversion and in case of success return positive number of iterations (see details below) and 0 otherwise
	/*! Failure can only be due to failure of applying the distortion model (outside max projection radius or zero denominator)
		Non convergence can be checked if return value is equal to max_nr_iterations. In case of divergence (increase of error
		during iteration) the iteration is also terminated and the return value is max_nr_iterations plus the number of
		iterations taken.*/
	unsigned invert_distortion_model(const vec2& xu, vec2& xd, unsigned max_nr_iterations = standard_max_nr_iterations, float epsilon = standard_epsilon);
};

/// extend distorted pinhole with external calibration
struct CGV_API camera : public distorted_pinhole
{
	// external calibration
	mat34 pose;
};

	}
}

#include <cgv/config/lib_end.h>