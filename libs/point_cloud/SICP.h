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

namespace cgv {

	namespace pointcloud {

		// Sparse ICP using Alternate Direction Method of Multipliers
		class CGV_API SICP : public point_cloud_types {
		public:
			struct SICPParameters {
				int max_runs;
				int max_inner_loop;
				int max_outer_loop;
				float mu;
				float p;
			}parameters;

			const point_cloud *sourceCloud;
			const point_cloud *targetCloud;
			ann_tree neighbor_tree;

			SICP();
			~SICP();
			void set_source_cloud(const point_cloud &inputCloud);
			void set_target_cloud(const point_cloud &inputCloud);
			void get_center_point(const point_cloud &input, Pnt &center_point);
			void register_pointcloud();
		};

	}
}
#include <cgv/config/lib_end.h>
