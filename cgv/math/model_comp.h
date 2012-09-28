#pragma once
#include <cmath>


namespace cgv{
	namespace math {

///Akaike's:  An Information Criterion 
///k...number of parameters
///L...maximized value of likelihood function for estimated model
///n/k should be > 40 otherwise use AICc (n ... number of samples)
template<typename T>
T AIC(int k, const T& L)
{
	return (const T)2.0*(T)k - (T)2*(T)log((double)L);
}


///Akaike's:  An information criterion for  
///k...number of parameters
///n...number of samples
///RSS...the residual sum of squares
///n/k should be greater 40 otherwise use AICc
template<typename T>
T AIC_ls(int k, int n, const T&RSS)
{
	return 2*k + (T)n*log(RSS/(T)n);
}

///Akaike's:  An information criterion with second order correction term
///k...number of parameters
///n.. number of samples
///L...maximized value of likelihood function for estimated model
template<typename T>
T AICc(int k, int n, const T& L)
{
	return AIC<T>(k,L) + (T)(2*k*(k+1))/((T)(n-k-1));
}

///Akaike's:  An information criterion 
///k...number of parameters
///n...number of samples
///RSS...the residual sum of squares
template<typename T>
T AICc_ls(int k, int n, const T&RSS)
{
	return (T)log((T)RSS/(T)n) + (T)(n+k)/(T)(n-k-2) ;
}

///Akaike's:  An information criterion 
///k...number of parameters
///n...number of samples
///RSS...the residual sum of squares from the estimated model
template<typename T>
T AICu_ls(int k, int n, const T&RSS)
{
	return (T)log((T)RSS/(n-k)) + (T)(n+k)/(T)(n-k-2) ;
}

///Bayesian information criterion (Schwarz Information Criterion)
///k...number of parameters
///n...number of samples
template<typename T>
T BIC(int k, int n, const T& L)
{
	return -(T)2*(T)log((T)L) + (T)k*log((T)n);
}

///Bayesian information criterion (Schwarz Information Criterion)
///k...number of parameters
///n...number of samples
///RSS...the residual sum of squares from the estimated model
template<typename T>
T BIC_ls(int k, int n, const T& RSS)
{
	return (T)n*log((T)RSS/(T)n) + (T)k*log((T)n);
}

	}
}



