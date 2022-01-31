#pragma once

#include <cgv/math/lin_solve.h>


namespace cgv{
	namespace math{

template <typename T>
bool line_line_intersection(const vec<T>& x1,const vec<T>& d1,const vec<T>& x2,const vec<T>& d2, T& t1, T& t2, T eps=0.00001)
{
	mat<T> A(x1.dim(),2);
	A.set_col(0,d1);
	A.set_col(1,d2);
	vec<T> b = transpose(A)*(x1-x2);
	A=transpose(A)*A;
	vec<T> t;
	solve(A,b,t);
	vec<T> s1 = x1 + t(0)*d1;	
	vec<T> s2 = x2 + t(1)*d2;
	t1 = t(0);
	t2 = t(1);
	return length(s1-s2)<= eps;	
}

template <typename T>
bool line_circle_intersection(const vec<T>& x,const vec<T>& d,const vec<T>& center,const vec<T>& normal,
							  const T& radius, vec<T>& nearest_point, T eps=0.00001)
{
	
	T s = dot(normal,d);
	
	if(s == 0)
		return false;
	T t = dot(normal,x)-dot(center,normal);
	
	t/=-s;
	
	vec<T> v1 = x+t*d-center;
	nearest_point = radius*normalize(v1)+center;
	
	
	if(t < 0)
		return false;
	return ( fabs(length(v1)-radius) <= eps);
	
}

template <typename T>
bool ray_plane_intersection(const vec<T>& x,const vec<T>& d,const vec<T>& plane, T& t)
{
	
	t =0;
	T s =0;
	unsigned n = x.dim();
	for(unsigned i = 0; i < n; i++)
	{
		t+=plane(i)*x(i);
		s+=plane(i)*d(i);
	}
	if(s == 0)
		return false;
	t+=plane(n);
	t/=-s;
	return ( t >= 0);
		
}

	}
}

