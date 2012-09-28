#pragma once

#include <cgv/math/mat.h>

#include <cgv/math/up_tri_mat.h>

#include <limits>
#include <algorithm>

namespace cgv {
namespace math {



//compute a cholesky factorisation of a square positive definite matrix
//returns false if matrix is not positive definite
// further if a is symmetric then a == l*l^t
template<typename T>
bool chol(const mat<T> &a, low_tri_mat<T> &l)

	{

		assert(a.is_square());

		unsigned N = a.nrows();

		l.resize(N);

				

		for(unsigned i = 0; i < N;i++)

		{

			for(unsigned j = i; j < N;j++)

			{

				T sum= a(i, j);

				for(int k = i - 1; k >= 0;k--)

				{

					sum -=  l(i, k) * l(j, k);

				}

 

				if(i == j)

				{

					if(sum <= 0)

						return false;//not positive definite

					else

						l(j, i) = sqrt(sum); 

				}

				else

					l(j, i) = sum / l(i, i);

			}

		}



   



		return true;			

	}


//returns true if A is positive definite otherwise false
template <typename T>
bool is_pos_def(const cgv::math::mat<T>& A)
{
	low_tri_mat<T> G;
	return chol(A,G);
}




}

}
