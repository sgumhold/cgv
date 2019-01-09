#pragma once
#include <cgv/math/quaternion.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/statistics.h>

void test_quat()
{
	static const unsigned N = 16;
	static const double degree = 360.0 / N;
	cgv::math::fmat<double, 3, 3> Rx = cgv::math::rotate3<double>(degree, cgv::math::fvec<double, 3>(1, 0, 0));
	cgv::math::fmat<double, 3, 3> Rz = cgv::math::rotate3<double>(degree, cgv::math::fvec<double, 3>(0, 1, 0));
	cgv::math::fmat<double, 3, 3> Ry = cgv::math::rotate3<double>(degree, cgv::math::fvec<double, 3>(0, 0, 1));

	cgv::utils::statistics stats;
	for (unsigned i = 0; i < N*N*N; ++i) {
		unsigned nx = i % N;
		unsigned ny = (i / N) % N;
		unsigned nz = i / (N*N);
		unsigned j;
		cgv::math::fmat<double, 3, 3> R;
		R.identity();
		for (j = 0; j < nx; ++j)
			R *= Rx;
		for (j = 0; j < ny; ++j)
			R *= Ry;
		for (j = 0; j < nz; ++j)
			R *= Rz;
		cgv::math::quaternion<double> q(R);
		cgv::math::fmat<double, 3, 3> R1;
		q.put_matrix(R1);
		stats.update((R - R1).frobenius_norm());
	}
#ifdef _DEBUG
	std::cout << "test_quat stats = " << stats << std::endl;
#endif
	assert(stats.get_max() < 1e-7);
}