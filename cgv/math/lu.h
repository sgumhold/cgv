#pragma once

#include <cgv/math/mat.h>
#include <cgv/math/perm_mat.h>
#include <cgv/math/low_tri_mat.h>
#include <cgv/math/up_tri_mat.h>
#include <cgv/math/vec.h>
#include <limits>
#include <algorithm>

namespace cgv {
namespace math {

///(P)LU decomposition of a matrix
/// returns false if matrix is singular otherwise a = p*l*u
template <typename T>
bool lu(const mat<T> &a,perm_mat& p, low_tri_mat<T>& l, up_tri_mat<T>& u) 
{

	
	unsigned n = a.nrows();
	unsigned m = a.ncols();
	assert(n==m);
	l.resize(n);
	u.resize(n);
	p.resize(n);
		
	for(unsigned i =0; i < a.ncols();i++)
		for(unsigned j = 0; j < a.nrows();j++)
		{
			if(i > j)
				l(i,j) = a(i,j);
			else
				u(i,j) = a(i,j);
		}
			
	

	const T eps=std::numeric_limits<T>::epsilon();
	unsigned i,imax,j,k;
	T big,temp;
	vec<T> vv(n);
	T d=1.0;

	for (i=0;i<n;i++) 
	{
		big=0.0;
		for (j=0;j<n;j++)
		{
			if(i > j)
			{
				if ((temp=std::abs(l(i,j))) > big) 
					big=temp;
			}
			else
			{
				if ((temp=std::abs(u(i,j))) > big) 
					big=temp;
			}

			
		}
		if (big == 0.0) return false;
		vv[i]=(T)1.0/big;
	}
	for (k=0;k<n;k++) 
	{
		big=0.0;
		for (i=k;i<n;i++) 
		{
			if(i > k)
				temp=vv[i]*std::abs(l(i,k));
			else
				temp=vv[i]*std::abs(u(i,k));

			if (temp > big) 
			{
				big=temp;
				imax=i;
			}
		}
		if (k != imax) 
		{
			for (j=0;j<n;j++) 
			{
				if(imax > j)
				{
					temp=l(imax,j);
					if(k > j)
					{
						l(imax,j)=l(k,j);
						l(k,j)=temp;
					}
					else
					{
						l(imax,j)=u(k,j);
						u(k,j)=temp;
					}

				}
				else
				{
					temp=u(imax,j);
					if(k > j)
					{
						u(imax,j)=l(k,j);
						l(k,j)=temp;
					}
					else
					{
						u(imax,j)=u(k,j);
						u(k,j)=temp;
					}
				}

				
			}
			d = -d;
			vv[imax]=vv[k];
			
		}
		p.swap(imax,k);
		
		if (u(k,k) == 0.0) u(k,k)=eps;
		for (i=k+1;i<n;i++) 
		{
		
			temp=l(i,k) /= u(k,k);
		
			for (j=k+1;j<n;j++)
			{
				if(i > j)
					l(i,j) -= temp*u(k,j);
				else
					u(i,j) -= temp*u(k,j);
			}
		}
	}
	for(unsigned i = 0; i < n; i++)
		{
			
			l(i,i)=(T)1;
			
		}

	
	return true;
}



}

}
