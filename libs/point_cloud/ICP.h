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
	const point_cloud *targetCloud;
	int maxIterations;
	int numRandomSamples;
	float eps;

	ICP();
	~ICP();
	void set_source_cloud(const point_cloud &inputCloud);
	void set_target_cloud(const point_cloud &inputCloud);
	void set_iterations(int Iter);
	void set_num_random(int NR);
	void set_eps(float e);
	void reg_icp(Mat &rotation_m, Dir &translation_v);
	void get_center_point(const point_cloud &input, Pnt &mid_point);
	float error(Pnt &ps, Pnt &pd, Mat &r, Dir& t);
	void print_rotation(float *rotationMatrix);
	void print_translation(float *translation);
};
#include <cgv/config/lib_end.h>
