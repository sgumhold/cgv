#pragma once
#include <cgv/math/eig.h>

void test_eig()
{
	using namespace cgv::math;
	//test svd decomposition
	mat<double> m(3,3);
	m(0,0) = 1.0; m(0,1) = 2.0;	m(0,2) = 3.0;
	m(1,0) = 2.0; m(1,1) = 5.0;	m(1,2) = 6.0;
	m(2,0) = 3.0; m(2,1) = 6.0;	m(2,2) = 2.0;
	
	
	mat<double> q;
	diag_mat<double> w;
	eig_sym(m,q,w);
	assert(frobenius_norm(q*w*inv(q)-m) < 0.0001);
}