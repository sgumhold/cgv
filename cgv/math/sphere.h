#pragma once
#include <cgv/math/vec.h>
#include <cgv/math/functions.h>
#include <cgv/math/det.h>
#include <cgv/math/lin_solve.h>
#include <limits>


namespace cgv{
	namespace math{

/**
* A sphere is defined as a vector (a,b,c,r) => center (a,b,c) and radius r
*/


///evaluate implicit sphere equation at x =(x1,x2,x3) 
///returns signed dist 
template <typename T>
T sphere_val(const vec<T>& sphere, const vec<T>& x)
{

	assert(sphere.size()-1  == x.size());

	T val=0;
	for(unsigned i = 0; i< x.size(); i++)
		val += sqr(x(i)-sphere(i));
	
	return sqrt(val)-sphere(sphere.size()-1);
}

///evaluate implicit sphere equation at x =(x1,x2,x3) 
///returns signed dist 
template <typename T>
vec<T> sphere_val(const vec<T>& sphere, const mat<T>& xs)
{

	assert(sphere.size()-1  == xs.ncols());

	vec<T> vals=0;
	for(unsigned i = 0; i< xs.ncols(); i++)
		vals(i) += sphere_val(sphere,xs.col(i));
	
	return vals;
}

///evaluate implicit sphere equation at x =(x1,x2,x3) 
///returns signed squared dist 
template <typename T>
T sphere_val2(const vec<T>& sphere, const vec<T>& x)
{

	assert(sphere.size()-1  == x.size());

	T val=0;
	for(unsigned i = 0; i< x.size(); i++)
		val += sqr(x(i)-sphere(i));

	return val-sqr(sphere(sphere.size()-1));
}

///evaluate implicit sphere equation at x =(x1,x2,x3) 
///returns signed dist 
template <typename T>
vec<T> sphere_val2(const vec<T>& sphere, const mat<T>& xs)
{

	assert(sphere.size()-1  == xs.ncols());

	vec<T> vals=0;
	for(unsigned i = 0; i< xs.ncols(); i++)
		vals(i) += sphere_val2(sphere,xs.col(i));
	
	return vals;
}

///construct smallest enclosing sphere of one point
template <typename T>
vec<T> sphere_fit(const vec<T>& p1)
{
	vec<T> sphere(4);
	sphere.set(p1(0),p1(1),p1(2),std::numeric_limits<T>::epsilon());
	
	return sphere;
}

///sphere through 2 points
template <typename T>
vec<T> sphere_fit(const vec<T>& p1,const vec<T>& p2)
{
	vec<T> sphere(4);
	vec<T> center = (T)0.5*(p1+p2);

	sphere.set(center(0),center(1),center(2),length(p2-center)+ std::numeric_limits<T>::epsilon());
	
	return sphere;
}

///sphere through 3 points
template<typename T>
vec<T> sphere_fit(const vec<T>& O,const vec<T>& A,const vec<T>& B)
{
	vec<T> a = A - O;
	vec<T> b = B - O;
	T denominator = (T)2.0 * dot(cross(a , b) , cross(a , b));
	vec<T> o = (dot(b,b) * cross(cross(a , b) , a) +
	            dot(a,a) * cross(b , cross(a , b))) / denominator;

	T	radius = length(o) + std::numeric_limits<T>::epsilon();
	vec<T>	center = O + o;
	
	return vec<T>(center(0),center(1),center(2),radius);

}




///sphere through 4 points
template <typename T>
vec<T> sphere_fit(const vec<T>& x1,const vec<T>& x2,const vec<T>& x3,const vec<T>& x4)
{
	mat<T> M(4,4);	
	M(0,0) = 1; M(0,1) = x1(0); M(0,2) = x1(1); M(0,3) = x1(2);
	M(1,0) = 1; M(1,1) = x2(0); M(1,2) = x2(1); M(1,3) = x2(2);
	M(2,0) = 1; M(2,1) = x3(0); M(2,2) = x3(1); M(2,3) = x3(2);
	M(3,0) = 1; M(3,1) = x4(0); M(3,2) = x4(1); M(3,3) = x4(2);
	vec<T> b(4);
	b(0) = -dot(x1,x1);
	b(1) = -dot(x2,x2);
	b(2) = -dot(x3,x3);
	b(3) = -dot(x4,x4);
	vec<T> x;
	cgv::math::solve(M,b,x);
	vec<T> center = vec<T>(-x(1)/(T)2.0,-x(2)/(T)2.0,-x(3)/(T)2.0);
	T radius  = sqrt(dot(center,center)-x(0));
	return vec<T>(center(0),center(1),center(2),radius);

	
}


//recursive helper function 
template <typename T>
vec<T> recurse_mini_ball(T** P, unsigned int p, unsigned int b=0)
{
	vec<T> mb;

	switch(b)
	{
		case 0:
			mb=vec<T>((T)0,(T)0,(T)0,(T)-1);
			break;
		case 1:
			mb = sphere_fit(vec<T>(3,(const T*)P[-1]));
			break;
		case 2:
			mb = sphere_fit(vec<T>(3,(const T*)P[-1]),vec<T>(3,(const T*)P[-2]));
			break;
		case 3:
			mb = sphere_fit(vec<T>(3,(const T*)P[-1]),vec<T>(3,(const T*)P[-2]),vec<T>(3,(const T*)P[-3]));
			break;
		case 4:
			mb = sphere_fit(vec<T>(3,(const T*)P[-1]),vec<T>(3,(const T*)P[-2]),vec<T>(3,(const T*)P[-3])
					,vec<T>(3,(const T*)P[-4]));
			return mb;
		}

		for(unsigned int i = 0; i < p; i++)
		{
			if(sphere_val2(mb,vec<T>(3,(const T*)P[i])) > 0)   // Signed square distance to sphere
			{
				for(unsigned int j = i; j > 0; j--)
				{
					T *a = P[j];
					P[j] = P[j - 1];
					P[j - 1] = a;
				}

				mb = recurse_mini_ball(P + 1, i, b + 1);
			}
		}

		return mb;
	}




///compute smallest enclosing sphere of points
template <typename T>
vec<T> mini_ball(cgv::math::mat<T>&  points)
{
	unsigned p = points.ncols();
	 T **L = new  T*[p];

	for(unsigned int i = 0; i < p; i++)
			L[i] = &(points(0,i));

	

	vec<T> mb = recurse_mini_ball(L, p);

	delete[] L;

	return mb;
	
}


	}
}

