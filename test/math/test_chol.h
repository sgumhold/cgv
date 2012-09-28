#pragma once
#include <cgv/math/chol.h>

void test_chol()
{
	using namespace cgv::math;
	//test chol decomposition on a symmetric positive definite matrix
	mat<double> m(3,3);
	m(0,0) = 10.0; m(0,1) = 2.0;	m(0,2) = 3.0;
	m(1,0) = 2.0; m(1,1) = 50.0;	m(1,2) = 6.0;
	m(2,0) = 3.0; m(2,1) = 6.0;	m(2,2) = 20.0;
	low_tri_mat<double> l;
	chol(m,l);
	assert(frobenius_norm(l*transpose(l)-m) < 0.0001);
}