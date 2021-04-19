#include "curvature_estimator.h"
#include "ann_tree.h"
#include "neighbor_graph.h"
#include "normal_estimator.h"
#include "pca.h"

using namespace cgv::pointcloud;

void curvature_estimator::compute_principal_curvature(std::vector<principal_curvature_estimation>& pce)
{
	static constexpr int k = 20;
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
		pce[i] = principal_curvature_estimation(eigenvectors[0], eigenvalues[0], eigenvalues[1]);
	}
}

