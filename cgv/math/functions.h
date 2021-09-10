#pragma once

#include <cmath>
#include <cassert>
#include <limits>
#include <vector>

#include "lib_begin.h"

namespace cgv{
	namespace math {

		/// returns the abs(a)*sign(b)
		template<typename T>
		T sign(const T&a, const T&b) { return (b < 0) ? -std::abs(a) : std::abs(a); }
		/// if v >= 0 returns 1 and otherwise -1
		template<typename T>
		T sign(const T&v) { return (v >= T(0)) ? T(1) : T(-1); }
		/// if v >= 0 returns v or 0 otherwise
		template<typename T>
		T plus(const T& v) { return v >= T(0) ? v : T(0); }
		/// clamp v at [a,b]
		template<typename T>
		T clamp(const T& v, const T& a, const T& b) { return v > b ? b : (v < a ? a : v); }
		/// clamp v at [0,1]
		template<typename T>
		T saturate(const T& v) { return v > T(1) ? T(1) : (v < T(0) ? T(0) : v); }
		namespace detail {
			//helper function to compute cheb approximation of erf functions
			template <typename T>
			T erfccheb(const T z)
			{
				static const int ncof = 28;
				static const double cof[28] = { -1.3026537197817094, 6.4196979235649026e-1,
				1.9476473204185836e-2,-9.561514786808631e-3,-9.46595344482036e-4,
				3.66839497852761e-4,4.2523324806907e-5,-2.0278578112534e-5,
				-1.624290004647e-6,1.303655835580e-6,1.5626441722e-8,-8.5238095915e-8,
				6.529054439e-9,5.059343495e-9,-9.91364156e-10,-2.27365122e-10,
				9.6467911e-11, 2.394038e-12,-6.886027e-12,8.94487e-13, 3.13092e-13,
				-1.12708e-13,3.81e-16,7.106e-15,-1.523e-15,-9.4e-17,1.21e-16,-2.8e-17 };

				int j;
				double t, ty, tmp, d = 0., dd = 0.;
				assert(z >= 0.);

				t = 2. / (2. + z);
				ty = 4.*t - 2.;
				for (j = ncof - 1; j > 0; j--)
				{
					tmp = d;
					d = ty * d - dd + cof[j];
					dd = tmp;
				}
				return (T)(t*exp(-z * z + 0.5*(cof[0] + ty * d) - dd));
			}
		}
		///error function
		template <typename T>
		T erf(const T x)
		{
			if (x >= 0.)
				return T(1.0) - detail::erfccheb(x);
			else
				return detail::erfccheb(-x) - T(1.0);
		}

		///complementary error function
		template <typename T>
		T erfc(const T x)
		{
			if (x >= 0.)
				return detail::erfccheb(x);
			else
				return 2.0 - detail::erfccheb(-x);
		}

		///scaled complementary error function
		template <typename T>
		T erfcx(const T x)
		{
			return exp(x*x)* erfccheb(x);
		}

		///inverse complementary error function
		template <typename T>
		T erfc_inv(const T p)
		{
			double x, err, t, pp;
			if (p >= 2.0) return -100.;
			if (p <= 0.0) return 100.;
			pp = (p < 1.0) ? p : 2. - p;
			t = sqrt(-2.*log(pp / 2.));
			x = -0.70711*((2.30753 + t * 0.27061) / (1. + t * (0.99229 + t * 0.04481)) - t);
			for (int j = 0; j < 2; j++) {
				err = erfc(x) - pp;
				x += err / (1.12837916709551257*exp(-(x*x)) - x * err);
			}
			return (p < 1.0 ? x : -x);
		}

		///inverse error function
		template <typename T>
		T erf_inv(const T p)
		{
			return erfc_inv(1. - p);
		}


		/// evaluate the cummulative normal distribution function
		template <typename T>
		T Phi(const T& x)
		{
			return (T)(0.5*erfc(-x*sqrt(0.5)));
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
