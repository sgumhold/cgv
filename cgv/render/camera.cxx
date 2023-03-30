#include "camera.h"

namespace cgv {
	namespace render {

		float distorted_pinhole::standard_epsilon = 1e-5f;

		unsigned distorted_pinhole::standard_max_nr_iterations = 20;

		bool distorted_pinhole::apply_distortion_model(const vec2& xd, vec2& xu, float epsilon, mat2* J_ptr) {
			vec2 od = xd - dc;
			float xd2 = od[0] * od[0];
			float yd2 = od[1] * od[1];
			float xyd = od[0] * od[1];
			float rd2 = xd2 + yd2;
			if (rd2 > max_radius_for_projection * max_radius_for_projection)
				return false;
			float v = 1.0f + rd2 * (k[3] + rd2 * (k[4] + rd2 * k[5]));
			if (fabs(v) < epsilon)
				return false;
			float inv_v = 1.0f / v;
			float u = 1.0f + rd2 * (k[0] + rd2 * (k[1] + rd2 * k[2]));
			float f = u * inv_v;
			xu[0] = f * od[0] + 2.0f * xyd * p[0] + (3.0f * xd2 + yd2) * p[1];
			xu[1] = f * od[1] + 2.0f * xyd * p[1] + (xd2 + 3.0f * yd2) * p[0];
			if (J_ptr) {
				mat2& J = *J_ptr;
				float df = 2.0f * rd2 * ((k[0] + rd2 * (2.0f * k[1] + 3.0f * rd2 * k[2])) * inv_v - (k[3] + rd2 * (2.0f * k[4] + 3.0f * rd2 * k[5])) * (u * inv_v)) * inv_v;
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
		unsigned distorted_pinhole::invert_distortion_model(const vec2& xu, vec2& xd, unsigned max_nr_iterations, float epsilon) {
			// start with approximate inversion
			vec2 od = xu - dc;
			float xd2 = od[0] * od[0];
			float yd2 = od[1] * od[1];
			float xyd = od[0] * od[1];
			float rd2 = xd2 + yd2;
			float inverse_radial = 1.0f + rd2 * (k[3] + rd2 * (k[4] + rd2 * k[5]));
			float enumerator = 1.0f + rd2 * (k[0] + rd2 * (k[1] + rd2 * k[2]));
			if (fabs(enumerator) >= epsilon)
				inverse_radial /= enumerator;
			od *= inverse_radial;
			od -= vec2((yd2 + 3 * xd2) * p[1] + 2 * xyd * p[0], (xd2 + 3 * yd2) * p[0] + 2 * xyd * p[1]);
			xd = od + dc;
			// iteratively improve approximation
			vec2 xd_best(0.0f);
			float err_best = std::numeric_limits<float>::max();
			for (unsigned i = 0; i < max_nr_iterations; ++i) {
				mat2 J;
				vec2 xu_i;
				if (!apply_distortion_model(xd, xu_i, epsilon, &J))
					return 0;
				// check for convergence
				vec2 dxu = xu - xu_i;
				float err = dxu.sqr_length();
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

	}
}