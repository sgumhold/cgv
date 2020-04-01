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

void ICP::set_source_cloud(const point_cloud &inputCloud) {
	sourceCloud = &inputCloud;
}

void ICP::set_target_cloud(const point_cloud &inputCloud) {
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

///output the rotation matrix and translation vector
void ICP::reg_icp(Mat& rotation_mat, Dir& translation_vec) {
	Pnt source_center;
	Pnt target_center;
	source_center.zeros();
	target_center.zeros();
	/// create the ann tree
	ann_tree* tree = new ann_tree();
	tree->build(*targetCloud);
	size_t num_source_points = sourceCloud->get_nr_points();

	get_center_point(*targetCloud, target_center);
	get_center_point(*sourceCloud, source_center);
	/// initialize the translation vector
	translation_vec.zeros();
	/// initialize the rotation matrix
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
		get_center_point(*sourceCloud, source_center);

		for (int i = 0; i < numRandomSamples; i++)
		{
			int randSample = std::rand() % sourceCloud->get_nr_points();
			/// sample the source point cloud
			p = sourceCloud->pnt(randSample);
			/// get the closest point in the target point cloud
			ann_tree::Idx Id = tree->find_closest(p);

			qs = p - source_center;
			qd = sourceCloud->pnt(Id) - target_center;

			fA += Mat(sourceCloud->pnt(i), targetCloud->pnt(i));
			
			cost += error(x, p, rotation_mat, translation_vec);
		}
		///cast fA to A
		cgv::math::mat<float> A(3, 3, &fA(0, 0));
		cgv::math::svd(A, U, Sigma, V);
		Mat fU(3, 3, &U(0, 0)), fV(3, 3, &V(0, 0));
		rotation_mat = fU * cgv::math::transpose(fV);
		///calculate rotation matrix and translation vector
		point_cloud_types::Pnt t;
		t.zeros();
		t = rotation_mat * source_center;
		translation_vec = target_center - t;

		//update the point cloud, the source point cloud will be changed in updating
		/*for (unsigned int i = 0; i < sourceCloud->get_nr_points(); i++)
		{
			rotate(&sourceCloud->pnt(i), rotation_mat, &p);
			translate(&p, translation_vec, &sourceCloud->pnt(i));
		}*/
	}
	delete tree;
}

void ICP::get_center_point(const point_cloud &input, Pnt &center_point) {
	center_point.zeros();
	for (unsigned int i = 0; i < input.get_nr_points(); i++)
		center_point += input.pnt(i);
	center_point /= (float)input.get_nr_points();
}

float ICP::error(Pnt &ps, Pnt &pd, Mat &r, Dir &t)
{
	Pnt res;
	res = r * pd;
	float err = pow(ps.x() - res.x() - t[0], 2.0);
	err += pow(ps.y() - res.y() - t[1], 2.0);
	err += pow(ps.z() - res.z() - t[2], 2.0);
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