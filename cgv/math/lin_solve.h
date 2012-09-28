#pragma once
#include <cgv/math/vec.h>
#include <cgv/math/mat.h>
#include <cgv/math/perm_mat.h>
#include <cgv/math/diag_mat.h>
#include <cgv/math/tri_diag_mat.h>
#include <cgv/math/up_tri_mat.h>
#include <cgv/math/low_tri_mat.h>
#include <cgv/math/lu.h>
#include <cgv/math/svd.h>
#include <cgv/math/qr.h>


namespace cgv {
	namespace math {

///solves linear system ax=b
///a is an upper triangular matrix
template<typename T>
bool solve(const up_tri_mat<T>& a,const vec<T>&b, vec<T>&x) 
{
	assert(a.nrows() == a.ncols());

	int N = a.nrows();
	x.resize(N);
	T sum;
	for(int i = N-1; i >= 0;i--)
	{
		sum =0;
		for(int j = i+1;j < N; j++) 
			sum += a(i,j)*x(j);
		if(a(i,i) == 0)
			return false;
		x[i] = (b[i] - sum)/a(i,i);
	}
	return true;	
}

///solves multiple linear systems ax=b 
///a is an upper triangular matrix
///x is the matrix of solution vectors (columns)
///b is the matrix of right-hand sides (columns)
template<typename T>
bool solve(const up_tri_mat<T>& a,const mat<T>&b,mat<T>&x) 
{
	assert(b.nrows() == a.ncols());
	vec<T> xcol;
	x.resize(b.nrows(),b.ncols());
	for(unsigned i = 0; i < b.ncols();i++)
	{
		if(!solve(a,b.col(i),xcol))
			return false;
		x.set_col(i,xcol);
	}
	return true;
}

///solves linear system ax=b
///a is a lower triangular matrix
template<typename T>
bool solve(const low_tri_mat<T>& a, const vec<T>&b, vec<T>&x) 
{

	int N = a.nrows();
	x.resize(N);
	T sum;
	for(int i = 0; i < N;i++)
	{
		sum =0;
		for(int j = 0;j < i; j++) 
			sum += a(i,j)*x(j);
		if(a(i,i) == 0)
			return false;
		x[i] = (b[i] - sum)/a(i,i);
	}
	return true;
	
}

///solves multiple linear systems ax=b 
///a is a lower triangular matrix
///x is the matrix of solution vectors (columns)
///b is the matrix of right-hand sides (columns)
template<typename T>
bool solve(const low_tri_mat<T>& a,const mat<T>&b,mat<T>&x) 
{
	assert(b.nrows() == a.ncols());
	vec<T> xcol;
	x.resize(b.nrows(),b.ncols());
	for(unsigned i = 0; i < b.ncols();i++)
	{
		if(!solve(a,b.col(i),xcol))
			return false;
		x.set_col(i,xcol);
	}
	return true;
}

///solves linear system ax=b
///a is a diagonal matrix
template<typename T>
bool solve(const diag_mat<T>& a, const vec<T>&b, vec<T>&x) 
{
	int N = a.ncols();
	x.resize(N);
	for(int i = 0; i < N;i++)
	{
		if(a(i) == 0)
			return false;
		x(i) = (T)b(i)/a(i);
	}
	return true;
}

///solves linear system ax=b
///a is a tri diagonal matrix
template <typename T>
bool solve(const tri_diag_mat<T>& a, const vec<T>& b, vec<T>& x)
{
	x.resize(b.dim());
	int i;
	vec<T> aa = a.band(-1);
	vec<T> bb = a.band(0);
	vec<T> cc = a.band(1);
	vec<T> dd = b;
	
	int n = b.dim();
	
	if(bb(0) == 0)
		return false;
	cc(0) /= bb(0);
	dd(0) /= bb(0);
	for(i = 1; i < n; i++)
	{
		T id = (bb(i) - cc(i-1) * aa(i));
		if(id == 0)
			return false;
		cc(i) /= id;			
		dd(i) = (dd(i) - dd(i-1) * aa(i))/id;
	}
 
	x(n - 1) = dd(n - 1);
	for(i = n - 2; i >= 0; i--)
		x(i) = dd(i) - cc(i) * x(i + 1);
	return true;
}

///solves multiple linear systems ax=b 
///a is a diagonal matrix
///x is the matrix of solution vectors (columns)
///b is the matrix of right-hand sides (columns)
template<typename T>
bool solve(const diag_mat<T>& a, const mat<T>&b, mat<T>&x) 
{
	assert(b.nrows() == a.ncols());
	vec<T> xcol;
	x.resize(b.nrows(),b.ncols());
	for(unsigned i = 0; i < b.ncols();i++)
	{
		if(!solve(a,b.col(i),xcol))
			return false;
		x.set_col(i,xcol);
	}
	return true;
}

///solves linear system ax=b
///a is a permutation matrix
template<typename T>
bool solve(const perm_mat &a, const vec<T> &b, vec<T>&x)
{

	x.resize(a.nrows());
	x=transpose(a)*b;
	return true;
}

///solves multiple linear systems ax=b 
///a is permutation matrix
///x is the matrix of solution vectors (columns)
///b is the matrix of right-hand sides (columns)
template<typename T>
bool solve(const perm_mat& a,const mat<T>&b,mat<T>&x) 
{
	assert(a.nrows() == a.ncols());
	vec<T> xcol;
	x.resize(b.nrows(),b.ncols());
	for(unsigned i = 0; i < b.ncols();i++)
	{
		if(!solve(a,b.col(i),xcol))
			return false;
		x.set_col(i,xcol);
	}
	return true;
}




///solve ax=b with lu decomposition
///a is a full storage matrix
template<typename T>
bool lu_solve(const mat<T>& a, const vec<T>&b, vec<T>&x) 
{
	assert(a.nrows() == a.ncols());
	x.resize(a.nrows());
	vec<T> temp1,temp2;
	low_tri_mat<T> L;
	up_tri_mat<T> U;
	perm_mat P;
	if(!lu(a,P,L,U))
		return false;
	if(!solve(P,b,temp1))
		return false;
	if(!solve(L,temp1,temp2))
		return false;
	return solve(U,temp2,x);		
}

///solve ax=b, standard solver for full storage matrix is lu_solve
///a is a full storage matrix
template<typename T>
bool solve(const mat<T>& a, const vec<T>&b, vec<T>&x) 
{
	return lu_solve( a, b, x) ;
}

///solves multiple linear systems ax=b 
///a is full storage matrix
///x is the matrix of solution vectors (columns)
///b is the matrix of right-hand sides (columns)
template<typename T>
bool lu_solve(const mat<T>& a,const mat<T>&b,mat<T>&x) 
{
	assert(a.nrows() == a.ncols());
	x.resize(b.nrows(),b.ncols());
	mat<T> temp1,temp2;
	low_tri_mat<T> L;
	up_tri_mat<T> U;
	perm_mat P;
	if(!lu(a,P,L,U))
		return false;
	if(!solve(P,b,temp1))
		return false;
	if(!solve(L,temp1,temp2))
		return false;
	return solve(U,temp2,x);
	
}

///solves multiple linear systems  ax=b with the svd solver 
///a is full storage matrix
///x is the matrix of solution vectors (columns)
///b is the matrix of right-hand sides (columns)
template<typename T>
bool solve(const mat<T>& a, const mat<T>&b, mat<T>&x) 
{
	return svd_solve( a, b, x) ;
}



///solve ax=b with qr decomposition
template<typename T>
bool qr_solve(const mat<T>& a, const vec<T>&b, vec<T>&x) 
{
	x.resize(a.nrows());
	vec<T> temp;
	mat<T> q;
	up_tri_mat<T> r;
	if(!qr(a,q,r))
		return false;
	Atx(q,b,temp);
	return solve(r,temp,x);
}


///solves multiple linear systems  ax=b with qr solver 
///a is full storage matrix
///x is the matrix of solution vectors (columns)
///b is the matrix of right-hand sides (columns)
template<typename T>
bool qr_solve(const mat<T>& a, const mat<T>&b, mat<T>&x) 
{
	assert(a.nrows() == a.ncols());
	x.resize(b.nrows(),b.ncols());
	mat<T> temp;
	mat<T> q;
	up_tri_mat<T> r;
	if(!qr(a,q,r))
		return false;
	AtB(q,b,temp);
	return solve(r,temp,x);
}



///solve ax=b with svd decomposition
template<typename T>
bool svd_solve(const mat<T>& a, const vec<T>&b, vec<T>&x) 
{
	x.resize(a.nrows());
	vec<T> temp;
	mat<T> u,v;
	diag_mat<T> d;
	if(!svd(a,u,d,v))
		return false;
	  
	Atx(u,b,temp);
	if(!solve(d,temp,x))
		return false;
	x=v*x;
	return true;
}

///solve ax=b with svd decomposition
template<typename T>
bool svd_solve(const mat<T>& a, const mat<T>&b, mat<T>&x) 
{
	assert(a.nrows() == a.ncols());
	x.resize(b.nrows(),b.ncols());
	
	mat<T> temp;
	mat<T> u,v;
	diag_mat<T> d;
	if(!svd(a,u,d,v))
		return false;  
	AtB(u,b,temp);
	if(!solve(d,temp,x))
		return false;
	x=v*x;
	return true;
}






}


}