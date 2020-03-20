#include <cgv/math/permute.h>
#include <cgv/math/det.h>
#include "point_cloud.h"
#include <cgv/utils/file.h>
#include <cgv/utils/stopwatch.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/media/mesh/obj_reader.h>
#include <fstream>
#include "ann_tree.h"
#include "ICP.h"

ICP::ICP() {
	sourceCloud = new point_cloud();
	targetCloud = new point_cloud();
	this->maxIterations = 400;
	this->numRandomSamples = 400;
	this->eps = 1e-8;
}

ICP::~ICP() {

}

void ICP::setSourceCloud(point_cloud &inputCloud) {
	sourceCloud = &inputCloud;
}

void ICP::setTargetCloud(point_cloud &inputCloud) {
	targetCloud = &inputCloud;
}

void ICP::setIterations(int Iter) {
	this->maxIterations = Iter;
}

void ICP::setNumRandom(int NR) {
	this->numRandomSamples = NR;
}

void ICP::setEps(int e) {
	this->eps = e;
}

void ICP::reg_icp() {
	float rotationMatrix[9];
	float translation[3];

	point_cloud* staticPointCloudCopy = new point_cloud();

	point_cloud_types::Pnt dynamicMid(0.0, 0.0, 0.0);
	point_cloud_types::Pnt staticMid(0.0, 0.0, 0.0);

	// copy the static point cloud
	for (unsigned int i = 0; i < targetCloud->get_nr_points(); i++)
	{
		point_cloud_types::Pnt pCopy; 
		pCopy.x() = targetCloud->pnt(i).x();
		pCopy.y() = targetCloud->pnt(i).y();
		pCopy.z() = targetCloud->pnt(i).z();
		staticPointCloudCopy->add_point(pCopy);
	}

	// create the kd tree
	ann_tree* tree = new ann_tree();
	tree->build(*sourceCloud);
	size_t numDynamicPoints = sourceCloud->get_nr_points();

	get_mid_point(*targetCloud, staticMid);
	get_mid_point(*sourceCloud, dynamicMid);

	// initialize the translation vector
	clearTranslation(translation);

	// initialize the rotation matrix
	clearRotation(rotationMatrix);

	point_cloud_types::Pnt p;
	point_cloud_types::Pnt x;

	float cost = 1.0;
	std::srand(std::time(0));

	point_cloud_types::Pnt qd;
	point_cloud_types::Pnt qs;

	float U[9];
	float w[9];
	float sigma[3];
	float V[9];

	float** uSvd = new float*[3];
	float** vSvd = new float*[3];
	uSvd[0] = new float[3];
	uSvd[1] = new float[3];
	uSvd[2] = new float[3];

	vSvd[0] = new float[3];
	vSvd[1] = new float[3];
	vSvd[2] = new float[3];

	for (int iter = 0; iter < maxIterations && abs(cost) > eps; iter++)
	{
		cost = 0.0;

		//clearRotation(rotationMatrix);
		clearMatrix(U);
		clearMatrix(V);
		clearMatrix(w);
		get_mid_point(*sourceCloud, dynamicMid);

		for (int i = 0; i < numRandomSamples; i++)
		{
			int randSample = std::rand() % sourceCloud->get_nr_points();
			// sample the dynamic point cloud
			p = sourceCloud->pnt(randSample);

			// get the closest point in the static point cloud
			ann_tree::Idx Id = tree->find_closest(p);

			qd = p - dynamicMid;
			qs = sourceCloud->pnt(Id) - staticMid;

			outerProduct(&qs, &qd, w);
			addMatrix(w, U, U);

			cost += error(&x, &p, rotationMatrix, translation);
		}
		copyMatToUV(U, uSvd);
		dsvd(uSvd, 3, 3, sigma, vSvd);
		copyUVtoMat(uSvd, U);
		copyUVtoMat(vSvd, V);

		transpose(V);
		matrixMult(U, V, rotationMatrix);

		point_cloud_types::Pnt t;
		t.x() = 0.0;
		t.y() = 0.0;
		t.z() = 0.0;
		rotate(&dynamicMid, rotationMatrix, &t);
		translation[0] = staticMid.x() - t.x();
		translation[1] = staticMid.y() - t.y();
		translation[2] = staticMid.z() - t.z();

		//update the point cloud
		for (unsigned int i = 0; i < sourceCloud->get_nr_points(); i++)
		{
			point_cloud_types::Pnt t = sourceCloud->pnt(i);
			rotate(&t, rotationMatrix, &p);
			translate(&p, translation, &t);
		}
		printRotation(rotationMatrix);
		printTranslation(translation);
	}

	staticPointCloudCopy->clear();
	delete tree;

	delete[] uSvd[0];
	delete[] uSvd[1];
	delete[] uSvd[2];
	delete[] uSvd;

	delete[] vSvd[0];
	delete[] vSvd[1];
	delete[] vSvd[2];
	delete[] vSvd;
}

void ICP::get_mid_point(const point_cloud &input, point_cloud_types::Pnt &mid_point) {
	mid_point.x() = 0.0;
	mid_point.y() = 0.0;
	mid_point.z() = 0.0;
	for (unsigned int i = 0; i < input.get_nr_points(); i++)
	{
		mid_point.x() += input.pnt(i).x();
		mid_point.y() += input.pnt(i).y();
		mid_point.z() += input.pnt(i).z();
	}
	mid_point.x() = mid_point.x() / (float)input.get_nr_points();
	mid_point.y() = mid_point.y() / (float)input.get_nr_points();
	mid_point.z() = mid_point.z() / (float)input.get_nr_points();
}

void ICP::clearTranslation(float* translation)
{
	translation[0] = 0.0;
	translation[1] = 0.0;
	translation[2] = 0.0;
}

void ICP::clearRotation(float* rotation)
{
	rotation[0] = 1.0;
	rotation[1] = 0.0;
	rotation[2] = 0.0;

	rotation[3] = 0.0;
	rotation[4] = 1.0;
	rotation[5] = 0.0;

	rotation[6] = 0.0;
	rotation[7] = 0.0;
	rotation[8] = 1.0;
}

void ICP::clearMatrix(float* mat)
{
	mat[0] = 0.0;
	mat[1] = 0.0;
	mat[2] = 0.0;

	mat[3] = 0.0;
	mat[4] = 0.0;
	mat[5] = 0.0;

	mat[6] = 0.0;
	mat[7] = 0.0;
	mat[8] = 0.0;
}

void ICP::rotate(point_cloud_types::Pnt* p, float* rotationMatrix, point_cloud_types::Pnt* result)
{
	result->x() = p->x() * rotationMatrix[0] + p->y() * rotationMatrix[1] + p->z() * rotationMatrix[2];
	result->y() = p->x() * rotationMatrix[3] + p->y() * rotationMatrix[4] + p->z() * rotationMatrix[5];
	result->z() = p->x() * rotationMatrix[6] + p->y() * rotationMatrix[7] + p->z() * rotationMatrix[8];
}

void ICP::translate(point_cloud_types::Pnt* p, float* translationVector, point_cloud_types::Pnt* result)
{
	result->x() = p->x() + translationVector[0];
	result->y() = p->y() + translationVector[1];
	result->z() = p->z() + translationVector[2];
}

void ICP::outerProduct(point_cloud_types::Pnt* a, point_cloud_types::Pnt* b, float* mat)
{
	mat[0] = a->x() * b->x();
	mat[1] = a->x() * b->y();
	mat[2] = a->x() * b->z();

	mat[3] = a->y() * b->x();
	mat[4] = a->y() * b->y();
	mat[5] = a->y() * b->z();

	mat[6] = a->z() * b->x();
	mat[7] = a->z() * b->y();
	mat[8] = a->z() * b->z();
}

void ICP::matrixMult(float* a, float* b, float* result)
{
	result[0] = a[0] * b[0] + a[1] * b[3] + a[2] * b[6];
	result[1] = a[0] * b[1] + a[1] * b[4] + a[2] * b[7];
	result[2] = a[0] * b[2] + a[1] * b[5] + a[2] * b[8];

	result[3] = a[3] * b[0] + a[4] * b[3] + a[5] * b[6];
	result[4] = a[3] * b[1] + a[4] * b[4] + a[5] * b[7];
	result[5] = a[3] * b[2] + a[4] * b[5] + a[5] * b[8];

	result[6] = a[6] * b[0] + a[7] * b[3] + a[8] * b[6];
	result[7] = a[6] * b[1] + a[7] * b[4] + a[8] * b[7];
	result[8] = a[6] * b[2] + a[7] * b[5] + a[8] * b[8];
}

void ICP::transpose(float* a)
{
	float temp;

	temp = a[1];
	a[1] = a[3];
	a[3] = temp;

	temp = a[2];
	a[2] = a[6];
	a[6] = temp;

	temp = a[5];
	a[5] = a[7];
	a[7] = temp;
}

void ICP::addMatrix(float* a, float* b, float* result)
{
	result[0] = a[0] + b[0];
	result[1] = a[1] + b[1];
	result[2] = a[2] + b[2];

	result[3] = a[3] + b[3];
	result[4] = a[4] + b[4];
	result[5] = a[5] + b[5];

	result[6] = a[6] + b[6];
	result[7] = a[7] + b[7];
	result[8] = a[8] + b[8];
}

float ICP::error(point_cloud_types::Pnt* ps, point_cloud_types::Pnt* pd, float* r, float* t)
{
	point_cloud_types::Pnt res;
	rotate(pd, r, &res);
	float err = pow(ps->x() - res.x() - t[0], 2.0);
	err += pow(ps->y() - res.y() - t[1], 2.0);
	err += pow(ps->z() - res.z() - t[2], 2.0);
	return err;
}

void ICP::copyMatToUV(float* mat, float** result)
{
	result[0][0] = mat[0];
	result[0][1] = mat[1];
	result[0][2] = mat[2];

	result[1][0] = mat[3];
	result[1][1] = mat[4];
	result[1][2] = mat[5];

	result[2][0] = mat[6];
	result[2][1] = mat[7];
	result[2][2] = mat[8];
}

void ICP::copyUVtoMat(float** mat, float* result)
{
	result[0] = mat[0][0];
	result[1] = mat[0][1];
	result[2] = mat[0][2];

	result[3] = mat[1][0];
	result[4] = mat[1][1];
	result[5] = mat[1][2];

	result[6] = mat[2][0];
	result[7] = mat[2][1];
	result[8] = mat[2][2];
}

void ICP::printRotation(float *rotation) {
	std::cout << "rotation" << std::endl;
	for (int i = 0; i < 9; i = i+3) {
		std::cout << rotation[i] << " " << rotation[i + 1] << " " << rotation[i + 2] << std::endl;
	}
}

void ICP::printTranslation(float *translation) {
	std::cout << "translation" << std::endl;
	std::cout << translation[0] << " " << translation[1] << " " << translation[2] << std::endl;
}