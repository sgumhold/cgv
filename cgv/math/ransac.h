#pragma once
#include <cmath>
#include <cgv/math/functions.h>
#include <vector>


namespace cgv{
	namespace math{

///returns the number of needed ransac iterations
///n_min... minimal number of needed samples
///p_out... percentage of inliers
///p_surety...  probability to sample at least one inlier
template <typename T>
unsigned num_ransac_iterations(unsigned n_min, const T p_out, const T p_surety = 0.99)
{
	unsigned int iter = (unsigned) ::ceil(log(1.0-p_surety)/log(1.0-pow(1.0-p_out,(T)n_min)));
	if (iter == 0)
		iter = 1;
	return iter;
}







	}
}

