#pragma once

#include <random>
#include <cgv/math/quaternion.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/statistics.h>

typedef cgv::math::quaternion<double> quat;
typedef cgv::math::fvec<double, 3> vec3;
typedef cgv::math::fmat<double, 3, 3> mat3;

bool sub_test_quat(const mat3& R, double eps)
{
	quat q(R);
	mat3 R1;
	q.put_matrix(R1);
	if ((R - R1).frobenius_norm() > eps)
		return false;
	std::default_random_engine r;
	std::uniform_real_distribution<double> d(0.0, 1.0);
	bool success = true;
	for (unsigned i = 0; i < 100; ++i) {
		vec3 x(d(r), d(r), d(r));
		vec3 yR = R * x;
		vec3 yq;
		q.put_image(x, yq);
		if ((yR - yq).length() > eps) {
			success = false;
			break;
		}
	}
	return success;
}

void test_quat()
{
	static const unsigned N = 16;
	static const double degree = 360.0 / N;
	mat3 Rx = cgv::math::rotate3<double>(degree, vec3(1, 0, 0));
	mat3 Rz = cgv::math::rotate3<double>(degree, vec3(0, 1, 0));
	mat3 Ry = cgv::math::rotate3<double>(degree, vec3(0, 0, 1));

	unsigned success_cnt = 0;
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
		if (sub_test_quat(R, 1e-7))
			++success_cnt;
	}

	std::default_random_engine r;
	std::uniform_real_distribution<double> d(0.0, 1.0);
	bool success = true;
	for (unsigned i = 0; i < N*N*N; ++i) {
		quat q;
		do {
			q = quat(d(r), d(r), d(r), d(r));
		} while (q.length() < 0.01);
		q.normalize();
		mat3 R;
		q.put_matrix(R);
		if (sub_test_quat(R, 1e-7))
			++success_cnt;
	}
	unsigned diff = success_cnt = 2 * N*N*N - success_cnt;
	if (diff > 0)
		std::cerr << "\n" << diff << " of " << 2 * N*N*N << " quat tests failed" << std::endl;
	else
		std::cout << "\n" << "quats ok" << std::endl;
	assert(success_cnt == 2*N * N*N);
}