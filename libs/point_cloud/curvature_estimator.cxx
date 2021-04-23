#include "curvature_estimator.h"
#include "ann_tree.h"
#include "neighbor_graph.h"
#include "normal_estimator.h"
#include "pca.h"

namespace {
	// describes a surface defined by a quadratic equation with two parameters
	struct quadric {
		using vec3 = cgv::math::fvec<float, 3>;
		
		/// coefficients for ax²+by²+c*xy+dx+ey
		double a, b, c, d, e;

		quadric() = default;
		quadric(double pa,double pb, double pc, double pd, double pe) : a(pa),b(pb), c(pc),d(pd),e(pe){}

		double evaluate(double u, double v)
		{
			return a * u * u + b * u * v + c * v * v + d * u + e * v;
		}

		double du(const double u, const double v) {
			return 2.0 * a * u + b * v + d;
		}

		double duu(const double u, const double v) {
			return 2.0 * a;
		}

		double dv(const double u, const double v) {
			return 2.0 * c * v + b * u + e;
		}

		double dvv(const double u, const double v) {
			return 2.0 * c;
		}

		double duv(const double u, const double v) {
			return b;
		}
		
		static quadric fit(const std::vector<vec3>& pnts)
		{
			assert(pnts.size() >= 5);
			size_t num_pnts = pnts.size();
			cgv::math::mat<double> A(num_pnts,5);
			cgv::math::mat<double> b(num_pnts,1);
			cgv::math::mat<double> x(5, 1); //solution vector
			
			//copy points to A and b
			for (int c = 0; c < num_pnts; ++c)
			{
				double u = pnts[c][0];
				double v = pnts[c][1];
				double y = pnts[c][2];

				A(c, 0) = u * u;
				A(c, 1) = u * v;
				A(c, 2) = v * v;
				A(c, 3) = u;
				A(c, 4) = v;

				b(c,0) = y;
			}

			//cgv::math::mat<double> U, V;
			//cgv::math::diag_mat<double> sigma;
			//cgv::math::svd(A, U, sigma, V, true);
			//solve for b
			
			auto pseudo_inverse_A = cgv::math::pseudo_inv(A);
			x = pseudo_inverse_A * b;
			return quadric(x(0,0),x(1,0),x(2,0),x(3,0),x(4,0));
		}
	};
}

using namespace cgv::pointcloud;

void curvature_estimator::compute_unsigned_principal_curvature(std::vector<principal_curvature_estimation>& pce)
{
	//curvature estimation for points with given normal
	static constexpr int k = 15;
	int num_points = pc.get_nr_points();
	ann_tree neighborhood;
	neighborhood.build(pc);
	neighbor_graph graph;
	graph.build(num_points, k, neighborhood);

	if (!pc.has_normals()) {
		normal_estimator normi = normal_estimator(pc, graph);
		normi.compute_plane_bilateral_weighted_normals(true);
	}
	
	std::vector<point_cloud::point_cloud_types::Idx> close_pnts;
	std::vector<point_cloud::point_cloud_types::Nml> projected_normals;
	pce.resize(num_points);

	for (int i = 0; i < num_points; ++i) {
		projected_normals.clear();
		const Dir& point_normal = pc.nml(i);
		neighborhood.extract_neighbors(i, k, close_pnts);
		Mat projection_matrix;
		projection_matrix.identity();
		projection_matrix -= Mat(point_normal, point_normal);

		for (int j = 0; j < close_pnts.size(); ++j) {
			// Project normals into the tangent plane
			Dir projected_normal = pc.nml(close_pnts[j])*projection_matrix;
			projected_normals.push_back(projected_normal);
		}

		pca3<float> analysis = pca3<float>(projected_normals.data(), k);
		auto* eigenvalues = analysis.eigen_values_ptr();
		auto* eigenvectors = analysis.eigen_vectors_ptr();
		//store the two largest eigenvalues as prinicpal curvature
		double kf = 1.0 / k;
		float curv1 = eigenvalues[0] * kf;
		float curv2 = eigenvalues[1] * kf;
		pce[i] = principal_curvature_estimation(eigenvectors[0], curv1, curv2);
	}
}
void curvature_estimator::compute_principal_curvature(std::vector<principal_curvature_estimation>& pce)
{
	//TODO fit quadratic ax²+by²+cxy+dx+ey to heighfield of neighborhood
	// 
	//curvature estimation for points with given normal
	static constexpr int k = 15;
	int num_points = pc.get_nr_points();
	ann_tree neighborhood;
	neighborhood.build(pc);
	neighbor_graph graph;
	graph.build(num_points, k, neighborhood);

	if (!pc.has_normals()) {
		normal_estimator normi = normal_estimator(pc, graph);
		normi.compute_plane_bilateral_weighted_normals(true);
	}

	std::vector<point_cloud::point_cloud_types::Idx> close_pnts;
	pce.resize(num_points);

	for (int i = 0; i < num_points; ++i) {
		
		//build quadratics
		auto point_quadtratics = cgv::math::mat<float>(5, k);
		neighborhood.extract_neighbors(i, k, close_pnts);

	}
}
/*
void curvature_estimator::compute_principal_curvature(std::vector<principal_curvature_estimation>& pce)
{
	//curvature estimation for points with given normal
	static constexpr int k = 15;
	int num_points = pc.get_nr_points();
	ann_tree neighborhood;
	neighborhood.build(pc);
	neighbor_graph graph;
	graph.build(num_points, k, neighborhood);

	if (!pc.has_normals()) {
		normal_estimator normi = normal_estimator(pc, graph);
		normi.compute_plane_bilateral_weighted_normals(true);
	}

	std::vector<point_cloud::point_cloud_types::Idx> close_pnts;
	std::vector<point_cloud::point_cloud_types::Nml> projected_normals;
	pce.resize(num_points);

	for (int i = 0; i < num_points; ++i) {
		projected_normals.clear();
		const Dir& point_normal = pc.nml(i);
		neighborhood.extract_neighbors(i, k, close_pnts);
		Mat projection_matrix;
		projection_matrix.identity();
		projection_matrix -= Mat(point_normal, point_normal);

		for (int j = 0; j < close_pnts.size(); ++j) {
			// Project normals into the tangent plane
			Dir projected_normal = pc.nml(close_pnts[j]) * projection_matrix;
			projected_normals.push_back(projected_normal);
		}

		pca3<float> analysis = pca3<float>(projected_normals.data(), k);
		auto* eigenvalues = analysis.eigen_values_ptr();
		auto* eigenvectors = analysis.eigen_vectors_ptr();
		//store the two largest eigenvalues as prinicpal curvature
		double kf = 1.0 / k;
		float curv1 = cgv::math::dot(point_normal, eigenvectors[0]) > 0.f ? eigenvalues[0] * kf : -eigenvalues[0] * kf;
		float curv2 = cgv::math::dot(point_normal, eigenvectors[1]) > 0.f ? eigenvalues[1] * kf : -eigenvalues[1] * kf;
		if (curv1 > curv2)
			pce[i] = principal_curvature_estimation(eigenvectors[0], curv1, curv2);
		else
			pce[i] = principal_curvature_estimation(eigenvectors[0], curv2, curv1);
	}
}*/