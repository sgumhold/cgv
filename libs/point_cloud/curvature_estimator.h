#pragma once

#include "point_cloud.h"
#include <array>

#include "lib_begin.h"

// this helper class stores and computes curvature estimations for a single point
class principal_curvature_estimation : public point_cloud_types {
	//principal components of the normals used for computation
	Dir principal_curvature_vector;
	//principal curvatures (eigen values)
	float pc_max;
	float pc_min;
public:
	inline principal_curvature_estimation() = default;

	inline principal_curvature_estimation(const Dir& principal_curvature, const float& principal_max, const float& principal_min) :
		principal_curvature_vector(principal_curvature),pc_max(principal_max), pc_min(principal_min)
	{}

	inline const float& min() const {
		return pc_min;
	}

	inline const float& max() const {
		return pc_max;
	}

	inline float mean() const {
		return (pc_min + pc_max) * 0.5f;
	}

	inline float gaussian() const {
		return pc_min * pc_max;
	}

	//returns a pointer to the 3 principal components
	inline const Dir& principal_curvature() const{
		return principal_curvature_vector;
	}
};

class CGV_API curvature_estimator : public point_cloud_types {
	point_cloud& pc;

public:
	curvature_estimator(point_cloud& spc) : pc(spc){
	}

	void compute_principal_curvature(std::vector<principal_curvature_estimation>& pce);
};

#include <cgv/config/lib_end.h>