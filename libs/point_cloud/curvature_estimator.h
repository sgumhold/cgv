#pragma once

#include "point_cloud.h"
#include <array>

#include "lib_begin.h"

// this helper class stores and computes curvature estimations for a single point
class principal_curvature_estimation : public point_cloud_types {
	//principal components of the normals used for computation
	std::array<Dir,2> principal_tangent_vectors;
	//principal curvatures (eigen values)
	float pc_max;
	float pc_min;
public:
	inline principal_curvature_estimation() = default;

	inline principal_curvature_estimation(const float& principal_max, const float& principal_min, const Dir& principal_tangent_max, const Dir& principal_tangent_min) :
		pc_max(principal_max), pc_min(principal_min)
	{
		principal_tangent_vectors[0] = principal_tangent_max;
		principal_tangent_vectors[1] = principal_tangent_min;
	}

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
	inline const Dir& principal_tangent_max() const{
		return principal_tangent_vectors[0];
	}

	//returns a pointer to the 3 principal components
	inline const Dir& principal_tangent_min() const {
		return principal_tangent_vectors[1];
	}
};

class CGV_API curvature_estimator : public point_cloud_types {
	point_cloud& pc;

public:
	curvature_estimator(point_cloud& spc) : pc(spc){
	}
	void compute_unsigned_principal_curvature(std::vector<principal_curvature_estimation>& pce);

	void compute_principal_curvature(std::vector<principal_curvature_estimation>& pce);
};

#include <cgv/config/lib_end.h>