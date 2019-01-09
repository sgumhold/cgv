#pragma once
#include <cgv/math/det.h>

void test_det()
{
	using namespace cgv::math;

	//determinant of  lower triangular matrix
	low_tri_mat<double> m1(3);
	m1(0,0) = 1.0; 
    m1(1,0) = 2.0; m1(1,1) = 3.0;
	m1(2,0) = 4.0; m1(2,1) = 5.0;	m1(2,2) = 6.0;
	assert(det(m1) == 18.0);
	
	
	//determinant of  upper triangular matrix
	up_tri_mat<double> m2(3);
	m2(0,0) = 1.0; m2(0,1) = 2.0;	m2(0,2) = 3.0;
				   m2(1,1) = 4.0;   m2(1,2) = 5.0;
								    m2(2,2) = 6.0; 
	assert(det(m2) == 24.0);
	
	

	//determinant of   full 3x3 matrix
	mat<double> m3(3,3);
	m3(0,0) = 1.0; m3(0,1) = 2.0;	m3(0,2) = 3.0;
	m3(1,0) = 4.0; m3(1,1) = 5.0;	m3(1,2) = 6.0;
	m3(2,0) = 1.0; m3(2,1) = 8.0;	m3(2,2) = 9.0;
	assert(fabs(det(m3) - 18) < 1e-10);
	

	//determinant of  diagonal matrix
	diag_mat<double> m4(3);
	m4(0) = 1.0; m4(1) = 2.0;	m4(2) = 3.0;
	assert(det(m4) == 6);

	//determinant of permutation matrix
	perm_mat m5(4);
	m5.swap(1,2);
	m5.swap(0,1);
	assert(det(m5) == 1);
	m5.swap(1,3);
	assert(det(m5) == -1);
	

}