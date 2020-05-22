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
			typedef cgv::math::fvec<float, 3> vec3;
			typedef cgv::math::fvec<float, 4> vec4;
			typedef cgv::math::fmat<float, 3,3> mat3;
			typedef cgv::math::fmat<float, 4,4> mat4;
			struct SICPParameters {
				int max_runs;
				int max_inner_loop;
				int max_outer_loop;
				float mu;
				float max_mu;
				float p;
				float stop;
				float alpha;
				bool use_penalty;
			}parameters;

			point_cloud *sourceCloud;
			point_cloud *targetCloud;
			ann_tree neighbor_tree;

			SICP();
			~SICP();
			void set_source_cloud(point_cloud &inputCloud);
			void set_target_cloud(point_cloud &inputCloud);
			void register_pointcloud();

		private:
			void point_to_point(vec3* X, vec3* Y, size_t size);
		};

	}
}
#include <cgv/config/lib_end.h>
