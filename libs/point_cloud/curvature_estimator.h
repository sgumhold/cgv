#pragma once

#include "point_cloud.h"
#include <array>

#include "lib_begin.h"

class CGV_API curvature_estimator : public point_cloud_types {
	point_cloud& pc;

protected:
	void compute_principal_curvature(std::vector<Dir>& pcv,std::vector<Dir>& pc1, std::vector<Dir>& pc2);

public:
	curvature_estimator(point_cloud& spc) : pc(spc){

	}
};

#include <cgv/config/lib_end.h>