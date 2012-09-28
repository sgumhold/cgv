#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


#include "vec.h"
#include "mat.h"
#include "transformations.h"
#include <limits>
#include <vector>
#include <algorithm>



namespace cgv {
namespace math {



///computes the mean of all column vectors
template<typename T>
inline vec<T> mean(const mat<T>& points)
{
	unsigned N = points.ncols();
	unsigned M = points.nrows();
	vec<T> mu;
	mu.zeros(M);
	

	for(unsigned j = 0; j < N;j++)
	{
		for(unsigned i = 0; i < M; i++)
		{
			mu(i)+=points(i,j);
		}
	}
	mu/=(T)N;
	return mu;
}


///returns the geometric median of a set of points
///
template <typename T>
cgv::math::vec<T> geometric_median(const cgv::math::mat<T>& points, const T& eps=0.0001, const unsigned max_iter=100)
{
	unsigned m = points.cols();
	unsigned n = points.rows();
	
	cgv::math::vec<T> y(n);
	cgv::math::vec<T> ytemp(n);

	
	for(unsigned iter = 0; iter < max_iter; iter++)
	{
		ytemp.zeros();
		T W =0;
		for(unsigned j = 0;j < m; j++)
		{
			T invdist = length(points.col(j) - y);
			if(invdist != 0)
				invdist=1.0/invdist;
			
			W += invdist;
			ytemp += invdist*points.col(j);
		}
		ytemp = ytemp/W;
		if(length(y-ytemp) < eps)
			return ytemp;
		else
			y=ytemp;	
	}
	return y;
}

///computes the weighted mean of all column vectors
template<typename T>
inline vec<T> weighted_mean(const vec<T>& weights,const mat<T>& points)
{
	assert(weights.dim() == points.ncols());
	unsigned N = points.ncols();
	unsigned M = points.nrows();
	vec<T> mu;
	mu.zeros(M);
	
	T sm =0;
	for(unsigned j = 0; j < N;j++)
	{
		for(unsigned i = 0; i < M; i++)
		{
			mu(i)+=weights(j)*points(i,j);
		}
		sm+=weights(j);
	}
	mu/=sm;
	return mu;
}





///compute covariance matrix of the column vectors of points 
template <typename T>
inline mat<T> covmat(const mat<T>& points)
{
	unsigned N = points.ncols();
	unsigned M = points.nrows();
	mat<T> covmat;
	covmat.zeroes(M,M);
	vec<T> m;
	m.zeros(M);
	
	
	for(unsigned c = 0; c < N;c++)
	{
		for(unsigned i = 0; i < M; i++)
		{
			for(unsigned j = 0; j < M; j++)
			{
				covmat(i,j)+=points(i,c)*points(j,c);			
			}
			m(i)+=points(i,c);		
		}
	}
	m/=(T)N;
	covmat-=((T)N)*dyad(m,m);
	covmat/=(T)N;
	
	
	return covmat;
}

///compute weighted covariance matrix of the column vectors of points 
template <typename T>
inline mat<T> weighted_covmat(const vec<T>& weights,const mat<T>& points)
{
	unsigned N = points.ncols();
	unsigned M = points.nrows();
	mat<T> wcovmat;
	wcovmat.zeros(M,M);
	vec<T> wmean(M);
	wmean.zeros();
	T sumsqrweights=0;
	T sumweights=0;
	
	for(unsigned i = 0; i < N;i++)
	{
		sumweights+=weights(i);
	}
	vec<T> wn=weights/sumweights;
	for(unsigned c = 0; c < N;c++)
	{
		for(unsigned i = 0; i < M;i++)
			wmean(i)+=wn(c)*points.col(i,c);
		sumsqrweights += wn(c)*wn(c);
	}



	for(unsigned c = 0; c < N;c++)
	{
		for(unsigned i = 0; i < M; i++)
		{
			for(unsigned j = 0; j < M; j++)
			{
				wcovmat(i,j)+=wn(c)*(points(i,c)-wmean(i))*(points(j,c)-wmean(j));			
			}
				
		}
	}
	wcovmat/=((T)1.0-sumsqrweights);
	

	
	return wcovmat;
}


///compute covariance matrix and mean of the column vectors of points in one step
template <typename T>
inline void weighted_covmat_and_mean(const vec<T>& weights,const mat<T>& points, mat<T>&wcovmat, vec<T>& wmean)
{
	unsigned N = points.ncols();
	unsigned M = points.nrows();
	wcovmat.zeros(M,M);
	wmean.resize(M);
	wmean.zeros();
	

	T sumsqrweights=0;
	T sumweights=0;
	
	for(unsigned i = 0; i < N;i++)
	{
		sumweights+=weights(i);
	}
	vec<T> wn=weights/sumweights;
	for(unsigned c = 0; c < N;c++)
	{
		for(unsigned i = 0; i < M;i++)
			wmean(i)+=wn(c)*points(i,c);
		sumsqrweights += wn(c)*wn(c);
	}



	for(unsigned c = 0; c < N;c++)
	{
		for(unsigned i = 0; i < M; i++)
		{
			for(unsigned j = 0; j < M; j++)
			{
				wcovmat(i,j)+=wn(c)*(points(i,c)-wmean(i))*(points(j,c)-wmean(j));			
			}
				
		}
	}
	wcovmat/=((T)1.0-sumsqrweights);
	
}

///compute covariance matrix and mean of the column vectors of points in one step
template <typename T>
inline void covmat_and_mean(const mat<T>& points, mat<T>&covmat, vec<T>& mean)
{
	unsigned N = points.ncols();
	unsigned M = points.nrows();
	covmat.zeros(M,M);
	mean.resize(M);
	mean.zeros();
	

	for(unsigned c = 0; c < N;c++)
	{
		for(unsigned i = 0; i < M; i++)
		{
			for(unsigned j = 0; j < M; j++)
			{
				covmat(i,j)+=points(i,c)*points(j,c);			
			}
			mean(i)+=points(i,c);		
		}
	}
	mean/=(T)N;
	covmat-=((T)N)*dyad(mean,mean);
	covmat/=(T)N;
	
}

// compute the joint mean of N1+N2 datapoints from mean1 of N1 datapoint and mean2 of N2 datapoints
template <typename T>
inline vec<T> joint_mean(const int N1, const vec<T>& mean1, 
				  const int N2, const vec<T>& mean2)
{
	T a = (T)N1/(T)(N2+N1);
	T b = (T)N2/(T)(N2+N1);
	return a*mean1+b*mean2;
}

//compute the joint mean and joint covariance matrix of N1+N2 datapoints 
//from  mean1 and covmat1 of N1 data points and mean2 and covmat2 of N2 data points
template <typename T>
inline void joint_mean_and_covmat(const int N1, const vec<T>& mean1,const mat<T> &covmat1,
				           const int N2, const vec<T>& mean2, const mat<T>&covmat2,
						   int&  jointN, vec<T>& jointmean, mat<T>& jointcovmat)
{

	jointN = N2+N1;
	T a = (T)N1/(T)(jointN);
	T b = (T)N2/(T)(jointN);
	jointmean = a*mean1+b*mean2;

	jointcovmat = N1*covmat1+N1*dyad(mean1,mean1) + N2*covmat2+N2*dyad(mean2,mean2);
	jointcovmat -= ((T)jointN)*dyad(jointmean,jointmean);
	jointcovmat /= (T)jointN;
	
}





//finds componentwise minimum vector of all column vectors in the matrix points
template<typename T>
inline vec<T> minimum(const mat<T>& points)
{
	unsigned N = points.ncols();
	unsigned M = points.nrows();
	vec<T> _min(M);
	_min.fill(std::numeric_limits<T>::max());
	
	for(unsigned j = 0; j < N;j++)
	{
		for(unsigned i = 0; i < M; i++)
		{
			_min(i) = _min(i) < points(i,j) ? _min(i) : points(i,j);
		}
	}
	
	return _min;
}

//finds componentwise maximum vector of all column vectors in the matrix points
template<typename T>
inline vec<T> maximum(const mat<T>& points)
{
	unsigned N = points.ncols();
	unsigned M = points.nrows();
	vec<T> _max(M);
	_max.fill(1-std::numeric_limits<T>::max());
	
	for(unsigned j = 0; j < N;j++)
	{
		for(unsigned i = 0; i < M; i++)
		{
			_max(i) = _max(i) > points(i,j) ? _max(i) : points(i,j);
		}
	}
	
	return _max;
}


///swap second with third row of point matrix
template <typename T>
inline void swap_XYZ_2_XZY(mat<T>& points)
{
	for(unsigned i = 0; i < points.ncols();i++)
	{
		T v = points(1,i);
		points(1,i) =points(2,i);
		points(2,i) = v;	
	}
}

//convert a non-homogeneous vector into a homogeneous vector 
template<typename T>
inline void homog(vec<T>& p)
{
	vec<T> t(p.size()+1);
	for(unsigned i = 0; i < p.size();i++)
	{
		t(i)=p(i);
	}
	t(p.size())=(T)1;
	p=t;
}

//convert a homogeneous vector into a non-homogeneous vector 
template<typename T>
inline void unhomog(vec<T>& p)
{
	unsigned N=p.size()-1;
	vec<T> temp(N);
	
	for(unsigned i = 0; i < N;i++)
	{
		temp(i)=p(i)/p(N);
	}
	
	p=temp;
}

///Convert a set of non-homogeneous points into homogeneous points
template<typename T>
inline void homog(mat<T>& points)
{
	mat<T> temp(points.nrows()+1,points.ncols());
	for(unsigned j = 0; j < points.ncols(); j++)
	{
		for(unsigned i = 0; i < points.nrows();i++)
		{
			temp(i,j)=points(i,j);
		}
		temp(points.nrows(),j)=(T)1;
	}
	points=temp;
}

///Convert a set of homogeneous points into non-homogeneous points by dividing with the last component
template<typename T>
inline void unhomog(mat<T>& points)
{
	unsigned N=points.nrows()-1;
	mat<T> temp(N,points.ncols());
	for(unsigned j = 0; j < points.ncols(); j++)
	{
		for(unsigned i = 0; i < N;i++)
		{
			temp(i,j)=points(i,j)/points(N,j);
		}
	}
	points=temp;
}


template<typename T>
inline void center_and_scale(mat<T>& points,const T& sf=10)
{
	if(points.ncols() > 0)
	{
		assert(points.nrows() == 3);
		vec<T> minv =minimum(points);
		vec<T> maxv =maximum(points);
		vec<T> t = vec<T>((maxv(0)+minv(0))/2.0,minv(1),(maxv(2)+minv(2))/2.0);
		points = points - dyad(t,ones<T>(points.ncols()));
		T s = max_value(maxv-minv);
		points=scale_33<T>(sf/s,
			sf/s,
			sf/s)*points;
	}
}

template<typename T>
inline cgv::math::mat<T> center_and_scale_44(mat<T>& points,const T& sf=1)
{
	if(points.ncols() > 0)
	{
		assert(points.nrows() == 3);
		vec<T> minv =minimum(points);
		vec<T> maxv =maximum(points);
		vec<T> t = vec<T>((maxv(0)+minv(0))/2.0,minv(1),(maxv(2)+minv(2))/2.0);
		T s = max_value(maxv-minv);
		return scale_44<T>(sf/s,
			sf/s,
			sf/s)*translate_44(-t);
	}
	return cgv::math::identity<T>(4);
}



 
}

}

