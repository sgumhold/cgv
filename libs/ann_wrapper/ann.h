#pragma once

#include <cgv/math/vec.h>
#include <cgv/math/mat.h>
#include <ANN/ANN.h>

#include "lib_begin.h"

namespace cgv{
	namespace math {

///wrapper to simplify the usage of ann lib on points stored
///as column in a matrix
struct CGV_API ann
{
private:
	double eps; // error bound
	unsigned maxPts; // maximum number of data points
	ANNpointArray dataPts; // data points
	ANNkd_tree* kdTree;
public:
	ann();
	ann(mat<double>& points, double eps=0);
	void setup(mat<double>& points, double eps=0);
	void setup(ANNpointArray data_points,unsigned n,unsigned dim=3, double eps=0);
	bool is_setup();
	~ann();
	void query_knn(double x, double y, double z, int k, int *indices, double *sqrdists=NULL);
	///query the number of points in the sphere with center (x,y,z) and radius "radius"
	int query_num_points_in_FR(double x, double y, double z,double radius);
	int query_num_points_in_FR(const math::vec<double> &query_point, double radius);
	void query_fixed_radius(double x, double y, double z,double radius, int k, int *indices,double *sqrdists=NULL);
	void query_fixed_radius(const math::vec<double> &query_point,double radius, int k, int *indices,double *sqrdists=NULL);
	void query_knn(const math::vec<double> &query_point, int k, int *indices, double *sqrdists =NULL);
	void query_knn(const math::vec<double> &query_point,int k, math::mat<double>& result_points);
	int query_fixed_radius(const math::vec<double> &query_point, double radius,math::mat<double>& result_points);
	unsigned num_points();
	void query_knn(unsigned i,int k, math::mat<double>& result_points);
};


	}
}

#include <cgv/config/lib_end.h>