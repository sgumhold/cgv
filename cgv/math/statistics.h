#pragma once
#include <cgv/math/functions.h>
#include <cgv/math/vec.h>

namespace cgv {
	namespace math {

///normal distribution prob density function
template <typename T>
T norm_pdf(const T x,const T mu=0,const T sig=1)
{
	return (T) (0.398942280401432678/sig)*exp(-0.5*sqr((x-mu)/sig));
}

///normal cumulative distribution function
template <typename T>
T norm_cdf(const T x,const T mu=0,const T sig=1)
{
	return T(0.5)*erfc(T(-0.707106781186547524*(x-mu)/sig));
}

///inverse normal cumulative distribution function
template <typename T>
T norm_inv(const T& p,const T mu=0,const T sig=1) 
{
	assert(p >= 0 && p <= 1);
		return -1.41421356237309505*sig*erfc_inv(2.*p)+mu;
}

///lognormal normal prob distribution function
template <typename T>
T logn_pdf(const T x,const T mu=0,const T sig=1) 
{
	assert (x >= 0.);
	if (x == 0.) return 0.;
	return (0.398942280401432678/(sig*x))*exp(-0.5*sqr((log(x)-mu)/sig));
}

///log normal cumulative distribution function
template <typename T>
T logn_cdf(const T x, const T mu=0,const T sig=1) 
{
	assert(x >= 0.);
	if (x == 0.) return 0.;
	return (T)(0.5*erfc(-0.707106781186547524*(log(x)-mu)/sig));
}

///inverse log normal cumulative distribution function
template <typename T>
T logn_inv(const T p, const T mu=0,const T sig=1) 
{
	assert (p > 0. && p < 1.);
		return exp(-1.41421356237309505*sig*erfc_inv(2.*p)+mu);
}


///uniform distribution prob density function
template <typename T>
T uniform_pdf(const T x,const T a,const T b)
{
	if(x < a)
		return 0;
	if(x > b)
		return 0;
	return 1.0/(b-a);
}

///uniform cumulative distribution function
template <typename T>
T uniform_cdf(const T x,const T a,const T b)
{
	if(x < a)
		return 0;
	if(x >=b)
		return 1;
	return (x-a)/(b-a);	
}

//cumulative kolmogorov-smirnov distribution function
template <typename T>
T ks_cdf(const T z) 
{
	assert(z >= 0.);
	if (z == 0.) return 0.;
	if (z < 1.18) 
	{
			double y = exp(-1.23370055013616983/sqr(z));
			return (T)(2.25675833419102515*sqrt(-log(y))
				*(y + pow(y,9) + pow(y,25) + pow(y,49)));
	} else 
	{
			double x = exp(-2.*sqr(z));
			return (T)(1. - 2.*(x - pow(x,4) + pow(x,9)));
	}
}

///complementary cumulative kolmogorov-smirnov distribution function
template <typename T>
T ksc_cdf(const T z) 
{
	assert (z >= 0.) ;
	if (z == 0.) return 1.;
	if (z < 1.18) return 1.-ks_cdf(z);
	double x = exp(-2.*sqr(z));
	return (T)(2.*(x - pow(x,4) + pow(x,9)));
}


///helper function to compute ksc_inv
template <typename T>
T invxlogx(const T y) 
{
	const double ooe = 0.367879441171442322;
	double t,u,to=0.;
	assert(y < 0. && y > -ooe);
 	if (y < -0.2) u = log(ooe-sqrt(2*ooe*(y+ooe)));
	else u = -10.;
	do {
		u += (t=(log(y/u)-u)*(u/(1.+u)));
		if (t < 1.e-8 && abs(t+to)<0.01*abs(t)) break;
		to = t;
	} while (abs(t/u) > 1.e-15);	
	return(T) exp(u);
}

///inverse complementary cumulative kolmogorv-smirnov distribution function
template <typename T>
T ksc_inv(const T q) 
{
		double y,logy,yp,x,xp,f,ff,u,t;
		assert (q > 0. && q <= 1.) 
		if (q == 1.) return 0.;
		if (q > 0.3) {
			f = -0.392699081698724155*SQR(1.-q);
			y = invxlogx(f);
			do {
				yp = y;
				logy = log(y);
				ff = f/sqr(1.+ pow(y,4)+ pow(y,12));
				u = (y*logy-ff)/(1.+logy);
				y = y - (t=u/std::max(0.5,1.-0.5*u/(y*(1.+logy))));
			} while (abs(t/y)>1.e-15);
			return (T)(1.57079632679489662/sqrt(-log(y)));
		} else {
			x = 0.03;
			do {
				xp = x;
				x = 0.5*q+pow(x,4)-pow(x,9);
				if (x > 0.06) x += pow(x,16)-pow(x,25);
			} while (abs((xp-x)/x)>1.e-15);
			return (T)sqrt(-0.5*log(x));
		}
}

///inverse of the cumulative kolmogorv-smirnov distribution function
template <typename T>
T ks_inv(const T p) 
{
	return ksc_inv(1.-p);
}



///kolmogorov-smirnov test for comparing samples with given normal distribution
///returns the p-value for the null hypothesis that the data is drawn from a normal distribution defined by mu and sigma
template <typename T>
T norm_ks_test(const T mu,const T sig,vec<T> data)
{
	sort_values(data);
	unsigned n = data.size();
	T fo=0;
	T dt,ff,fn,en,d=0.0;
	en=(T)n;
	for(unsigned i = 0; i < n; i++)
	{
		fn = (T)(i+1.0)/en;
		ff = norm_cdf(data(i),mu,sig);
		dt = std::max(std::abs(fo-ff),std::abs(fn-ff));
		if(dt > d)
			d=dt;
		fo=fn;
	}
	en=sqrt(en);
	return ksc_cdf((en+0.12+0.11/en)*d);
}

///kolmogorov-smirnov test for comparing two sampled distributions
///returns the p-value for the null hypothesis that the the samples data1 are drawn from the same distribution as the samples of data2
template <typename T>
T ks_test(vec<T> data1, vec<T> data2)
{
	int j1=0,j2=0,n1=data1.size(),n2=data2.size();
	T d1,d2,dt,en1,en2,en,fn1=0.0,fn2=0.0;
	KSdist ks;
	sort_values(data1);
	sort_values(data2);
	en1=n1;
	en2=n2;
	T d=0.0;
	while (j1 < n1 && j2 < n2) {
		if ((d1=data1(j1)) <= (d2=data2(j2)))
			do
				fn1=++j1/en1;
			while (j1 < n1 && d1 == data1(j1));
		if (d2 <= d1)
			do
				fn2=++j2/en2;
			while (j2 < n2 && d2 == data2(j2));
		if ((dt=abs(fn2-fn1)) > d) d=dt;
	}
	en=sqrt(en1*en2/(en1+en2));
	return ksc_cdf((en+0.12+0.11/en)*d);
}


	}
}