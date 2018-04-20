#include "normal_estimation.h"

#include <cgv/math/mat.h>
#include <cgv/math/eig.h>
#include <cgv/math/point_operations.h>

namespace cgv {
	namespace math {

void estimate_normal_ls(unsigned nr_points, const float* _points, float* _normal, float* _evals, float* _mean, float* _evecs)
{
	cgv::math::mat<float> points;
	points.set_extern_data(3, nr_points, const_cast<float*>(_points));

	cgv::math::vec<float> normal;
	normal.set_extern_data(3, _normal);

	cgv::math::mat<float> covmat;
	cgv::math::vec<float> mean;

	cgv::math::covmat_and_mean(points,covmat,mean);
	cgv::math::mat<double> dcovmat(covmat), v;
	cgv::math::diag_mat<double> d;
	cgv::math::eig_sym(dcovmat,v,d);

	normal = cgv::math::vec<float>(normalize(v.col(2)));
	if (_evals) {
		_evals[0] = (float)d(0);		
		_evals[1] = (float)d(1);		
		_evals[2] = (float)d(2);
	}
	if (_mean) {
		_mean[0] = (float)mean(0);
		_mean[1] = (float)mean(1);
		_mean[2] = (float)mean(2);
	}
	if (_evecs) {
		_evecs[0] = (float)v(0, 0);
		_evecs[1] = (float)v(1, 0);
		_evecs[2] = (float)v(2, 0);
		_evecs[3] = (float)v(0, 1);
		_evecs[4] = (float)v(1, 1);
		_evecs[5] = (float)v(2, 1);
		_evecs[6] = (float)v(0, 2);
		_evecs[7] = (float)v(1, 2);
		_evecs[8] = (float)v(2, 2);
	}
}

void estimate_normal_wls(unsigned nr_points, const float* _points, const float* _weights, float* _normal, float* _evals, float* _mean, float* _evecs)
{
	cgv::math::mat<float> points;
	points.set_extern_data(3, nr_points, const_cast<float*>(_points));
	cgv::math::vec<float> weights;
	weights.set_extern_data(nr_points, const_cast<float*>(_weights));
	cgv::math::vec<float> normal;
	normal.set_extern_data(3, _normal);

	cgv::math::mat<float> covmat;
	cgv::math::vec<float> mean;

	

	cgv::math::weighted_covmat_and_mean(weights,points,covmat,mean);
	cgv::math::mat<double> dcovmat(covmat), v;
	cgv::math::diag_mat<double> d;
	cgv::math::eig_sym(dcovmat,v,d);

	normal = cgv::math::vec<float>(normalize(v.col(2)));

	if (_evals) {
		_evals[0] = (float)d(0);		
		_evals[1] = (float)d(1);		
		_evals[2] = (float)d(2);
	}
	if (_mean) {
		_mean[0] = (float)mean(0);
		_mean[1] = (float)mean(1);
		_mean[2] = (float)mean(2);
	}
	if (_evecs) {
		_evecs[0] = (float)v(0, 0);
		_evecs[1] = (float)v(1, 0);
		_evecs[2] = (float)v(2, 0);
		_evecs[3] = (float)v(0, 1);
		_evecs[4] = (float)v(1, 1);
		_evecs[5] = (float)v(2, 1);
		_evecs[6] = (float)v(0, 2);
		_evecs[7] = (float)v(1, 2);
		_evecs[8] = (float)v(2, 2);
	}
}

	}
}