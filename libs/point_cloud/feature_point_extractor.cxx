#include "feature_point_extractor.h"


feature_points_extractor::feature_points_extractor() {
			sourceCloud = nullptr;
}

feature_points_extractor::~feature_points_extractor() {

}


void feature_points_extractor::get_feature_points(const point_cloud& source_pc, int n, float nml_threshold, point_cloud& output_cloud)
{
	std::vector<const Pnt*> knn;
	std::vector<Idx> N;
	ann_tree* tree = new ann_tree();
	tree->build(source_pc);
	for (int i = 0; i < source_pc.get_nr_points(); ++i)
	{
		tree->extract_neighbors(i, n, N);
		if (source_pc.has_normals())
		{
			float deviation = 0.0;
			for (int j = 0; j < n; ++j)
				deviation += get_nml_deviation(source_pc.nml(i), source_pc.nml(N.at(j)));
			deviation /= n;
			if (deviation < nml_threshold)
			{
				output_cloud.add_point(source_pc.pnt(i), source_pc.nml(i));
			}
		}
	}
}

float feature_points_extractor::get_nml_deviation(const Nml& a, const Nml& b)
		{
			float d = (a.x() * b.x() + a.y() * b.y() + a.z() * b.z()) / (sqrt(pow(a.x(), 2) + pow(a.y(), 2) + pow(a.z(), 2)) * sqrt(pow(b.x(), 2) + pow(b.y(), 2) + pow(b.z(), 2)));
			return d;
		}