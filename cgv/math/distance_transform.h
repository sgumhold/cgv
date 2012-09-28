#pragma once

#include <cgv/math/vec.h>
#include <cgv/math/mat.h>
#include <cgv/math/functions.h>
#include <limits>

namespace cgv{
	namespace math{

/**
* Implementation of 1d and 2d linear time distance transformation
* see: "Distance Transforms of Sampled Functions
* Pedro F. Felzenszwalb and Daniel P. Huttenlocher"
* for details.
*
*/

template <typename T>
void sqrdist_transf_1d(const vec<T>& f, vec<T>& d) 
{
	static T INF =std::numeric_limits<T>::max();
	unsigned n = f.size();
	d.resize(n);
	int *v = new int[n];
	T *z = new T[n+1];
	int k = 0;
	v[0] = 0;
	z[0] = -INF;
	z[1] = +INF;
  
	for (unsigned q = 1; q <= n-1; q++) 
	{
		T s  = ((f[q]+sqr(q))-(f[v[k]]+sqr(v[k])))/(2*q-2*v[k]);
	    while (s <= z[k]) 
		{
			k--;
			s  = ((f[q]+sqr(q))-(f[v[k]]+sqr(v[k])))/(2*q-2*v[k]);
		}
		k++;
		v[k] = q;
		z[k] = s;
		z[k+1] = +INF;
	}
	k = 0;
	for (unsigned q = 0; q <= n-1; q++) 
	{
		while (z[k+1] < q)
			k++;
		d[q] = sqr(q-v[k]) + f[v[k]];
	}

  delete [] v;
  delete [] z;
}



template <typename T>
void sqrdist_transf_2d(mat<T> &im) 
{
	static T INF =std::numeric_limits<T>::max();
	unsigned width = im.ncols();
	unsigned height = im.nrows();
    
	vec<T> d;

	// transform along columns
	for (unsigned x = 0; x < width; x++) 
	{
	  sqrdist_transf_1d<T>(im.col(x), d);
	  im.set_col(x, d);
	}

	// transform along rows
	for (unsigned y = 0; y < height; y++) 
	{
	   sqrdist_transf_1d<T>(im.row(y), d);
	   im.set_row(y, d);
	}

}

///compute the squared distance transform of an input image 
template <typename T>
void sqrdist_transf_2d(const mat<T>& input,mat<T>& output,T on = 1) 
{
	static T INF =std::numeric_limits<T>::max();
	unsigned width = input.nrows();
	unsigned height = input.ncols();
	output.resize(width,height);
  
	for (unsigned y = 0; y < height; y++) 
	{
		for (unsigned x = 0; x < width; x++) 
		{
			if (input(x,y) == on)
				output(x,y) = 0;
			else
				output(x,y) = INF;
		}
    }
  

  sqrdist_transf_2d<T>(output);
  
}

///compute the  distance transform of an input image 
template <typename T>
void dist_transf_2d(mat<T>& input, mat<T>& output,T on = 1) 
{
	sqrdist_transf_2d(input,output,on );
	for(unsigned y = 0; y < output.ncols();y++)
	{
		for(unsigned x = 0; x < output.nrows();x++)
		{
			output(x,y)=sqrt(output(x,y));
		}

	}
  
}

	}
}
