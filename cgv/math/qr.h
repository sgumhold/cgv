#pragma once

#include "mat.h"
#include "det.h"
#include "functions.h"
#include "up_tri_mat.h"

#include <limits>
#include <algorithm>

namespace cgv {
namespace math {

//qr decomposition returns false if matrix is singular
template<typename T>
bool qr(const mat<T> &a, mat<T> &q, up_tri_mat<T> &r) 
{
	mat<T> rr = a;
	
	bool sing =false;
	unsigned n = a.nrows();
	unsigned m = a.ncols();
	assert(n == m);
	q.resize(n,n);
	r.resize(n);
	unsigned i,j,k;
	vec<T> c(n), d(n);
	T scale,sigma,sum,tau;
	for (k=0;k<n-1;k++) 
	{
		scale=0.0;
		for (i=k;i<n;i++) 
			scale=std::max((T)scale,(T)std::abs(rr(i,k)));
		if (scale == 0.0) 
		{
			sing=true;
			c[k]=d[k]=0.0;
		} else {
			for (i=k;i<n;i++) rr(i,k) /= scale;
			for (sum=0.0,i=k;i<n;i++) sum += rr(i,k)*rr(i,k);
			sigma=sign((T)sqrt(sum),(T)rr(k,k));
			rr(k,k) += sigma;
			c[k]=sigma*rr(k,k);
			d[k] = -scale*sigma;
			for (j=k+1;j<n;j++) {
				for (sum=0.0,i=k;i<n;i++) 
					sum += rr(i,k)*rr(i,j);
				tau=sum/c[k];
				for (i=k;i<n;i++) 
					rr(i,j) -= tau*rr(i,k);
			}
		}
	}
	d[n-1]=rr(n-1,n-1);
	if (d[n-1] == 0.0) 
		sing=true;
	for (i=0;i<n;i++) {
		for (j=0;j<n;j++) q(i,j)=0.0;
		q(i,i)=1.0;
	}
	for (k=0;k<n-1;k++) 
	{
		if (c[k] != 0.0) 
		{
			for (j=0;j<n;j++) 
			{
				sum=0.0;
				for (i=k;i<n;i++)
					sum += rr(i,k)*q(j,i);
				sum /= c[k];
				for (i=k;i<n;i++)
					q(j,i) -= sum*rr(i,k);
			}
		}
	}
	for (i=0;i<n;i++) 
	{
		rr(i,i)=d[i];
		for (j=0;j<i;j++) rr(i,j)=0.0;
	}
	for(unsigned i = 0; i < n;i++)
		for(unsigned j = i; j < n; j++)
			r(i,j)=rr(i,j);
	return !sing;
}


//qr decomposition by modified gram schmidt 
template<typename T>
bool qr_mgs(const mat<T> &a, mat<T> &q, up_tri_mat<T> &r) 
{
	unsigned n = a.nrows();
	unsigned m = a.ncols();
	assert(n == m);
	q.resize(n,n);
	r.resize(n);

	for(unsigned j = 0; j < m; j++)
	{	
		vec<T> v = a.col(j);
		for(unsigned i = 0; i < j; i++)
		{
			r(i,j) = dot(q.col(i),v);
			v= v- r(i,j)*q.col(i);
		}
		r(j,j) = length(v);
		if(std::abs(r(j,j)) == 0)
			return false;
		q.set_col(j, v/r(j,j));
		
	}
	return true;
}







// rq decomposition using tricky column and row permutations
template <typename T>
bool rq(const mat<T> &a, up_tri_mat<T> &r, mat<T> &q)
{
	unsigned N = a.nrows();
	r.resize(N);
	q.resize(N,N);
	mat<T> p=transpose(a);
	p.fliplr();
	p.flipud();
	if(!qr(p,q,r)) return false;
	mat<T> rr =r;
	rr.transpose();
	rr.fliplr();
	rr.flipud();
	
	q.transpose();
	q.fliplr();
	q.flipud();
	
	if (det(q)<0)
	{

		rr.set_col(0,-rr.col(0));
		q.set_row(0,-q.row(0));
		
	}
	
	for(unsigned i = 0; i< r.nrows();i++)
		for(unsigned j = i; j< r.ncols();j++)
			r(i,j)=rr(i,j);

	return true;
}







}

}
