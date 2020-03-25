#pragma once

#include <vector>
#include "point_cloud.h"
#include <random>
#include <ctime>
#include <cgv/math/svd.h> 
#include "lib_begin.h"

class CGV_API ICP : public point_cloud_types{
public:
	const point_cloud *sourceCloud;
	point_cloud *targetCloud;
	int maxIterations;
	int numRandomSamples;
	float eps;

	ICP();
	~ICP();
	void set_source_cloud(point_cloud &inputCloud);
	void set_target_cloud(point_cloud &inputCloud);
	void set_iterations(int Iter);
	void set_num_random(int NR);
	void set_eps(float e);
	void reg_icp();
	void get_mid_point(const point_cloud &input, point_cloud_types::Pnt &mid_point);
	void rotate(point_cloud_types::Pnt* p, float* rotationMatrix, point_cloud_types::Pnt* result);
	void translate(point_cloud_types::Pnt* p, float* translationVector, point_cloud_types::Pnt* result);
	float error(point_cloud_types::Pnt* ps, point_cloud_types::Pnt* pd, float* r, float* t);
	void print_rotation(float *rotationMatrix);
	void print_translation(float *translation);
};
#include <cgv/config/lib_end.h>
