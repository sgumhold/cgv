#pragma once

#include <vector>

namespace cgv {
	namespace math {

		/// permute array \c A with \c N values of type \c T according to permutation \c P such that \c A[i] moves to \c A[P[i]]
		template <typename T, typename I>
		void permute_array(size_t N, T* A, I* P) {
			for (size_t i = 0; i < N; ++i) {
				if (P[i] < 0) {
					// unmark permutation index
					P[i] = -P[i] - 1;
					continue;
				}
				T v = A[i];
				I j = P[i];
				while (j != i) {
					std::swap(A[j], v);
					I old_j = j;
					j = P[j];
					P[old_j] = -P[old_j] - 1;
				}
				A[i] = v;
			}
		}
		/// interface to permute function for arrays and permutations stored in vectors 
		template <typename T, typename I>
		void permute_vector(std::vector<T>& V, std::vector<I>& P) {
			permute_array(V.size(), &V.front(), &P.front());
		}

		/// permute \c nr_rows rows of nested 2D array with \c nr_columns columns, step sizes to step by one row / column need to be provided
		template <typename T, typename I>
		void permute_arrays(T* data, I* P, size_t nr_rows, size_t nr_columns, size_t row_step, size_t column_step)
		{
			for (size_t y = 0; y < nr_columns; ++y) {
				for (size_t x = 0; x < nr_rows; ++x) {
					if (P[x] < 0) {
						P[x] = -P[x] - 1;
						continue;
					}
					T v = data[x*row_step + y*column_step];
					size_t j = P[x];
					while (j != x) {
						std::swap(data[j*row_step + y*column_step], v);
						size_t old_j = j;
						j = P[j];
						P[old_j] = -P[old_j] - 1;
					}
					data[x*row_step + y*column_step] = v;
				}
			}
		}


	}
}