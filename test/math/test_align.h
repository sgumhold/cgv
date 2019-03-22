#pragma once

#include <random>
#include <cgv/math/align.h>
#include <cgv/math/inv.h>

template <typename T = float, typename T_SVD = T>
void test_align(unsigned nr_points, unsigned nr_tests, bool test_reflection, bool test_scaling)
{
	unsigned i;
	// generate random point set 
	std::default_random_engine generator;
	std::uniform_real_distribution<T> uniform_distribution(T(-1), T(1));
	// generate 100 points
	std::vector<cgv::math::fvec<T, 3> > target_points;
	for (i = 0; i < nr_points; ++i)
		target_points.push_back(cgv::math::fvec<T, 3>(uniform_distribution(generator), uniform_distribution(generator), uniform_distribution(generator)));
	// do 100 tests
	std::normal_distribution<T> normal_distribution;
	for (i = 0; i < nr_tests; ++i) {
		// sample rotation
		cgv::math::quaternion<T> q(normal_distribution(generator), normal_distribution(generator), normal_distribution(generator), normal_distribution(generator));
		q.normalize();
		cgv::math::fmat<T, 3, 3> O, _O;
		q.put_matrix(O);

		// sample reflection
		cgv::math::fmat<T, 3, 3> R;
		R.identity();
		if (test_reflection && (uniform_distribution(generator) < 0)) {
			R(2, 2) = -1;
			O = O*R;
		}

		// sample translation
		cgv::math::fvec<T, 3> t(uniform_distribution(generator), uniform_distribution(generator), uniform_distribution(generator)), _t;
		// sample scaling
		T s = uniform_distribution(generator) + 1.1f, _s;
		if (!test_scaling)
			s = 1.0;

		// compute source point cloud
		std::vector<cgv::math::fvec<T, 3> > source_points;
		cgv::math::fmat<T, 3, 3> A = inv(O);
		A *= 1.0f / s;
		for (unsigned j = 0; j < nr_points; ++j)
			source_points.push_back(A*(target_points[j] - t));

		// compute alignment
		if (test_scaling) {
			cgv::math::align<T, T_SVD>(source_points, target_points, _O, _t, &_s, test_reflection);
			// check result
			if (fabs(s - _s) > 1e-6)
				std::cout << i << ": scales differ " << s << " <-> " << _s << std::endl;
		}
		else {
			cgv::math::align(source_points, target_points, _O, _t, (T*)0, test_reflection);
		}
		if (length(t - _t) > 1e-6)
			std::cout << i << ": translations differ " << t << " <-> " << _t << std::endl;
		if (length(O - _O) > 1e-6)
			std::cout << i << ": orientations differ " << O << " <-> " << _O << std::endl;
	}
}
