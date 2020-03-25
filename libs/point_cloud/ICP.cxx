#include "point_cloud.h"
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

void ICP::set_source_cloud(point_cloud &inputCloud) {
	sourceCloud = &inputCloud;
}

void ICP::set_target_cloud(point_cloud &inputCloud) {
	targetCloud = &inputCloud;
}

void ICP::set_iterations(int Iter) {
	this->maxIterations = Iter;
}

void ICP::set_num_random(int NR) {
	this->numRandomSamples = NR;
}

void ICP::set_eps(float e) {
	this->eps = e;
}

void ICP::reg_icp() {
	Mat rotation_mat;
	cgv::math::fvec<float, 3> translation_vec;

	point_cloud* targetPointCloudCopy = new point_cloud();

	Pnt sourceMid(0.0, 0.0, 0.0);
	Pnt targetMid(0.0, 0.0, 0.0);

	// copy the target point cloud
	for (size_t i = 0; i < targetCloud->get_nr_points(); i++)
	{
		Pnt pCopy; 
		pCopy.x() = targetCloud->pnt(i).x();
		pCopy.y() = targetCloud->pnt(i).y();
		pCopy.z() = targetCloud->pnt(i).z();
		targetPointCloudCopy->add_point(pCopy);
	}

	// create the ann tree
	ann_tree* tree = new ann_tree();
	tree->build(*targetPointCloudCopy);
	size_t num_source_points = sourceCloud->get_nr_points();

	get_mid_point(*targetCloud, targetMid);
	get_mid_point(*sourceCloud, sourceMid);

	// initialize the translation vector
	translation_vec.zeros();

	// initialize the rotation matrix
	rotation_mat.zeros();

	Pnt p;
	Pnt x;

	float cost = 1.0;
	std::srand(std::time(0));

	point_cloud_types::Pnt qs;
	point_cloud_types::Pnt qd;

	Mat fA(0.0f);             // this initializes fA to matrix filled with zeros

	cgv::math::mat<float> U, V;
	cgv::math::diag_mat<float> Sigma;
	

	for (int iter = 0; iter < maxIterations && abs(cost) > eps; iter++)
	{
		cost = 0.0;
		U.zeros();
		V.zeros();
		Sigma.zeros();
		get_mid_point(*sourceCloud, sourceMid);

		for (int i = 0; i < numRandomSamples; i++)
		{
			int randSample = std::rand() % sourceCloud->get_nr_points();
			// sample the source point cloud
			p = sourceCloud->pnt(randSample);

			// get the closest point in the target point cloud
			ann_tree::Idx Id = tree->find_closest(p);

			qs = p - sourceMid;
			qd = sourceCloud->pnt(Id) - targetMid;

			fA += Mat(sourceCloud->pnt(i), targetCloud->pnt(i));
			
			cost += error(&x, &p, rotation_mat, translation_vec);
		}
		///cast fA to A
		cgv::math::mat<float> A(3, 3, &fA(0, 0));
		cgv::math::svd(A, U, Sigma, V);
		Mat fU(3, 3, &U(0, 0)), fV(3, 3, &V(0, 0));
		rotation_mat = fU * cgv::math::transpose(fV);

		point_cloud_types::Pnt t;
		t.x() = 0.0;
		t.y() = 0.0;
		t.z() = 0.0;
		rotate(&sourceMid, rotation_mat, &t);
		translation_vec[0] = targetMid.x() - t.x();
		translation_vec[1] = targetMid.y() - t.y();
		translation_vec[2] = targetMid.z() - t.z();

		//update the point cloud
		for (unsigned int i = 0; i < sourceCloud->get_nr_points(); i++)
		{
			point_cloud_types::Pnt t = sourceCloud->pnt(i);
			rotate(&t, rotation_mat, &p);
			translate(&p, translation_vec, &t);
		}
	}
	print_rotation(rotation_mat);
	print_translation(translation_vec);
	targetPointCloudCopy->clear();
	delete tree;
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

float ICP::error(point_cloud_types::Pnt* ps, point_cloud_types::Pnt* pd, float* r, float* t)
{
	point_cloud_types::Pnt res;
	rotate(pd, r, &res);
	float err = pow(ps->x() - res.x() - t[0], 2.0);
	err += pow(ps->y() - res.y() - t[1], 2.0);
	err += pow(ps->z() - res.z() - t[2], 2.0);
	return err;
}
///print rotation matrix
void ICP::print_rotation(float *rotation) {
	std::cout << "rotation" << std::endl;
	for (int i = 0; i < 9; i = i+3) {
		std::cout << rotation[i] << " " << rotation[i + 1] << " " << rotation[i + 2] << std::endl;
	}
}
///print translation vector
void ICP::print_translation(float *translation) {
	std::cout << "translation" << std::endl;
	std::cout << translation[0] << " " << translation[1] << " " << translation[2] << std::endl;
}