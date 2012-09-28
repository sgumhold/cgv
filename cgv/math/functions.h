#pragma once

#include <cmath>
#include <cassert>
#include <limits>
#include <vector>

#include "lib_begin.h"

namespace cgv{
	namespace math {

	//returns the abs(a)*sign(b)
	template<typename T> 
	T sign(const T&a, const T&b)
	{
		if(b < 0)
		 return -std::abs(a) ;
		else
			return std::abs(a);
	}

		namespace detail {
			template <typename T>
			T erf_fitting_function(const T& u)
			{
				 return (T) (
					  - 1.26551223 + u*(1.00002368 + u*(0.37409196 + u*(0.09678418 + 
					  u*(-0.18628806 + u*(0.27886807 + u*(-1.13520398 + u*(1.48851587 +
					  u*(-0.82215223 + u*0.17087277)))))))));
			}
		}

		/// evaluate the error function erf(x) = 2/sqrt(Pi)*int(exp(-y^2),y=0..x)
		template <typename T>
		T erf(const T& x)
		{
			if (x < 0)
				return -erf(-x);
			T u = 1/(1 + (T)0.5*x);
			T ans = u*exp(-x*x + detail::erf_fitting_function(u));
			return 1 - ans;
		}

		/// evaluate the complementary error function erfc(x) = 1-erf(x)
		template <typename T>
		T erfc(const T& x)
		{
			return 1-erf(x);
		}

		/// evaluate the cummulative normal distribution function
		template <typename T>
		T Phi(const T& x)
		{
			return 0.5*erfc(-x*sqrt(0.5));
		}

		/// evaluate the inverse of the error function Phi^(-1)(p), p=0..1
		template <typename T>
		T inv_Phi(const T& p)
		{
			static T a[] = {(T)-3.969683028665376e+01,(T) 2.209460984245205e+02,(T) -2.759285104469687e+02,(T) 1.383577518672690e+02,(T) -3.066479806614716e+01,(T) 2.506628277459239e+00};
			static T b[] = {(T)-5.447609879822406e+01,(T) 1.615858368580409e+02,(T) -1.556989798598866e+02,(T) 6.680131188771972e+01,(T) -1.328068155288572e+01};
			static T c[] = {(T)-7.784894002430293e-03,(T)-3.223964580411365e-01,(T)-2.400758277161838e+00,(T)-2.549732539343734e+00,(T) 4.374664141464968e+00,(T) 2.938163982698783e+00};
			static T d[] = {(T)7.784695709041462e-03,(T)3.224671290700398e-01,(T)2.445134137142996e+00,(T)3.754408661907416e+00};

			if (p <= 0)
				return -std::numeric_limits<T>::infinity();
			if (p >= 1)
				return  std::numeric_limits<T>::infinity();
			T x;
			if (p < (T)0.02425) {
				T q = sqrt(-(T)2.0*log(p));
				x = (((((c(1)*q+c(2))*q+c(3))*q+c(4))*q+c(5))*q+c(6)) /
					((((d(1)*q+d(2))*q+d(3))*q+d(4))*q+1);
			}
			else if (p <= (T)0.97575) {
			  T q = p - (T)0.5;
			  T r = q*q;
			  x = (((((a(1)*r+a(2))*r+a(3))*r+a(4))*r+a(5))*r+a(6))*q /
				   (((((b(1)*r+b(2))*r+b(3))*r+b(4))*r+b(5))*r+1);
			}
			else {
			  T q = sqrt(-(T)2.0*log(1-p));
			  x = -(((((c(1)*q+c(2))*q+c(3))*q+c(4))*q+c(5))*q+c(6)) /
					 ((((d(1)*q+d(2))*q+d(3))*q+d(4))*q+1);
			}
			return x;
		}

		//return -1 if v is negative and 1 if v >= 0
		template <typename T>
		T sign(const T& v)
		{
			return (v < 0) ?  (T)-1 : (T)1;
		}

		/// compute volume of unit n-ball living in n-dimensional space with n>=1; results are cached for O(1) runtime
		extern CGV_API double compute_unit_ball_volume(unsigned n);
		/// compute volume of n-ball of radius R living in n-dimensional space with n>=1; results are cached for O(1) runtime
		extern CGV_API double compute_ball_volume(unsigned n, double R);
		/// compute surface area of a unit n-ball living in n-dimensional space with n>=2 (this is a n-1 dimensional area); results are cached for O(1) runtime
		extern CGV_API double compute_unit_sphere_area(unsigned n);
		/// compute surface area of a n-ball of radius R living in n-dimensional space with n>=2 (this is a n-1 dimensional area); results are cached for O(1) runtime
		extern CGV_API double compute_sphere_area(unsigned n, double R);

		//returns v*v
		template <typename T>
		T sqr(const T& v)
		{
			return v*v;
		}

		//return the maximum of a and b
		template <typename T>
		T maximum(const T& a, const T&b)
		{
			return a > b ? a : b;
		}

		//return minimum of a and b
		template<typename T>
		T minimum(const T &a, const T&b)
		{
			return a < b ? a : b;
		}

		//return the maximum of a, b and c
		template <typename T>
		T maximum(const T& a, const T&b, const T& c)
		{
			if(a > b)
			{
				return a > c ? a : c;
			}
			else
			{
				return b > c ? b : c;
			}
		}

		//return the minimum of a, b and c
		template <typename T>
		T minimum(const T& a, const T&b, const T& c)
		{
			if(a < b)
			{
				return a < c ? a : c;
			}
			else
			{
				return b < c ? b : c;
			}
		}




		//convert angles measured in degree to angles measured in rad
		template <typename T>
		T deg2rad(const T& deg)
		{
			return (T)(deg *3.14159/180.0);
		}


		//convert angles measured in rad to angles measured in degree
		template <typename T>
		T rad2deg(const T& rad)
		{
			return (T)(rad*180.0/3.14159);
		}

		//ln(gamma(x))
		extern CGV_API double gamma_ln(const double xx);

		//factorial of n  (n!)
		extern CGV_API double fac(const int n);

		//returns ln(n!)
		extern CGV_API double fac_ln(const int n);

		//beta function
		extern CGV_API double beta(const double z, const double w);

		//binomial coefficient n over k
		extern CGV_API double nchoosek(const int n, const int k);

		//returns 0 if v < th otherwise 1
		template <typename T>
		T thresh(const T v,const T th)
		{
			if(v < th)
				return (T)0;
			else
				return (T)1;
		}
	}
}

#include <cgv/config/lib_end.h>
