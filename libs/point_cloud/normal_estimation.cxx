#include "normal_estimation.h"

#include <cgv/math/mat.h>
#include <cgv/math/eig.h>
#include <cgv/math/point_operations.h>

void estimate_normal_ls(unsigned nr_points, const float* _points, float* _normal, float* _evs)
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
	if (_evs) {
		_evs[0] = (float)d(0);		
		_evs[1] = (float)d(1);		
		_evs[2] = (float)d(2);
	}
}

void estimate_normal_wls(unsigned nr_points, const float* _points, const float* _weights, float* _normal, float* _evs)
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

	if (_evs) {
		_evs[0] = (float)d(0);		
		_evs[1] = (float)d(1);		
		_evs[2] = (float)d(2);
	}
}
