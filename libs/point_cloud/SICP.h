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
			
			enum ComputationMode {
				CM_DEFAULT = 0,
				CM_POINT_TO_POINT = 1,
				CM_POINT_TO_PLANE = 2
			};
			
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

			const point_cloud *sourceCloud;
			const point_cloud *targetCloud;
			ann_tree neighbor_tree;

			SICP();
			~SICP();
			void set_source_cloud(const point_cloud &inputCloud);
			void set_target_cloud(const point_cloud &inputCloud);
			void register_point_to_point(mat3& rotation, vec3& translation);
			void register_point_to_plane(mat3& rotation, vec3& translation);
			void register_point_cloud(ComputationMode cm, mat3& rotation, vec3& translation);

		private:
			void point_to_point(vec3* X, const vec3* Y, size_t size, mat3& rotation, vec3& translation);
			void point_to_plane(vec3* X, vec3* Y, vec3* N, const float* u, size_t size, mat3& rotation, vec3& translation);
		};

	}
}
#include <cgv/config/lib_end.h>
