#include "curvature_estimator.h"
#include "ann_tree.h"
#include "neighbor_graph.h"
#include "normal_estimator.h"
#include "pca.h"

using namespace cgv::pointcloud;

void curvature_estimator::compute_principal_curvature(std::vector<Dir>& pcv, std::vector<Dir>& pc1, std::vector<Dir>& pc2)
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
	std::vector<point_cloud::point_cloud_types::Nml> normals;
	pc1.resize(num_points); pc2.resize(num_points);

	for (int i = 0; i < num_points; ++i) {
		neighborhood.extract_neighbors(i, k, close_pnts);
		normals.clear();
		for (int j = 0; j < close_pnts.size(); ++j) {
			// TODO Project normals into the tangent plane
			normals.push_back(pc.nml(close_pnts[j]));
		}
		pca3<float> analysis = pca3<float>(normals.data(), k);
		auto* eigenvalues = analysis.eigen_values_ptr();
		//TODO extract principal curvature
	}
}

