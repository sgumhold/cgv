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
	}
}