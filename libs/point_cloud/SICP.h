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

		//Enumerator for setting the underlying ICP method used
		enum IcpMethod { pointToPoint, pointToPlane };

		class CGV_API SICP : public point_cloud_types {
		public:
			struct SICPParameters {
				int max_runs;
				int max_inner_loop;
				int max_outer_loop;
				float mu;
			}parameters;

			const point_cloud *sourceCloud;
			const point_cloud *targetCloud;
			point_cloud closest_points;
			ann_tree neighbor_tree;

			SICP();
			~SICP();
			void set_source_cloud(const point_cloud &inputCloud);
			void set_target_cloud(const point_cloud &inputCloud);
			void get_center_point(const point_cloud &input, Pnt &center_point);
			void register_pointcloud(float mu=0.00001f, int max_runs=1000);
		};

	}
}
#include <cgv/config/lib_end.h>
