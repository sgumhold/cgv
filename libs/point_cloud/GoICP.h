/*
Implementation of Go-ICP (see http://jlyang.org/go-icp/)
WIP
*/
#pragma once
#include <vector>
#include "point_cloud.h"
#include <random>
#include <ctime>
#include <vector>
#include <cgv/math/svd.h>
#include <memory>
#include "3ddt.h"
#include "ICP.h"


#include "lib_begin.h"

namespace cgv {

	namespace pointcloud {

		struct rotation_node
		{
			float a, b, c, w; // (a,b,c) angle axis representation ,w: side length of rotation subcube
			float ub, lb; //upper bound, lower bound
			int l; //level
			friend bool operator<(const struct rotation_node & node_a, const struct rotation_node & node_b)
			{
				if (node_a.lb != node_b.lb)
					return node_a.lb > node_b.lb;
				else
					return node_a.w < node_b.w;
			}
		};

		struct translation_node
		{
			float x, y, z, w; //translation
			float ub, lb; //upper bound, lower bound
			friend bool operator < (const translation_node & noda_a, const translation_node & node_b)
			{
				if (noda_a.lb != node_b.lb)
					return noda_a.lb > node_b.lb;
				else
					return noda_a.w < node_b.w;
			}
		};

		class CGV_API GoICP : public point_cloud_types {
			typedef cgv::math::fvec<float, 3> vec3;
			typedef cgv::math::fmat<float, 4, 4> mat4;
			typedef cgv::math::fmat<float, 3, 3> mat3;
		public:
			static const int max_rot_level = 20;

			GoICP();
			~GoICP();

			inline void set_source_cloud(const point_cloud &inputCloud) {
				source_cloud = &inputCloud;
				sample_size = source_cloud->get_nr_points();
			}
			inline void set_target_cloud(const point_cloud &inputCloud) {
				target_cloud = &inputCloud;
			};

			inline void set_sample_size(int num) {
				sample_size = num;
			}

			inline float register_pointcloud()
			{
				initialize();
				outerBnB();
				clear();
				return optimal_error;
			}
			void initialize();
			void clear();
		protected:
			void buildDistanceTransform();
			float icp(mat3& R_icp, vec3& t_icp);
			void outerBnB();
			float innerBnB(float* maxRotDisL, translation_node* nodeTransOut);
		private:
			const point_cloud *source_cloud;
			const point_cloud *target_cloud;
			point_cloud temp_source_cloud;
			int sample_size; // < source cloud size

			Mat rotation;
			Dir translation;
			std::vector<float> norm_data;
			DT3D distance_transform;
			ICP icp_obj;

			float** max_rot_dis; //rotation uncertainity radius
			std::vector<float> min_dis;
			int inlier_num;

			rotation_node init_rot_node,optimal_rot_node;
			translation_node init_trans_node, optimal_trans_node;

		public:
			mat3 optimal_rotation;
			vec3 optimal_translation;
			float optimal_error;
			//settings
			float mse_threshhold,sse_threshhold;
			float trim_fraction;
			float distance_transform_size;
			float distance_transform_expand_factor;
			bool do_trim;
		};

	}
}
#include <cgv/config/lib_end.h>