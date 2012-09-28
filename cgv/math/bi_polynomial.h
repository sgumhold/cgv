#pragma once
#include <cgv/math/polynomial.h>

namespace cgv {
	namespace math {


/**
* bivariate polynomials are stored in matrices in the following order:
* [a,b,c,d		a*x1^3*x2^3 + b*x1^2*x2^3 + c*x1*x2^3 + d*x2^3 + 
*  e,f,g,h		e*x1^3*x2^2 + f*x1^2*x2^2 + g*x1*x2^2 + h*x2^2 + 
*  i,j,k,l		i*x1^3*x2   + j*x1^2*x2   + k*x1*x2   + l*x2   + 
*  m,n,o,p]		m*x1^3      + n*x1^2      + o*x1      + p   
*/

/// evaluate a bivariate polynomial p at (x1,x2)
/// p is the coefficients matrix of the bivariate polynomial 
/// evaluation is done by using the horner scheme
template <typename T>
T bipoly_val(const mat<T>& bp, const T& x1, const T& x2)
{
	unsigned n = bp.ncols();
	unsigned m = bp.nrows();
	assert(n > 0);
	assert(m > 0);

	vec<T> r(m);
	for(unsigned i = 0; i < m;i++)
		r(i) = poly_val(bp.row(i),x1);

	return poly_val(r,x2);
}

/// evaluate a bivariate polynomial p at 2d position x
/// p is the coefficients matrix of the bivariate polynomial 
/// evaluation is done by using the horner scheme
template <typename T>
T bipoly_val(const mat<T>& bp, const vec<T>& x)
{
	unsigned n = bp.ncols();
	unsigned m = bp.nrows();
	assert(n > 0);
	assert(m > 0);
	assert(v.size() == 2)

	vec<T> r(n);
	for(unsigned i = 0; i < m;i++)
		r(i) = poly_val(bp.row(i),x(0));

	return poly_val(r,x(1));
}


//returns vandermonde matrix for bivariate polynomial fitting
template <typename T>
cgv::math::mat<T> vander2(unsigned degree1, unsigned degree2,
				  const vec<T>& X1,const vec<T> X2)
{
	assert(X1.size()>0);
	assert(X1.size() == X2.size());
	unsigned n = X1.size();
	unsigned m = (degree1+1)*(degree2+1);
	cgv::math::mat<T> Vxy(n,m);
	
	cgv::math::mat<T> Vx = vander(degree1,X1);
	cgv::math::mat<T> Vy = vander(degree2,X2);
		
	for(unsigned r = 0; r < n; r++)
	{
		for(unsigned i = 0; i < degree2+1; i++)
			for(unsigned j =0; j < degree1+1; j++)
				Vxy(r,j*(degree2+1)+i) = Vy(r,i)*Vx(r,j);
			
	}
	
			
	return Vxy;

}



//fit a bivariate polynomial 
template <typename T>
mat<T> bipoly_fit(unsigned degree1, unsigned degree2,
				  const vec<T>& X1,const vec<T> X2,const vec<T> Y)
{
	
	assert(X1.size() == X2.size());
	assert(X1.size() == Y.size());
	
	
	mat<T> V = cgv::math::vander2(degree1,degree2,X1,X2);
	mat<T> A;
	vec<T> b,bp;
	AtA(V,A);
	Atx(V,Y,b);
	svd_solve(A,b,bp);
		
	return reshape(bp,degree2+1,degree1+1);
	
}


/*
template <typename T>
mat<T> bipoly_fit2(unsigned degree1, unsigned degree2,
				  const vec<T>& X1,const vec<T> X2,const vec<T> Y)
{
	
	assert(X1.size() == X2.size());
	assert(X1.size() == Y.size());
	
	vec<T> C = poly_fit<T>(degree2,X2,Y);
	//std::cout << C << "\n";
	mat<T> B = dyad(cgv::math::ones<T>(X1.size()),C);
	mat<T> V = cgv::math::vander(degree1,X1);
	mat<T> A;
	
	AtA(V,A);
	
	mat<T> D,bp;
	AtB(V,B,D);
	
	svd_solve(A,D,bp);
	//std::cout <<":\n"<< A*bp-D;
		
	return bp;
}*/

////analytic derivation of bivariate polynomial dp(x1,x2)/dx1
template <typename T>
mat<T> bipoly_der_x1(const mat<T>& bp)
{

	assert(bp.ncols() > 0 && bp.nrows() > 0);
	if(bp.ncols() == 1)
		return zeros<T>(1,1);

	mat<T> m(bp.nrows(),bp.ncols()-1);
	for(unsigned i = 0; i < bp.nrows(); i++)
		m.set_row(i,poly_der(bp.row(i)));
	return m;
}

////analytic derivation of bivariate polynomial dp(x1,x2)/dx2
template <typename T>
mat<T> bipoly_der_x2(const mat<T>& bp)
{

	assert(bp.ncols() > 0 && bp.nrows() > 0);
	if(bp.nrows() == 1)
		return zeros<T>(1,1);
	T n = (T)(bp.nrows()-1);

	mat<T> m(bp.nrows()-1,bp.ncols());
	for(unsigned i = 0; i < bp.nrows()-1; i++)
		m.set_row(i,(n-(T)i)*  bp.row(i));
	return m;
}

///analytic integration of bivariate polynomial int p(x1,x2) dx1
template <typename T>
mat<T> bipoly_int_x1(const mat<T>& bp,const T& k=0)
{
	mat<T> m(bp.nrows(),bp.ncols()+1);
	for(unsigned i = 0; i < bp.nrows();i++) 
		m.set_row(i,poly_int(bp.row(i),k));
	return m;
}

///analytic integration of bivariate polynomial int p(x1,x2) dx2
template <typename T>
mat<T> bipoly_int_x2(const mat<T>& bp,const T& k=0)
{
	
	mat<T> v(1,bp.ncols());
	v.zeros();
	v(0,bp.ncols()-1)=k;
	mat<T> m = vertcat(bp,v);
	
	for(unsigned i = 0; i < m.nrows()-2;i++)
		for(unsigned j = 0; j < m.ncols();j++)
			m(i,j)=m(i,j)/(m.nrows()-1-i);

	
	
	return m;
}








	}
}