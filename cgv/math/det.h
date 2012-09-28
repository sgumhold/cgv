#pragma once
#include <cgv/math/mat.h>
#include <cgv/math/perm_mat.h>
#include <cgv/math/diag_mat.h>
#include <cgv/math/low_tri_mat.h>
#include <cgv/math/up_tri_mat.h>
#include <cgv/math/lu.h>

namespace cgv {
namespace math {

//compute determinant of a diagonal matrix
template <typename T>
T det(const diag_mat<T>& m) 
{
	T v = (T)1.0;
	for(unsigned i =0; i< m.nrows(); i++)
		v *= m(i);
	return v;
}

//compute determinant of an lower triangular matrix
template <typename T>
T det(const low_tri_mat<T>& m) 
{
	T v = (T)1.0;
	for(unsigned i =0; i< m.nrows(); i++)
		v *= m(i,i);
	return v;
}

//compute determinant of an upper triangular matrix
template <typename T>
T det(const up_tri_mat<T>& m) 
{
	T v = (T)1.0;
	for(unsigned i =0; i< m.nrows(); i++)
		v *= m(i,i);
	return v;
}

//compute determinant of a permutation matrix
inline int det(const perm_mat& m) 
{
	//count number of transpositions needed to get the identity permutation
	perm_mat temp = m;
	unsigned num_transpositions = 0;
	for(unsigned i = 0; i < m.size();i++)
	{
		while(temp(i) != i)
		{
			std::swap(temp(i),temp(temp(i)));
			num_transpositions++;
		}
	}
	//if number of transpositions is even return 1 else -1
	if(num_transpositions % 2 == 0)
		return 1;
	else 
		return -1;
	
}

//compute determinant of a square matrix m
template <typename T>
T det(const mat<T>& m) 
{
	assert(m.is_square());
	perm_mat p;
	low_tri_mat<T> l;
	up_tri_mat<T> u;
	lu(m,p,l,u);
	return det(p)*det(l)*det(u);
}

//compute determinant of 2x2 matrix 
template<typename T>
T det_22(const T a11,const T a12,
		 const T a21,const T a22)
{
	return a11*a22-a21*a12;	
}

//compute determinant of 3x3 matrix 
template<typename T>
T det_33(const T a11,const T a12,const T a13,
	const T a21,const T a22,const T a23,
	const T a31,const T a32,const T a33)

{
		return a11 * (a22 * a33 - a32 * a23) -
		       a21 * (a12 * a33 - a32 * a13) +
		       a31 * (a12 * a23 - a22 * a13);
}

//compute determinant of 4x4 matrix 
template<typename T>
T det_44(const T a11,const T a12,const T a13,const T a14,
		 const T a21,const T a22,const T a23,const T a24,
		 const T a31,const T a32,const T a33,const T a34,
		 const T a41,const T a42,const T a43,const T a44)
	{
		T a3344 = a33 * a44 - a43 * a34;
		T a2344 = a23 * a44 - a43 * a24;
		T a2334 = a23 * a34 - a33 * a24;
		T a1344 = a13 * a44 - a43 * a14;
		T a1334 = a13 * a34 - a33 * a14;
		T a1324 = a13 * a24 - a23 * a14;

		return a11 * (a22 * a3344 - a32 * a2344 + a42 * a2334) -
		       a21 * (a12 * a3344 - a32 * a1344 + a42 * a1334) +
		       a31 * (a12 * a2344 - a22 * a1344 + a42 * a1324) -
		       a41 * (a12 * a2334 - a22 * a1334 + a32 * a1324);
	}

	}

}


