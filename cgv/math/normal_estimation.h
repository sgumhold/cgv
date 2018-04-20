#pragma once

#include "lib_begin.h"

namespace cgv {
	namespace math {

		//! Compute least squares normal from an array of 3D points.
		/*! Input is the number of points \c nr_points and a pointer \c _points to the point array. Points
		    are assumed to be float tripples laying consecutively in the memory. Thus a vector \c P of type
			\c std::vector<cgv::math::fvec<float,3> > can be passed to this function according to
			estimate_normal_ls(P.size(), &P.front()[0], ...). The resulting 3D normal is written into the memory
			point to by \c _normal assuming space for 3 floats. If \c _evals is specified, also the eigenvalues from the
			fit are written to 3 floats pointed to by \c _evals. If \c _mean is specified, also the point mean through 
			which the least squares plane goes is returned. If \c _evecs is specified, also the eigenvectors from the 
			fit are written in 3 float trippels to memory pointed to by \c _evecs.*/
		extern CGV_API void estimate_normal_ls(unsigned nr_points, const float* _points, float* _normal, float* _evals = 0, float* _mean = 0, float* _evecs = 0);

		/// Weighted version of \c estimate_normal_ls with additional input \c _weights pointing to \c nr_points scalar weights.
		extern CGV_API void estimate_normal_wls(unsigned nr_points, const float* _points, const float* _weights, float* _normal, float* _evals = 0, float* _mean = 0, float* _evecs = 0);
	}
}
#include <cgv/config/lib_end.h>
