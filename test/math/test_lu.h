#pragma once
#include <cgv/math/lu.h>

void test_lu()
{
	using namespace cgv::math;
	//test lu decomposition
	mat<double> m(3,3);
	m(0,0) = 1.0; m(0,1) = 2.0;	m(0,2) = 3.0;
	m(1,0) = 4.0; m(1,1) = 5.0;	m(1,2) = 6.0;
	m(2,0) = 1.0; m(2,1) = 8.0;	m(2,2) = 2.0;
	perm_mat p;
	low_tri_mat<double> l;
	up_tri_mat<double> u;
	lu(m,p,l,u);
	assert(p*l*u == m);

}