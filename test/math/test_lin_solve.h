#pragma once
#include <cgv/math/lin_solve.h>

void test_lin_solve()
{
	using namespace cgv::math;

	tri_diag_mat<float> m(5);
	m.fill(3.0f);
	

	vec<float> b(5);
	for(unsigned i =0; i < 5;i++)
	{
		m(i,0)=4.0f;
		b(i) =(float)i;
	}
	std::cout << (mat<float>)m<<std::endl;
	std::cout <<"b:"<< b << std::endl;
	
	vec<float> x;
	if(solve(m,b,x))
	{
		std::cout <<"x:"<< x << std::endl;
		
		std::cout << "M*x: "<<m*x << std::endl;
		std::cout << "b: "<<b << std::endl;
	}
	else
		std::cout << "not invertible"<<std::endl;
	

}