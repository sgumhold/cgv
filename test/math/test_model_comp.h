#pragma once
#include <iostream>
#include <cgv/math/model_comp.h>
#include <cgv/math/polynomial.h>
#include <cgv/math/random.h>
#include <cgv/math/lin_solve.h>
#include <cgv/math/functions.h>


cgv::math::vec<double> f(const cgv::math::vec<double>& x)
{
	cgv::math::random rangen;
	cgv::math::vec<double> r(x.size());
	double e;
	for(unsigned i=0; i < x.size(); i++)
	{
		rangen.normal(0.0,0.05,e);
		r(i) = cgv::math::sqr(sin(x(i)*x(i)*x(i)/100.0)) + e;
	}

	return r;
}

double compute_RSS(cgv::math::vec<double>& p,
				   cgv::math::vec<double>& x,
				   cgv::math::vec<double>& y)
{
	double rss = 0;
	rss+=cgv::math::sqr_length(y-cgv::math::poly_val(p,x));
	return rss;
}


double fit_AIC(int deg,cgv::math::vec<double>&X,cgv::math::vec<double>&Y)
{

	cgv::math::vec<double> p = cgv::math::poly_fit<double>(deg,X,Y);
	
	double rss = compute_RSS(p,X,Y);
	double aic = cgv::math::AICc_ls<double>(p.size(),X.size(),rss);
	std::cout <<"degree: "<<deg<< " AICc:"<<aic  <<" rss:"<<rss<<std::endl;
	return aic;
}

double fit_BIC(int deg,cgv::math::vec<double>&X,cgv::math::vec<double>&Y)
{
	cgv::math::vec<double> p = cgv::math::poly_fit<double>(deg,X,Y);

	double rss = compute_RSS(p,X,Y);
	double bic = cgv::math::BIC<double>(p.size(),X.size(),rss);
	std::cout <<"degree: "<<deg<< " BIC:"<<bic  <<" rss:"<<rss<<std::endl;
	return bic;
}

void test_model_comparison()
{
	using namespace cgv::math;

	vec<double> X = cgv::math::lin_space<double>(0.0,10.0,300);
	vec<double> Y = f(X);
	double ic=0;
	int deg =-1;
	for(unsigned i = 1; i < 30;i++)
	{
		 double ict = fit_AIC(i,X,Y);
		 if(i == 1)
		 {
			 ic=ict;
			 deg=i;
		 }
		 if(ic > ict)
		 {
			 ic=ict;
			 deg=i;
		 }
	}
	std::cout << "best deg:" << deg<<std::endl;
	
	/*cgv::math::mat<double> V = cgv::math::vander(21,X);
	cgv::math::mat<double> AtA =transpose(V)*V;
	cgv::math::vec<double> Aty=transpose(V)*Y;
	
	cgv::math::vec<double> p;
	cgv::math::svd_solve(AtA,Aty,p);

	std::cout << p<<std::endl;
	double rss = compute_RSS(p,X,Y);
	std::cout <<"rss:"<<rss<<std::endl;
		

	std::cout << cgv::math::AICc_ls<double>(3,20,rss);*/

}