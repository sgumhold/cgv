#pragma once

#include "mat.h"
#include "diag_mat.h"
#include "functions.h"

#include <limits>
#include <algorithm>

namespace cgv {
namespace math {

//helper funcition for svd
template <typename T>
T pythag(const T a, const T b) {
		T absa=std::abs(a), absb=std::abs(b);
		return T((absa > absb ? absa*sqrt(1.0+(absb/absa)*(absb/absa)) :
			(absb == 0.0 ? 0.0 : absb*sqrt(1.0+(absa/absb)*(absa/absb)))));
	}

///computes the singular value decomposition of an MxN matrix a = u* w * v^t where u is an MxN matrix, w is a diagonal
///NxN matrix and v is a NxN square matrix. If the algorithm can't achieve a convergence after 20 iteration false is returned other wise true.
///The resulting matrices are stored in the parameters u,w,v. Attention: v is returned not v^T ! So to compute the original matrix a from the 
///decomposed matrices you have to multiply u*w*transpose(v). 
///It is possible to store u directly into a to save memory, just put the same reference into a and u.
///If ordering is true the singular values are sorted in descending order.
///To ensure that u*w*v^t remains equal to the matrix a the algorithm also exchanges the columns of u and v. 
template <typename T>
bool svd(const mat<T> &a, mat<T> &u, diag_mat<T> &w,  mat<T> &v,bool ordering=true, int maxiter=30)
{
	int m = a.nrows();
	int n = a.ncols();
	
	u=a;
	v.resize(n,n);
	w.resize(n);
	T eps=std::numeric_limits<T>::epsilon();
	//decompose
	bool flag;
		int i,its,j,jj,k,l,nm;
		T anorm,c,f,g,h,s,scale,x,y,z;
		vec<T> rv1(n);
		g = scale = anorm = 0.0;
		for (i=0;i<n;i++) {
			l=i+2;
			rv1[i]=scale*g;
			g=s=scale=0.0;
			if (i < m) {
				for (k=i;k<m;k++) scale += std::abs(u(k,i));
				if (scale != 0.0) {
					for (k=i;k<m;k++) {
						u(k,i) /= scale;
						s += u(k,i)*u(k,i);
					}
					f=u(i,i);
					g = -sign(std::sqrt(s),f);
					h=f*g-s;
					u(i,i)=f-g;
					for (j=l-1;j<n;j++) {
						for (s=0.0,k=i;k<m;k++) s += u(k,i)*u(k,j);
						f=s/h;
						for (k=i;k<m;k++) u(k,j) += f*u(k,i);
					}
					for (k=i;k<m;k++) u(k,i) *= scale;
				}
			}
			w[i]=scale *g;
			g=s=scale=0.0;
			if (i+1 <= m && i+1 != n) {
				for (k=l-1;k<n;k++) scale += std::abs(u(i,k));
				if (scale != 0.0) {
					for (k=l-1;k<n;k++) {
						u(i,k) /= scale;
						s += u(i,k)*u(i,k);
					}
					f=u(i,l-1);
					g = -sign(std::sqrt(s),f);
					h=f*g-s;
					u(i,l-1)=f-g;
					for (k=l-1;k<n;k++) rv1[k]=u(i,k)/h;
					for (j=l-1;j<m;j++) {
						for (s=0.0,k=l-1;k<n;k++) s += u(j,k)*u(i,k);
						for (k=l-1;k<n;k++) u(j,k) += s*rv1[k];
					}
					for (k=l-1;k<n;k++) u(i,k) *= scale;
				}
			}
			anorm=(std::max)(anorm,(std::abs(w[i])+std::abs(rv1[i])));
		}
		for (i=n-1;i>=0;i--) {
			if (i < n-1) {
				if (g != 0.0) {
					for (j=l;j<n;j++)
						v(j,i)=(u(i,j)/u(i,l))/g;
					for (j=l;j<n;j++) {
						for (s=0.0,k=l;k<n;k++) s += u(i,k)*v(k,j);
						for (k=l;k<n;k++) v(k,j) += s*v(k,i);
					}
				}
				for (j=l;j<n;j++) v(i,j)=v(j,i)=0.0;
			}
			v(i,i)=1.0;
			g=rv1[i];
			l=i;
		}
		for (i=(std::min)(m,n)-1;i>=0;i--) {
			l=i+1;
			g=w[i];
			for (j=l;j<n;j++) u(i,j)=0.0;
			if (g != 0.0) {
				g=(T)1.0/g;
				for (j=l;j<n;j++) {
					for (s=0.0,k=l;k<m;k++) s += u(k,i)*u(k,j);
					f=(s/u(i,i))*g;
					for (k=i;k<m;k++) u(k,j) += f*u(k,i);
				}
				for (j=i;j<m;j++) u(j,i) *= g;
			} else for (j=i;j<m;j++) u(j,i)=0.0;
			++u(i,i);
		}
		for (k=n-1;k>=0;k--) {
			for (its=0;its<maxiter;its++) {
				flag=true;
				for (l=k;l>=0;l--) {
					nm=l-1;
					if (l == 0 || std::abs(rv1[l]) <= eps*anorm) {
						flag=false;
						break;
					}
					if (std::abs(w[nm]) <= eps*anorm) break;
				}
				if (flag) {
					c=0.0;
					s=1.0;
					for (i=l;i<k+1;i++) {
						f=s*rv1[i];
						rv1[i]=c*rv1[i];
						if (std::abs(f) <= eps*anorm) break;
						g=w[i];
						h=pythag(f,g);
						w[i]=h;
						h=(T)1.0/h;
						c=g*h;
						s = -f*h;
						for (j=0;j<m;j++) {
							y=u(j,nm);
							z=u(j,i);
							u(j,nm)=y*c+z*s;
							u(j,i)=z*c-y*s;
						}
					}
				}
				z=w[k];
				if (l == k) {
					if (z < 0.0) {
						w[k] = -z;
						for (j=0;j<n;j++) v(j,k) = -v(j,k);
					}
					break;
				}
				if (its == maxiter-1)
					return false;
				x=w[l];
				nm=k-1;
				y=w[nm];
				g=rv1[nm];
				h=rv1[k];
				f=((y-z)*(y+z)+(g-h)*(g+h))/((T)2.0*h*y);
				g=pythag(f,(T)1.0);
				f=((x-z)*(x+z)+h*((y/(f+sign(g,f)))-h))/x;
				c=s=1.0;
				for (j=l;j<=nm;j++) {
					i=j+1;
					g=rv1[i];
					y=w[i];
					h=s*g;
					g=c*g;
					z=pythag(f,h);
					rv1[j]=z;
					c=f/z;
					s=h/z;
					f=x*c+g*s;
					g=g*c-x*s;
					h=y*s;
					y *= c;
					for (jj=0;jj<n;jj++) {
						x=v(jj,j);
						z=v(jj,i);
						v(jj,j)=x*c+z*s;
						v(jj,i)=z*c-x*s;
					}
					z=pythag(f,h);
					w[j]=z;
					if (z) {
						z=(T)1.0/z;
						c=f*z;
						s=h*z;
					}
					f=c*g+s*y;
					x=c*y-s*g;
					for (jj=0;jj<m;jj++) {
						y=u(jj,j);
						z=u(jj,i);
						u(jj,j)=y*c+z*s;
						u(jj,i)=z*c-y*s;
					}
				}
				rv1[l]=0.0;
				rv1[k]=f;
				w[k]=x;
			}
		}
		if(ordering)
		{
			int i,j,k,s,inc=1;
		T sw;
		vec<T> su(m), sv(n);
		do { inc *= 3; inc++; } while (inc <= n);
		do {
			inc /= 3;
			for (i=inc;i<n;i++) {
				sw = w[i];
				for (k=0;k<m;k++) su[k] = u(k,i);
				for (k=0;k<n;k++) sv[k] = v(k,i);
				j = i;
				while (w[j-inc] < sw) {
					w[j] = w[j-inc];
					for (k=0;k<m;k++) u(k,j) = u(k,j-inc);
					for (k=0;k<n;k++) v(k,j) = v(k,j-inc);
					j -= inc;
					if (j < inc) break;
				}
				w[j] = sw;
				for (k=0;k<m;k++) u(k,j) = su[k];
				for (k=0;k<n;k++) v(k,j) = sv[k];

			}
		} while (inc > 1);
		for (k=0;k<n;k++) {
			s=0;
			for (i=0;i<m;i++) if (u(i,k) < 0.) s++;
			for (j=0;j<n;j++) if (v(j,k) < 0.) s++;
			if (s > (m+n)/2) {
				for (i=0;i<m;i++) u(i,k) = -u(i,k);
				for (j=0;j<n;j++) v(j,k) = -v(j,k);
			}
		}
	}

	T tsh = (T)(0.5*sqrt(m+n+1.)*w[0]*eps);
	return true;
	


	}

///compute the null space of a matrix
template <typename T>
mat<T> null(const mat<T>& a)
{
	T eps =std::numeric_limits<T>::epsilon();
	mat<T> u,v;
	diag_mat<T> s;
	svd(a,u,s,v);
	
	unsigned r = 0;
	T tol;
	if(a.nrows() > a.ncols())
		tol = (T)a.nrows()*eps;
	else
		tol = (T)a.ncols()*eps;
	
	for(unsigned i = 0; i < s.ncols(); i++)
	{
		if(s(i) > tol)
			r++;
	}
	if (r < a.ncols())
	{
      
		mat<T> N = v.sub_mat(0, r,v.nrows(),a.ncols()-r);
		for(unsigned i = 0; i < N.nrows();i++)
			for(unsigned j = 0; j < N.ncols();j++)
				if(std::abs(N(i,j)) < eps) N(i,j)=(T)0;
		
		return N;
	}
    else
      return zeros<T> (a.ncols(), 0);

}

template <typename T>
mat<T> pseudo_inv(const mat<T>& a, T eps = std::numeric_limits<T>::epsilon())
	{
		mat<T> u,v;
		diag_mat<T> s;
		svd(a,u,s,v);
		for(unsigned i = 0; i < s.ncols(); i++)
		{
			if(s(i) > eps)
				s(i)=1/s(i);
			else
				s(i) = 0;
		}
		return v*s*transpose(u);
		
	}


///compute the effective null space of a matrix using user defined tolerance
template <typename T>
mat<T> null(const mat<T>& a, T tol)
{
	T eps =std::numeric_limits<T>::epsilon();
	mat<T> u,v;
	diag_mat<T> s;
	svd(a,u,s,v);
	unsigned r = 0;
	
	
	for(unsigned i = 0; i < s.ncols(); i++)
	{
		if(s(i) > tol)
			r++;
	}
	if (r < a.ncols())
	{
      
		mat<T> N = v.submat(0, r,v.nrows(),a.ncols()-r);
		for(unsigned i = 0; i < N.nrows();i++)
			for(unsigned j = 0; j < N.ncols();j++)
				if(std::abs(N(i,j)) < eps) N(i,j)=(T)0;
		
		return N;
	}
    else
      return zeros<T> (a.ncols(), 0);

}


///computes the rank of a matrix using svd
template<typename T>
unsigned rank(const mat<T>& a)
{
	mat<T> u,v;
	diag_mat<T> s;
	svd(a,u,s,v);
	unsigned r = 0;
	T tol;
	if(a.nrows() > a.ncols())
		tol = (T)a.nrows()*std::numeric_limits<T>::epsilon();
	else
		tol = (T)a.ncols()*std::numeric_limits<T>::epsilon();
	
	for(unsigned i = 0; i < s.ncols(); i++)
	{
		if(s(i) > tol)
			r++;
	}
	return r;
}

///computes the effective rank of a matrix using svd and the given tolerance tol
template<typename T>
unsigned rank(const mat<T>& a, T tol)
{
	mat<T> u,v;
	diag_mat<T> s;
	svd(a,u,s,v);
	unsigned rank = 0;
		
	for(unsigned i = 0; i < s.ncols(); i++)
	{
		if(s(i) > tol)
			rank++;
	}
	return rank;
}


///return the rank of m x n matrix A  with m < n
/// the solution of the system Ax=b is given as x= p + N *(l_1,...,l_{n-r})^T, r is the rank of A, are the free parameters and N is the nullspace 
template <typename T>
unsigned solve_underdetermined_system(const cgv::math::mat<T>& A, const cgv::math::vec<T>& b, 
									  cgv::math::vec<T>& p, cgv::math::mat<T>& N)
{
	T eps =std::numeric_limits<T>::epsilon();
	mat<T> u,v;
	diag_mat<T> s;
	svd(A,u,s,v);
	//compute rank
	unsigned r = 0;
	T tol;
	if(A.nrows() > A.ncols())
		tol = (T)A.nrows()*eps;
	else
		tol = (T)A.ncols()*eps;

	for(unsigned i = 0; i < s.ncols(); i++)
	{
		if(s(i) > tol)
			r++;
		else
			s(i)=0;
	}
	p=v*s*transpose(u)*b;
	if (r < A.ncols())
	{
      
		N = v.sub_mat(0, r,v.nrows(),A.ncols()-r);
		for(unsigned i = 0; i < N.nrows();i++)
			for(unsigned j = 0; j < N.ncols();j++)
				if(std::abs(N(i,j)) < eps) N(i,j)=(T)0;
		
		return r;
	}
	else
	{
		N = zeros<T> (A.ncols(), 0);
		return r;
	}
}








}

}
