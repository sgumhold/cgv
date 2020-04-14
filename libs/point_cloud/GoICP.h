#pragma once
#include <vector>
#include "point_cloud.h"
#include <random>
#include <ctime>
#include <cgv/math/svd.h>


#include "lib_begin.h"

class CGV_API goicp : public point_cloud_types{
	typedef cgv::math::fvec<float, 3> vec3;
public:
	point_cloud *sourceCloud;
	const point_cloud *targetCloud;

	Mat rotation;
	Dir translation;

	void set_source_cloud(point_cloud &inputCloud);
	void set_target_cloud(const point_cloud &inputCloud);
};

#include <cgv/config/lib_end.h>