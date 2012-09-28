#pragma once
#include <cgv/math/gaussj.h>

void test_gaussj()
{
	using namespace cgv::math;
	//test svd decomposition
	mat<double> a(3,3);
	a(0,0) = 1.0; a(0,1) = 2.0;	a(0,2) = 3.0;
	a(1,0) = 2.0; a(1,1) = 5.0;	a(1,2) = 6.0;
	a(2,0) = 3.0; a(2,1) = 6.0;	a(2,2) = 2.0;
	mat<double> ainv=a;

	
	mat<double> b(3,2);
	b(0,0) = 1; b(1,0) = 2; b(2,0) = 3;

	b(0,1) = 1; b(1,1) = 0; b(2,1) = 1;
	mat<double> x = b;
	gaussj(ainv,x);
	

	//std::cout <<round(a*x-b) <<std::endl;//zero matrix
	//std::cout <<round(a*ainv) <<std::endl;//identity
	
}