#pragma once
#include <cgv/math/svd.h>
#include <random>

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

template <typename T>
void test_svd_3d(unsigned nr_tests)
{
	unsigned i, j;
	// generate random point set 
	std::default_random_engine generator;
	std::uniform_real_distribution<T> uniform_distribution(T(-1), T(1));

	cgv::math::fmat<T, 3, 3> A, B;
	for (j = 0; j < nr_tests; ++j) {
		for (i = 0; i < 9; ++i)
			((cgv::math::fvec<T, 9>&)A)(i) = uniform_distribution(generator);
		cgv::math::fmat<T, 3, 3> U, V_t, Diag; Diag.identity();
		cgv::math::fvec<T, 3> D;
		cgv::math::svd(A, U, D, V_t);
		Diag(0, 0) = D(0);
		Diag(1, 1) = D(1);
		Diag(2, 2) = D(2);
		B = U*Diag*V_t;
		if (length(A - B) > T(1e-6))
			std::cout << j << "(" << sizeof(T) << "): svd failed\n" << A << "\n<->\n" << B << std::endl;
	}
}
