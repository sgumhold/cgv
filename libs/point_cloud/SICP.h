#pragma once

#include <vector>
#include <random>
#include <ctime>
#include <iostream>
#include <iomanip> 
#include <cgv/math/svd.h> 
#include "point_cloud.h"
#include "normal_estimator.h"
#include "lib_begin.h"

//Enumerator for setting the underlying ICP method used
enum IcpMethod { pointToPoint, pointToPlane };

class CGV_API SICP : public point_cloud_types{
public:
	const point_cloud *sourceCloud;
	const point_cloud *targetCloud;
	point_cloud* movingCloud;
	normal_estimator* nml_est;
	neighbor_graph ng;

	point_cloud *lambda; //Lagrange multiplier for step 2.1
	//Parameters
	size_t k_normals = 10;       //Number of nearest neighbours in order to estimate the normals
	int num_iterations = 25;      //Number of iterations for the algorithm
	int num_iterations_in = 2;     //Number of iterations for the step 2 of the algorithm 
	double mu = 10.0;            //Parameter for step 2.1
	int num_iter_shrink = 3;       //Number of iterations for shrink step (2.1 also)
	double p = 0.5;             //We use the norm L_p
	bool verbose = false;       //Verbosity trigger
	IcpMethod method = pointToPlane; //Underlying ICP method
	std::vector<double> iterations;
	double referenceDist;
	bool hasBeenComputed;
	//const int MAX = 100;       // for cholesky_decomposition

	SICP();
	~SICP();
	void set_source_cloud(const point_cloud &inputCloud);
	void set_target_cloud(const point_cloud &inputCloud);
	void get_center_point(const point_cloud &input, Pnt &center_point);
	int perform_sparceICP();
	std::vector<int> compute_correspondances(const point_cloud& refCloud, point_cloud& queryCloud);
	point_cloud move_pointcloud(point_cloud& pointCloud, Dir &t);
	void rigid_transform_point_to_point(point_cloud &a, point_cloud &b, Mat &rotation_m, Dir &translation_v);
	void rigid_transform_point_to_plane(point_cloud &a, point_cloud &b, point_cloud& n);
	Pnt shrink(Pnt &h);
	double norm(Dir &h);
	///Computing composition of tNew by tOld (tNew o tOld)
	void compose(Mat& r_new, Dir &t_new, Mat& r_old, Dir &t_old);
	///Selection of a subset in a PointCloud
	point_cloud select_subset_pc(point_cloud p, std::vector<int> indice);
	///Updates the iterations measure by estimating the amplitude of rigid motion t
	void update_iter(Mat &r, Dir &t);
	///Save iterations to file
	void save_iter(std::string pathToFile);
	void division_pc(point_cloud& input, double d);
	void cholesky_decomposition(int matrix[][100], int n);
};
#include <cgv/config/lib_end.h>
