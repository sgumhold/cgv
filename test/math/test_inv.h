#pragma once
#include <cgv/math/inv.h>


void test_inv()
{
	using namespace cgv::math;

	//invert lower triangular matrix
	low_tri_mat<double> m1(3);
	m1(0,0) = 1.0; 
    m1(1,0) = 2.0; m1(1,1) = 3.0;
	m1(2,0) = 4.0; m1(2,1) = 5.0;	m1(2,2) = 6.0;
	mat<double> mr1 = m1*inv(m1);
	assert(mr1 == identity<double>(3));
	
	//invert upper triangular matrix
	up_tri_mat<double> m2(3);
	m2(0,0) = 1.0; m2(0,1) = 2.0;	m2(0,2) = 3.0;
				   m2(1,1) = 4.0;   m2(1,2) = 5.0;
								    m2(2,2) = 6.0; 
	mat<double> mr2 =m2*inv(m2);
	assert(mr2 == identity<double>(3));

	//invert  full 3x3 matrix
	mat<double> m3(3,3);
	m3(0,0) = 1.0; m3(0,1) = 2.0;	m3(0,2) = 3.0;
	m3(1,0) = 4.0; m3(1,1) = 5.0;	m3(1,2) = 6.0;
	m3(2,0) = 1.0; m3(2,1) = 8.0;	m3(2,2) = 9.0;
	mat<double> mr3 = m3*inv(m3);
	assert(mr3 == identity<double>(3));

	//invert diagonal matrix
	diag_mat<double> m4(3);
	m4(0) = 1.0; m4(1) = 2.0;	m4(2) = 3.0;
	mat<double> mr4 = m4*inv(m4);
	assert(mr4 == identity<double>(3));

	//invert permutation matrix
	perm_mat m5(4);
	m5.swap(1,3);
	m5.swap(2,3);
	m5.swap(1,2);
	mat<double> mr5 = m5*inv(m5);
	assert(mr5 == identity<double>(4));


}