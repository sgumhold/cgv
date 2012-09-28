#pragma once
#include <cgv/math/sparse_mat.h>


void test_sparse_mat()
{
	using namespace cgv::math;
	sparse_mat<double> m1(100,100,10);
	m1.add_entry(10,10,1.0);
	m1.add_entry(10,20,2.0);
	m1.add_entry(10,30,3.0);
	m1.add_entry(30,10,4.0);
	m1.add_entry(10,10,5.0);
//	m1.compress();
	std::cout << m1;

}