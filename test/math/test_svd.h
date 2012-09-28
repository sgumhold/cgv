#pragma once
#include <cgv/math/svd.h>

void test_svd()
{
	using namespace cgv::math;
	//test svd decomposition
	mat<double> m(4,3);
	m(0,0) = 1.0; m(0,1) = 2.0;	m(0,2) = 3.0;
	m(1,0) = 4.0; m(1,1) = 5.0;	m(1,2) = 6.0;
	m(2,0) = 1.0; m(2,1) = 8.0;	m(2,2) = 2.0;
	m(3,0) = 4.0; m(3,1) = 1.0;	m(3,2) = 2.0;
	
	mat<double> u,v;
	diag_mat<double> w;
	svd(m,u,w,v);

	assert(frobenius_norm(u*w*transpose(v)-m) < 0.0001);
}