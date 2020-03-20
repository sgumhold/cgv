#pragma once

#include <vector>
#include "point_cloud.h"
#include <random>
#include <ctime>
#include "SVD.h"
#include "lib_begin.h"

class CGV_API ICP {
public:
	point_cloud *sourceCloud;
	point_cloud *targetCloud;
	int maxIterations;
	int numRandomSamples;
	float eps;

	ICP();
	~ICP();
	void setSourceCloud(point_cloud &inputCloud);
	void setTargetCloud(point_cloud &inputCloud);
	void setIterations(int Iter);
	void setNumRandom(int NR);
	void setEps(int e);
	void reg_icp();
	void get_mid_point(const point_cloud &input, point_cloud_types::Pnt &mid_point);
	void clearTranslation(float* translation);
	void clearRotation(float* rotation);
	void clearMatrix(float* mat);
	void rotate(point_cloud_types::Pnt* p, float* rotationMatrix, point_cloud_types::Pnt* result);
	void translate(point_cloud_types::Pnt* p, float* translationVector, point_cloud_types::Pnt* result);
	void outerProduct(point_cloud_types::Pnt* a, point_cloud_types::Pnt* b, float* mat);
	void matrixMult(float* a, float* b, float* result);
	void transpose(float* a);
	void addMatrix(float* a, float* b, float* result);
	float error(point_cloud_types::Pnt* ps, point_cloud_types::Pnt* pd, float* r, float* t);
	void copyMatToUV(float* mat, float** result);
	void copyUVtoMat(float** mat, float* result);
	void printRotation(float *rotationMatrix);
	void printTranslation(float *translation);
};
#include <cgv/config/lib_end.h>
