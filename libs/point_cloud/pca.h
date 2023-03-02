#pragma once
#include <array>

#include <cgv/math/mat.h>
#include <cgv/math/svd.h>
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>

namespace cgv {
	namespace pointcloud {
		/// class containing principal component analysis methods for 3 dimensions
		template<typename Real>
		class pca3 {
		public:
			using vec3 = cgv::math::fvec<Real, 3>;
			using mat3 = cgv::math::fmat<Real, 3,3>;
			using matrix_t = cgv::math::mat<Real>;
			using diag_matrix_t = cgv::math::diag_mat<Real>;
		private:
			// input data
			const matrix_t* A;
			// output data
			mat3 covariance_matrix;
			
			std::array<vec3,3> eigen_vector_data;
			std::array<Real,3> eigen_value_data;
			vec3 mean_point;

		public:
			pca3() = default;
			/// principal component analysis for matrix m (computing constructor pattern)
			/// if ordering is true eigen values are sorted in descending order, also the eigen vectors are reordered to conserve consistency
			pca3(const vec3* input_pnts, const size_t size, const bool ordering = true) {
				compute(input_pnts,size, ordering);
			}

			const size_t num_eigen_values() const {
				return 3;
			}

			const vec3* eigen_vectors_ptr() const {
				return eigen_vector_data.data();
			}

			const std::vector<vec3> eigen_vectors() const {
				return std::vector<vec3>(eigen_vectors_ptr(), eigen_vectors_ptr()+3);
			}

			const Real* eigen_values_ptr() const {
				return eigen_value_data.data();
			}

			const std::vector<Real> eigen_values() const {
				return std::vector<Real>(eigen_values_ptr(), eigen_values_ptr() + 3);
			}

			const vec3 mean() const {
				return mean_point;
			}

		private:
			void compute(const vec3* input_pnts, const size_t size, const bool ordering) {
				auto centered_pnts = std::vector<vec3>(size);
				auto mean_pnts = std::vector<vec3>(size);
				matrix_t U, V;
				diag_matrix_t sigma;
				//compute centered matrix by removing mean
				vec3 mean = vec3(0.0);
				for (int i = 0; i < size; ++i) {
					mean += input_pnts[i];
				}
				mean /= size;
				this->mean_point = mean;

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

				// do singular value decomposition 
				matrix_t A = matrix_t(3, 3, covariance_matrix.begin());
				cgv::math::svd(A, U, sigma, V, ordering);
				// columns of U contain vectors spanning the eigenspace and sigma contains the eigen values
				for (int i = 0; i < 3; ++i) {
					eigen_value_data[i] = sigma(i);
					for (int j = 0; j < 3; ++j) {
						eigen_vector_data[i](j) = U(j, i);
					}
				}
				//TODO extent to deal with matrix defect (num_eigen_values() < 3)
			}
		};
	}
}

