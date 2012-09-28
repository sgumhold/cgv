#pragma once

#include <iostream>
#include <cmath>
#include <cassert>
#include <cgv/math/vec.h>
#include <cgv/math/mat.h>
#include <cgv/math/functions.h>
#include <cgv/math/lin_solve.h>

namespace cgv {
namespace math {

/**
* polynomials are stored in vectors in the following order:
* [a,b,c,d] ...a*x^3 + b*x^2 + c*x + d 
*/

/// evaluate a polynomial p at x
/// p is the vector of length n+1 whose elements are the coefficients 
/// of the polynomial in descending powers.
/// evaluation is done by using the horner scheme

template <typename T>

T poly_val(const vec<T>& p, const T& x)
{
	assert(p.size() > 0);
	T y=p(0);

	for(unsigned i = 1; i<p.size(); i++)
		y= y*x +p(i);

	return y;
}

/// evaluate a polynomial p at multiple positions x
/// p is the vector of length n+1 whose elements are the coefficients 
/// of the polynomial in descending powers.
/// evaluation is done by using the horner scheme
template <typename T>
vec<T> poly_val(const vec<T>& p, const vec<T>& x)

{

	assert(p.size() > 0);
	vec<T> y(x.size());
	y.fill(p(0));

	for(unsigned i = 1; i<p.size(); i++)
		y= x*y + p(i);

	return y;

}

///1d convolution or polynomial multiplication
template <typename T>
vec<T> poly_mult(const vec<T>& u, const vec<T>& v)
{
	unsigned s =u.size() + v.size()-1;
	vec<double> w(s);
	for(unsigned k = 1; k <= s; k++)
	{
		w(k-1) = 0;
		int j = 0;
		
		int jmin = (int)k+1-(int)v.size();
		if(jmin < 1)
			jmin = 1;

		int jmax = (int)u.size();
		if (jmax > (int)k)
			jmax=k;
		

		for(int j = jmin; j <=jmax; j++)
			w(k-1)+=u(j-1)*v(k-j);
	}
	return w;
}

///polynomial division
///f(x)/g(x) = q(x) + r(x)/g(x)
///returns true if r(x)=0 (rest 0)
template <typename T>
bool poly_div(const vec<T>& f, const vec<T>& g, vec<T>& q, vec<T>& r)
{
	if(g.size() > f.size())
	{
		q.resize(1);
		q(0)=0;
		r=f;
		return false;
	}
	else
	{
		vec<T> p = f;
		//std::cout << p << std::endl;
		q.resize(f.size()-g.size()+1);
		for(unsigned j = 0; g.size()+j <=p.size();j++)
		{
			q(j) = p(j)/g(0);
			for(unsigned i = j; i < g.size()+j; i++)
			{
				p(i)= p(i)- q(j)*g(i-j);
			}
			//std::cout <<p<<std::endl;
		}
		unsigned s=g.size();
		unsigned a=p.size()-g.size();
		while(p(a) == 0 && s > 0)
		{
			a++;s--;
		}

		if(s == 0)
		{
			r.resize(1);
			r(0)=0;
			return true;
		}
		else
		{
			r=p.sub_vec(a,s);		
			return false;
		}
	}
	
	
}

//returns addition of polynomials u and v
template <typename T>
vec<T> poly_add(const vec<T>& u, const vec<T>& v)
{
	unsigned n = u.size();
	unsigned m = v.size();

	vec<double> r;
	if(n >= m)
	{
		r.resize(n);
		unsigned i = 0;
		unsigned d = n-m;
		for(;i < d;i++)
			r(i) = u(i);
		
		for(;i < n;i++)
			r(i) = u(i)+v(i-d);
	}else
	{
		r.resize(m);
		unsigned i = 0;
		unsigned d = m-n;
		for(;i < d;i++)
			r(i) = v(i);	
		for(;i < m;i++)
			r(i) = v(i)+u(i-d);
	}
		
	return r;
}

//returns subtraction of polynomials u and v
template <typename T>
vec<T> poly_sub(const vec<T>& u, const vec<T>& v)
{
	unsigned n = u.size();
	unsigned m = v.size();

	vec<double> r;
	if(n >= m)
	{
		r.resize(n);
		unsigned i = 0;
		unsigned d = n-m;
		for(;i < d;i++)
			r(i) = u(i);
		
		for(;i < n;i++)
			r(i) = u(i)-v(i-d);
	}else
	{
		r.resize(m);
		unsigned i = 0;
		unsigned d = m-n;
		for(;i < d;i++)
			r(i) = -v(i);	
		for(;i < m;i++)
			r(i) = u(i-d)-v(i);
	}
		
	return r;
}


//analytic derivation of polynomial p 
template <typename T>
vec<T> poly_der(const vec<T>& p)
{
	assert(p.size() > 0);
	unsigned n = p.size(); 

	if(n <= 1)
	{
		vec<T> q(1);
		q(0) = 0;
		return q;
	}
	else
	{
		unsigned m = n-1;
		vec<T> q(m);
		for(unsigned i = 0; i< m; i++)
			q(i)= p(i)*(m-i);
		return q;
	}
}

//analytic integrate polynomial p using integration constant k
template <typename T>
vec<T> poly_int(const vec<T>& p, const T& k=0)
{
	assert(p.size() > 0);
	unsigned m = p.size()+1; 
	
	vec<T> q(m);
	for(unsigned i = 0; i < m-1; i++)
		q(i)= p(i)/(m-i-1);
		
	
	q(m-1)=k;
	return q;
}

///returns bernstein polynomial (bezier basis)
template <typename T>
vec<T> bernstein_polynomial(unsigned j, unsigned g)
{
	using namespace cgv::math;
	
	vec<T> p;
	p.zeros(j+1);
	p(0)=1.0;// p = t^j	

	
	for(unsigned i = 0; i < g-j; i++)	
		p = poly_mult(p, vec<T>(-1.0,1));	//p=p*(1-t)	
	
	return nchoosek(g,j)*p;
}


//returns lagrange basis polynomial
template <typename T>
vec<T> lagrange_basis_polynomial(unsigned i, const vec<T>& u)
{
	using namespace cgv::math;
	
	//polynomial p = 1
	vec<T> p(1);
	p(0)=1.0;	

	unsigned g = u.size()-1;

	for(unsigned j = 0; j <= g; j++)
	{
		if(i == j) continue;	
		p = poly_mult(p, vec<T>(1.0,-u(j))) / (u(i)-u(j));		
	}
	return p;
}


template <typename T>
vec<T> newton_basis_polynomial(unsigned i, const vec<T>& u)
{
	using namespace cgv::math;
	
	//polynomial p = 1
	vec<T> p(1);
	p(0)=1.0;	

	unsigned g = u.size()-1;

	for(unsigned j = 0; j <= g; j++)
	{
		
		p = poly_mult(p, vec<T>(1.0,-u(j)));		
	}
	return p;
}


//returns the vandermonde matrix which can be used for fitting 
//polynomials (for large order polynomials use orthogonal basis instead)
//x = [ 1 2 3 4] -> V(i,j) x(i)^(n-j) V is n x n with n = x.size()

template <typename T>
mat<T> vander(const vec<T>& x)
{
	return vander(x.size()-1,x);
}

//returns vandermonde matrix 
template <typename T>
mat<T> vander(unsigned degree,const vec<T>& x)
{
	assert(x.size() >= 1);
	unsigned n = x.size();
	cgv::math::mat<T> V(n,degree+1);
	
	for(unsigned i = 0; i < n; i++)
	{
		V(i,degree) = (T)1;
		for(int j = (int)degree-1; j>= 0; j--)
			V(i,j)=V(i,j+1)*x(i);
	}

	return  V;
}

//fit a polynomial of degree n into data such that E = sum_i ||p(X(i)) - Y(i)||² is minimized
//uses vandermonde matrix and svd solver 
//large degrees can be numerically instable keep n < 10
//otherwise use orthogonal basis for fitting
template <typename T>
vec<T> poly_fit(unsigned n,const vec<T>&X, const vec<T>&Y)
{
	mat<T> V = cgv::math::vander(n,X);
	mat<T> A;
	vec<T> b,p;
	AtA(V,A);
	Atx(V,Y,b);
	svd_solve(A,b,p);
	return p;
}

}
}