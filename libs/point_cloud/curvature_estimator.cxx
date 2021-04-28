#include "curvature_estimator.h"
#include "ann_tree.h"
#include "neighbor_graph.h"
#include "normal_estimator.h"
#include "pca.h"

#include <cgv/math/inv.h>

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
		pce[i] = principal_curvature_estimation(curv1, curv2, eigenvectors[0], eigenvectors[1]);
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
	std::vector<point_cloud::point_cloud_types::Pnt> plane_projected_points;
	;
	pce.resize(num_points);

	for (int i = 0; i < num_points; ++i) {
		const Pnt& selected_point = pc.pnt(i);
		Mat projection_matrix; // point to plane projection
		Mat plane_reference_matrix; // point to plane projection

		//build quadratics
		auto point_quadtratics = cgv::math::mat<float>(5, k);
		neighborhood.extract_neighbors(i, k, close_pnts);

		// project point into normal plane
		Dir& point_normal = pc.nml(i);
		projection_matrix.identity();
		projection_matrix -= Mat(point_normal, point_normal);
		
		Dir y_axis = cgv::math::normalize(projection_matrix * pc.pnt(close_pnts[close_pnts.size()-1]));
		Dir x_axis = cgv::math::cross(y_axis, point_normal);
		plane_reference_matrix.set_row(0, x_axis);
		plane_reference_matrix.set_row(1, y_axis);
		plane_reference_matrix.set_row(2, point_normal);
		Mat inv_plane_reference_matrix = cgv::math::inv(plane_reference_matrix);

		// change reference system, z is distance to plane, and x,y span the tangent space
		plane_projected_points.clear();
		for (const Idx& point_idx : close_pnts) {
			plane_projected_points.push_back(plane_reference_matrix * (pc.pnt(point_idx)- selected_point));
		}
		// fit quadric
		quadric q = quadric::fit(plane_projected_points);

		// compute first fundamental form of the quadric
		double inv_sqrt_ab = 1.0 / (sqrt(q.a * q.a + q.b * q.b + 1.0));
		double E = 1.0 + q.a*q.a;
		double F = q.a*q.b;
		double G = 1.0 + q.b * q.b;
		// compute secound fundamental form
		double L = (2.0 * q.c) * inv_sqrt_ab;
		double M = q.d * inv_sqrt_ab;
		double N = (2.0 * q.e) * inv_sqrt_ab;
		// reshape
		cgv::math::mat<double> FFF = cgv::math::mat<double>(2, 2);
		cgv::math::mat<double> SFF = cgv::math::mat<double>(2, 2);
		FFF(0, 0) = E;
		FFF(0,1) = FFF(1,0) = F;
		FFF(1,1) = G;

		SFF(0, 0) = L;
		SFF(0, 1) = SFF(1,0) = M;
		SFF(1, 1) = N;
		// calculate the shape operator of the quadric
		cgv::math::mat<double> S = -SFF * cgv::math::inv_22(FFF);
		
		// compute principal curvatures
		cgv::math::mat<double> U, V;
		cgv::math::diag_mat<double> s;
		cgv::math::svd(S, U, s, V,false);

		std::array<Dir, 2> eigen_vectors;
		// columns of U contain vectors spanning the eigenspace and sigma contains the eigen values
		for (int i = 0; i < 2; ++i) {
			eigen_vectors[i] = Dir(0.0,0.0,0.0);
			for (int j = 0; j < 2; ++j) {
				eigen_vectors[i](j) = (float) U(j, i);
			}
		}
		// transform back to world coordinates
		for (auto& v : eigen_vectors) {
			v = inv_plane_reference_matrix * v;
		}
		//evaluate q at point p
		if (q.duu(0, 0) < 0) {
			s[0] = -s[0];
		}
		if (q.dvv(0, 0) < 0) {
			s[1] = -s[1];
		}
		int min_curv_ix = (s[0] < s[1]) ? 0 : 1;
		int max_curv_ix = (s[0] < s[1]) ? 1 : 0;
		pce[i] = principal_curvature_estimation(s[max_curv_ix], s[min_curv_ix], eigen_vectors[max_curv_ix], eigen_vectors[min_curv_ix]);
	}
}
