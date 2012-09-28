#pragma once
#include <cgv/math/qr.h>

void test_qr()
{
	using namespace cgv::math;
	//test qr decomposition
	mat<double> m(3,3);
	m(0,0) = 1.0; m(0,1) = 2.0;	m(0,2) = 3.0;
	m(1,0) = 4.0; m(1,1) = 5.0;	m(1,2) = 6.0;
	m(2,0) = 1.0; m(2,1) = 8.0;	m(2,2) = 2.0;
	mat<double> q;
	up_tri_mat<double> r;
	qr(m,q,r);
	assert(frobenius_norm(q*r- m) < 0.0001);
	
	//test rq decomposition
	rq(m,r,q);
	assert(frobenius_norm(r*q- m) < 0.0001);

}