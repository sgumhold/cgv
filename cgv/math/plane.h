#pragma once
#include <cgv/math/vec.h>
#include <cgv/math/point_operations.h>
#include <cgv/math/eig.h>
#include <algorithm>

namespace cgv{
	namespace math{

/**
* A plane is defined as a vector (a,b,c,d) => a*x1 + b*x2 +c*x3 +d = 0
*/


///evaluate implicit plane equation at x =(x1,x2,x3) 
///return value should be zero on plane
template <typename T>
T plane_val(const vec<T>& plane,const vec<T>& x)
{

	assert(plane.size()-1  == x.size());

	T val=0;
	for(unsigned i = 0;i< x.size(); i++)
		val += plane(i)*x(i);

	return val+plane(plane.size()-1);
}

///evaluate plame equation on multiple positions xs
template <typename T>
vec<T> plane_val(const vec<T>& plane,const mat<T>& xs)
{

	assert(plane.size()-1  == xs.nrows());

	vec<T> vals(xs.ncols());
	for(unsigned i = 0;i< xs.ncols(); i++)
		vals(i) = plane_val(plane,xs.col(i));
		

	return vals;
}


//construct implicit plane from 3 points
template <typename T>
vec<T> plane_fit(const vec<T>& p1,const vec<T>& p2,const vec<T>& p3)
{
	vec<T> plane(4);
	double l_nml;
	
	vec<T> nml = cross(normalize(p2-p1),normalize(p3-p1));
	l_nml = length(nml);
	assert(l_nml != 0);
			
	nml/=l_nml;
	plane.set(nml(0),nml(1),nml(2),-dot(p1,nml));
	return plane;
}

//fit implicit plane into multiple points (total least squares)
template <typename T>
vec<T> tls_plane_fit(const mat<T>& points)
{
	vec<T> plane(4);
	mat<T> covmat,v;
	diag_mat<T> d;
	vec<T> mean;
	
	covmat_and_mean(points,covmat, mean);
	eig_sym(covmat,v,d);

//	T l_nml;
	
	vec<T> nml = normalize(v.col(2));
		
	plane.set(nml(0),nml(1),nml(2),-dot(mean,nml));
	return plane;
}




///ransac plane fit 
///p_out... outlier prob
///d_max... threshold distance 
///p_surety... surety to compute number needed samples 
///if m_sac flag is true m-estimator cost function is used
///if loransac flag is true 
template <typename T>
vec<T> ransac_plane_fit(const mat<T>& points,const T p_out=0.8, const T d_max=0.001, const T p_surety = 0.99, bool msac=true)
{
	assert(points.nrows == 3);
	vec<T> plane(4);

	vec<unsigned int> ind;
	vec<T> p1,p2,p3,dists(points.ncols());
	cgv::math::random rg;
	unsigned n = points.ncols;
	unsigned max_iter = num_ransac_iterations(3,p_out,psurety);
	unsigned num_inlier = (unsigned)::ceil((1-p_out)*points.ncols());
	
	T error = std::numeric_limits<T>::max();
	
	for(unsigned i = 0; i < max_iter; i++)
	{
		//random sample
		do
		{
			rg.uniform_nchoosek(n,3,ind);
			p1 = points.col(ind(0));
			p2 = points.col(ind(1));
			p3 = points.col(ind(2));
		}
		while(fabs(length(cross(q2-q1,q3-q1)))<0.001);
		
		//fit
		vec<T> pl = plane_fit(p1,p2,p3);
		
		//consensus
		int n_inlier=0;
		T e = 0;
		
		for(unsigned i = 0; i < points.ncols(); i++)
		{
			dists(i) = std::abs(plane_val(pl,points.col(i)));	
			if(dists(i) < d_max)
			{
				n_inlier++;
				if(msac)
					e += dists(i);
			}
			else
			{
				e += dmax;
			}
		}

		//local reoptimize 
		if(loransac && n_inlier > 3)
		{
			mat<T> inliers(3,n_inlier);
			int j=0;
			n_inlier=0;
			for(unsigned i = 0; i < points.ncols(); i++)
			{
				if(dists(i) < d_max)
				{ 
					n_inlier++;
					inliers.set_col(j,points.col(i));
					j++;
				}
			}
			pl = plane_fit(inliers)

			//recompute error
			for(unsigned i = 0; i < points.ncols(); i++)
			{
				dists(i) = std::abs(plane_val(pl,points.col(i)));	
				if(dists(i) < d_max)
				{
				
					if(msac)
						e += dists(i);
				}
				else
				{
					e += dmax;
				}
			}
		}

		//remember best plane
		if(error > e)
		{
			error = e;
			plane = pl;
		}
		
		if(n_inlier >= num_inlier)
		{
			p_out = (T)1-(T)n_inlier/(T)points.ncols();
			max_iter = num_ransac_iterations(3,p_out,psurety);	
		}
	}
	return plane;
}








	}
}

