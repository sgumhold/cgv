#include "constants.h"
#include "functions.h"

namespace cgv{
	namespace math {

/// compute volume of unit n-ball living in n-dimensional space with n>=1; results are cached for O(1) runtime
double compute_unit_ball_volume(unsigned n)
{
	static std::vector<double> cache;
	if (cache.empty()) {
		cache.reserve(10);
		cache.push_back(2.0);
		cache.push_back(PI);
	}
	if (n == 0)
		return 0;
	while (n > cache.size())
		cache.push_back(*(cache.end()-2) * TWO_PI/cache.size());
	return cache[n-1];
}
/// compute volume of n-ball of radius R living in n-dimensional space with n>=1; results are cached for O(1) runtime
double compute_ball_volume(unsigned n, double R)
{
	return compute_unit_ball_volume(n)*pow(R,(double)n);
}
/// compute surface area of a unit n-ball living in n-dimensional space with n>=2 (this is a n-1 dimensional area); results are cached for O(1) runtime
double compute_unit_sphere_area(unsigned n)
{
	if (n < 2)
		return 0;
	return TWO_PI*compute_unit_ball_volume(n-1);
}
/// compute surface area of a n-ball of radius R living in n-dimensional space with n>=2 (this is a n-1 dimensional area); results are cached for O(1) runtime
double compute_sphere_area(unsigned n, double R)
{
	return compute_unit_sphere_area(n)*pow(R,(double)(n-1));
}



//ln(gamma(x))
double gamma_ln(const double xx) 
{
	int j;
	double x,tmp,y,ser;
	static const double cof[14]={57.1562356658629235,-59.5979603554754912,
	14.1360979747417471,-0.491913816097620199,.339946499848118887e-4,
	.465236289270485756e-4,-.983744753048795646e-4,.158088703224912494e-3,
	-.210264441724104883e-3,.217439618115212643e-3,-.164318106536763890e-3,
	.844182239838527433e-4,-.261908384015814087e-4,.368991826595316234e-5};
	assert(xx >0);
	y=x=xx;
	tmp = x+5.24218750000000000;
	tmp = (x+0.5)*log(tmp)-tmp;
	ser = 0.999999999999997092;
	for (j=0;j<14;j++) ser += cof[j]/++y;
	return tmp+log(2.5066282746310005*ser/x);
}



//factorial of n  (n!)
double fac(const int n)
{
	double a;
	assert(n >=0 && n <171);
	
	a=1.;
	for(int i = 1; i <= n;i++)
		a=(double)i * a;
	
	return a;
}

//returns ln(n!)
double fac_ln(const int n)
{	
	assert(n >=0 && n <171);
		
	double a=1.;
	for(int i = 1; i <=n;i++)
		a=i*a;
	
	return a;
}

//beta function
double beta(const double z, const double w) 
{
	return std::exp(gamma_ln(z)+gamma_ln(w)-gamma_ln(z+w));
}


//binomial coefficient n over k
double nchoosek(const int n, const int k) 
{
	assert (n>=0 && k>=0 && k<=n);
	if (n<171) 
		return ::floor(0.5+fac(n)/(fac(k)*fac(n-k)));
	return ::floor(0.5+exp(fac_ln(n)-fac_ln(k)-fac_ln(n-k)));
}

	}
}