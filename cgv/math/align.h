#pragma once

#include <cgv/math/fvec.h>
#include <cgv/math/quaternion.h>
#include <cgv/math/fmat.h>
#include <cgv/math/det.h>
#include <cgv/math/svd.h>

namespace cgv {
	namespace math {

		/// mean of point set
		template <typename T>
		fvec<T, 3> mean(const std::vector<fvec<T, 3> >& P, T inv_n = T(1) / P.size())
		{
			fvec<T, 3> mu(T(0));
			for (unsigned i = 0; i < P.size(); ++i)
				mu += P[i];
			mu *= inv_n;
			return mu;
		}

		/// svd wrapper
		template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
		void svd(const fmat<T, N, M>& A, fmat<T, N, N>& U, fvec<T, M>& D, fmat<T, M, M>& V_t, bool ordering = true, int maxiter = 30)
		{
			mat<T> _A(N, M, &A(0, 0)), _U, _V;
			diag_mat<T> _D;
			svd(_A, _U, _D, _V, ordering, maxiter);
			U = fmat<T, N, N>(N, N, &_U(0, 0));
			cgv::type::uint32_type i, j;
			for (j = 0; j < M; ++j) {
				D(j) = _D(j);
				for (i = 0; i < M; ++i)
					V_t(j, i) = _V(i, j);
			}
		}

		//! compute rigid body transformation and optionally uniform scaling to align source point set to target point set in least squares sense
		/*! Given source points X and target points Y find orthogonal matrix O\in O(3), translation vector t\in R^3 and optionally scale factor s > 0
		to minimize

		\sum_i ||y_i - (s*O*x_i + t)||^2

		Optionally, one can enforce O to be a rotation. Algorithm taken from

		Umeyama, Shinji. "Least-squares estimation of transformation parameters between two point patterns."
		IEEE Transactions on pattern analysis and machine intelligence 13.4 (1991): 376-380.

		Second template argument allows to specify a different number type for svd computation
		*/
		template <typename T, typename T_SVD = T>
		void align(
			const std::vector<fvec<T, 3> >& source_points, const std::vector<fvec<T, 3> >& target_points, // input point sets
			fmat<T, 3, 3>& O, fvec<T, 3>& t, T* scale_ptr = nullptr,                        // output transformation
			bool allow_reflection = false)										  // configuration
		{
			// ensure that source_points and target_points have same number of points and that we have at least 3 point pairs
			assert(source_points.size() == target_points.size() && source_points.size() > 2);
			T inv_n = T(1) / source_points.size();
			// compute mean points
			fvec<T, 3> mu_source = mean(source_points, inv_n);
			fvec<T, 3> mu_target = mean(target_points, inv_n);
			// compute covariance matrix and variances of point sets
			fmat<T, 3, 3> Sigma(T(0));
			T sigma_S = 0, sigma_T = 0;
			for (size_t i = 0; i < source_points.size(); ++i) {
				Sigma += fmat<T, 3, 3>(target_points[i] - mu_target, source_points[i] - mu_source);
				sigma_S += sqr_length(source_points[i] - mu_source);
				sigma_T += sqr_length(target_points[i] - mu_target);
			}
			Sigma *= inv_n;
			sigma_S *= inv_n;
			sigma_T *= inv_n;
			// compute SVD of covariance matrix
			fvec<T_SVD, 3> D;
			fmat<T_SVD, 3, 3> U, V_t;
			svd(fmat<T_SVD,3,3>(Sigma), U, D, V_t);
			// account for reflections
			fmat<T_SVD, 3, 3> S;
			S.identity();
			if (!allow_reflection && det(mat<T>(3,3,&Sigma(0,0))) < 0)
				S(2, 2) = T_SVD(-1);
			// compute results
			O = fmat<T,3,3>(U*S*V_t);
			if (scale_ptr) {
				*scale_ptr = T(D(0) + D(1) + S(2, 2)*D(2)) / sigma_S;
				t = mu_target - *scale_ptr * O * mu_source;
			}
			else
				t = mu_target - O*mu_source;
		}
	}
}