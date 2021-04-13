#pragma once

#include <cgv/math/mat.h>
#include <cgv/math/svd.h>
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>

namespace cgv {
	namespace pointcloud {
		/// class containing principal component analysis methods for 3 dimensions
		template<typename Real>
		class pca {
		public:
			using vec3 = cgv::math::fvec<Real, 3>;
			using mat3 = cgv::math::fmat<Real, 3,3>;
			using matrix_t = cgv::math::mat<Real>;
			using diag_matrix_t = cgv::math::diag_mat<Real>;
		private:
			// input data
			const matrix_t* A;
			// output data
			matrix_t centered_matrix;
			matrix_t mean_matrix;
			matrix_t covariance_matrix;
		public:
			pca() = default;
			/// principal component analysis for matrix m (computing constructor pattern)
			pca(const vec3* input_pnts, const size_t size) {
				compute(input_pnts,size);
			}

		private:
			void compute(const vec3* input_pnts, const size_t size) {
				auto centered_pnts = std::vector<vec3>(size);
				auto mean_pnts = std::vector<vec3>(size);
				mat3 covariance_matrix;
				//compute centered matrix by removing mean
				vec3 mean = vec3(0.0);
				for (int i = 0; i < size; ++i) {
					mean += input_pnts[i];
				}

				{
					const vec3* input = input_pnts;
					std::vector<vec3>::iterator it = centered_pnts.begin();
					while (it < centered_pnts.end()) {
						*it =  *input - mean;
						++input; ++it;
					}
				}
				//compute mean of each point
				for (int i = 0; i < size; ++i) {
					mean_pnts[i] = input_pnts[i] - centered_pnts[i];
				}

				covariance_matrix.zeros();
				// compute the covariance matrix
				for (int i = 0; i < size; ++i) {
					//build covariance matrice out of the sum of outer products
					covariance_matrix += mat3(centered_pnts[i], centered_pnts[i]); 
				}
				covariance_matrix = covariance_matrix/ (Real)size;

				//TODO solve system, find eigen vectors
			}
		};
	}
}

