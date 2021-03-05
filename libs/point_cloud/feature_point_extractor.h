#pragma once

#include <vector>
#include "point_cloud.h"
#include "ann_tree.h"
#include <random>
#include <ctime>
#include <cgv/math/svd.h> 
#include "lib_begin.h"

class CGV_API feature_points_extractor : public point_cloud_types {
		public:
			const point_cloud* sourceCloud;

			feature_points_extractor();
			~feature_points_extractor();

			void get_feature_points(const point_cloud& source_pc, int n, float nml_threshold, point_cloud& output_cloud);
			float get_nml_deviation(const Nml& a, const Nml& b);

		private:
			std::shared_ptr<ann_tree> tree;
};
#include <cgv/config/lib_end.h>
