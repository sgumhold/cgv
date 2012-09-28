#pragma once

#include <cgv/math/mat.h>
#include <cgv/math/inv.h>
#include <cgv/math/constants.h>
#include <limits>
#include <algorithm>


namespace cgv {
namespace math {




///polar decomposition of matrix c=r*a
///r orthonormal matrix
///a positive semi-definite matrix 
template <typename T>
void polar(const mat<T> &c, mat<T> &r, mat<T> &a,int num_iter=15)
{
	r = c;
	for(int i =0; i < num_iter;i++)
		r = ((T)0.5)*(r+transpose(inv(r)));

	a = inv(r)*c;
	/*
	* Alternative way using svd:
	* c = u*d*v^t
	* r = u*v^t,  a=v*d*v^t 
	*/
}


/// extract axis and angle from rotation matrix 
///returns true if successful problematic cases are angle == 0° and angle == 180°
template <typename T>
bool decompose_rotation(const cgv::math::mat<T>& R,
		cgv::math::vec<T>& axis,
		T& angle)
{
	assert(R.nrows() == 3 && R.ncols() == 3);
	
	
	mat<T> A = (T)0.5*(R - transpose(R));
	mat<T> S = (T)0.5*(R + transpose(R));

	
	T abssina = (T)sqrt(0.5*(double)(A.frobenius_norm()*A.frobenius_norm()));
	T cosa = 0.5*(trace(S)-1.0);
	
		

	angle =(T)(asin(abssina)*180.0/PI);
	if(cosa < 0)
		angle = (T)180.0-angle;
		
	if(angle == 0 || angle == 180.0)
		 return false;
	
	axis = (T)1.0/abssina*cgv::math::vec<T>(A(2,1),A(0,2),A(1,0));

	return true;
}




}

}
