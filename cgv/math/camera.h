#pragma once

#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/inv.h>
#include "lib_begin.h"

namespace cgv {
	namespace math {

/// all members necessary for undistorted pinhole camera
template <typename T>
class pinhole
{
public:
	// extent in pixels
	unsigned w, h;
	// focal length in pixel units for x and y
	fvec<T,2> s;
	// optical center in pixel units
	fvec<T,2> c;
	// skew strength;
	T skew = 0.0f;

	fmat<T,2,3> get_camera_matrix() const {
		return { s[0], 0.0f, skew, s[1], c[0], c[1] };
	}
	fmat<T,3,3> get_squared_camera_matrix() const {
		return { s[0], 0.0f, 0.0f, skew, s[1], 0.0f, c[0], c[1], 1.0f };
	}
	fmat<T,4,4> get_homogeneous_camera_matrix() const {
		return { s[0], 0.0f, 0.0f, 0.0f, skew, s[1], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, c[0], c[1], 0.0f, 1.0f };
	}
	fvec<T,2> image_to_pixel_coordinates(const fvec<T,2>& x) {
		return vec2(s[0] * x[0] + h * x[1] + c[0], s[1] * x[1] + c[1]);
	}
	fvec<T,2> pixel_to_image_coordinates(const fvec<T,2>& p) {
		float y = (p[1] - c[1]) / s[1]; return vec2((p[0] - c[0] - h*y)/s[0], y);
	}
};

/// extension of pinhole to distorted pinhole
template <typename T>
class distorted_pinhole : public pinhole<T>
{
public:
	/// default epsilon used to check for zero denominator or in case of inversion zero enumerator
	static T standard_epsilon = T(1e-6f);
	/// default maximum number of iterations used for inversion of distortion models
	static unsigned standard_max_nr_iterations = 20;
	// distortion center
	fvec<T,2> dc;
	// internal calibration
	T k[6], p[2];
	// maximum radius allowed for projection
	T max_radius_for_projection = T(10);
	//! apply distortion model from distorted to undistorted image coordinates used in projection direction and return whether successful
	/*! Failure cases are zero denominator in distortion formula or radius larger than max projection radius. */
	bool apply_distortion_model(const fvec<T, 2>& xd, fvec<T, 2>& xu, T epsilon = standard_epsilon, fmat<T, 2, 2>* J_ptr = 0) {
		fvec<T,2> od = xd - dc;
		T xd2 = od[0] * od[0];
		T yd2 = od[1] * od[1];
		T xyd = od[0] * od[1];
		T rd2 = xd2 + yd2;
		if (rd2 > max_radius_for_projection * max_radius_for_projection)
			return false;
		T v = 1.0f + rd2 * (k[3] + rd2 * (k[4] + rd2 * k[5]));
		if (fabs(v) < epsilon)
			return false;
		T inv_v = 1.0f / v;
		T u = 1.0f + rd2 * (k[0] + rd2 * (k[1] + rd2 * k[2]));
		T f = u * inv_v;
		xu[0] = f * od[0] + 2.0f * xyd * p[0] + (3.0f * xd2 + yd2) * p[1];
		xu[1] = f * od[1] + 2.0f * xyd * p[1] + (xd2 + 3.0f * yd2) * p[0];
		if (J_ptr) {
			fmat<T,2,2>& J = *J_ptr;
			T df = 2.0f * rd2 * ((k[0] + rd2 * (2.0f * k[1] + 3.0f * rd2 * k[2])) * inv_v - (k[3] + rd2 * (2.0f * k[4] + 3.0f * rd2 * k[5])) * (u * inv_v)) * inv_v;
			J(0, 0) = f + df * xd2 + 2.0f * od[1] * p[0] + 6.0f * od[0] * p[1];
			J(1, 1) = f + df * yd2 + 6.0f * od[1] * p[0] + 2.0f * od[0] * p[1];
			J(1, 0) = J(0, 1) = df * xyd + 2.0f * (od[0] * p[0] + od[1] * p[1]);
		}
		return true;
	}
	//! invert model for image coordinate inversion and in case of success return positive number of iterations (see details below) and 0 otherwise
	/*! Failure can only be due to failure of applying the distortion model (outside max projection radius or zero denominator)
		Non convergence can be checked if return value is equal to max_nr_iterations. In case of divergence (increase of error
		during iteration) the iteration is also terminated and the return value is max_nr_iterations plus the number of
		iterations taken.*/
	unsigned invert_distortion_model(const fvec<T,2>& xu, fvec<T,2>& xd, unsigned max_nr_iterations = standard_max_nr_iterations, T epsilon = standard_epsilon) {
		// start with approximate inversion
		fvec<T,2> od = xu - dc;
		T xd2 = od[0] * od[0];
		T yd2 = od[1] * od[1];
		T xyd = od[0] * od[1];
		T rd2 = xd2 + yd2;
		T inverse_radial = 1.0f + rd2 * (k[3] + rd2 * (k[4] + rd2 * k[5]));
		T enumerator = 1.0f + rd2 * (k[0] + rd2 * (k[1] + rd2 * k[2]));
		if (fabs(enumerator) >= epsilon)
			inverse_radial /= enumerator;
		od *= inverse_radial;
		od -= fvec<T,2>((yd2 + 3 * xd2) * p[1] + 2 * xyd * p[0], (xd2 + 3 * yd2) * p[0] + 2 * xyd * p[1]);
		xd = od + dc;
		// iteratively improve approximation
		fvec<T,2> xd_best(0.0f);
		T err_best = std::numeric_limits<T>::max();
		for (unsigned i = 0; i < max_nr_iterations; ++i) {
			fmat<T,2,2> J;
			fvec<T,2> xu_i;
			if (!apply_distortion_model(xd, xu_i, epsilon, &J))
				return 0;
			// check for convergence
			fvec<T,2> dxu = xu - xu_i;
			T err = dxu.sqr_length();
			if (err < epsilon * epsilon)
				return i;
			// check for divergence
			if (err > err_best) {
				xd = xd_best;
				return i + max_nr_iterations;
			}
			// improve approximation before the last iteration
			xd_best = xd;
			err_best = err;
			xd += inv(J) * dxu;
		}
		return max_nr_iterations;
	}
};

/// extend distorted pinhole with external calibration
template <typename T>
struct CGV_API camera : public distorted_pinhole<T>
{
	// external calibration
	fmat<T,3,4> pose;
};

	}
}

#include <cgv/config/lib_end.h>