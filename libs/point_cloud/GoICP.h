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
#include "ann_tree.h"


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
			/// supported methods for finding nearest neighbor correspondences outside icp. 
			/// The used icp implementation currently only uses the ann_tree
			enum DistanceComputationMode {
				DCM_DISTANCE_TRANSFORM = 0, // faster for large pointclouds, setup takes more time
				DCM_ANN_TREE = 1, // higher precision ,slower distance computation, fast setup
				DCM_NONE = -1
			} dc_mode;

			static const int max_rot_level = 20;

			GoICP();
			~GoICP();

			// requires that initializeRegistration() and initializeDistanceComputation() completed without errors
			float registerPointcloud();
			// initialize parameters for processing the source cloud
			void initializeRegistration(const point_cloud &inputCloud);
			// initialize and build data structures needed for the distance computation method set by setDistanceComputationMode
			void initializeDistanceComputation(const point_cloud &inputCloud);
			// free additional allocated memory
			void clear();
			// set computation mode, for possible values see DistanceComputationMode
			inline void setDistanceComputationMode(DistanceComputationMode dcm) {
				dc_mode = dcm;
			}
		protected:
			template<GoICP::DistanceComputationMode DCM>
			void outerBnB();
			template<GoICP::DistanceComputationMode DCM>
			float innerBnB(float * max_rot_distance_list, translation_node * trans_node_out);
			template<GoICP::DistanceComputationMode DCM>
			float distance_to_target(const GoICP::Pnt & p);
			template<GoICP::DistanceComputationMode DCM>
			float icp(mat3 & R_icp, vec3 & t_icp);
			// build the distance transform for the DCM_DISTANCE_TRANSFORM mode
			void buildDistanceTransform();
			// build the aproximate nearest neighbor tree for the DCM_ANN_TREE mode
			void buildKDTree();
		private:
			const point_cloud *source_cloud;
			const point_cloud *target_cloud;
			point_cloud temp_source_cloud;
			int sample_size; // < source cloud size

			Mat rotation;
			Dir translation;
			std::vector<float> norm_data;
			std::shared_ptr<DT3D> distance_transform;
			ICP icp_obj;
			std::shared_ptr<ann_tree> neighbor_tree; // alternative to distance transform

			float** max_rot_dis; //rotation uncertainity radius
			std::vector<float> min_dis;
			int inlier_num;

			rotation_node init_rot_node, optimal_rot_node;
			translation_node init_trans_node, optimal_trans_node;

		public:
			mat3 optimal_rotation;
			vec3 optimal_translation;
			float optimal_error;
			//settings
			float mse_threshhold, sse_threshhold;
			float trim_fraction;
			float distance_transform_size;
			float distance_transform_expand_factor;
			int max_icp_iterations;
			bool do_trim;
		};

		template<GoICP::DistanceComputationMode DCM>
		inline float GoICP::distance_to_target(const GoICP::Pnt & p)
		{
			switch (DCM) {
			case DCM_DISTANCE_TRANSFORM:
				return distance_transform->distance(p.x(), p.y(), p.z());
			case DCM_ANN_TREE:
				return (target_cloud->pnt(neighbor_tree->find_closest(p)) - p).length();
			}
		}

		template<GoICP::DistanceComputationMode DCM>
		inline float GoICP::innerBnB(float * max_rot_distance_list, translation_node * trans_node_out)
		{
			priority_queue<translation_node> tnodes;

			float opt_trans_err = optimal_error;

			tnodes.push(init_trans_node);

			while (!tnodes.empty())
			{
				translation_node trans_node_parent = tnodes.top(); tnodes.pop();
				translation_node trans_node;

				if (opt_trans_err - trans_node_parent.lb < sse_threshhold)
				{
					break;
				}

				trans_node.w = trans_node_parent.w / 2;
				float max_trans_dis = sqrt(3) / 2.0*trans_node.w;

				for (int j = 0; j < 8; j++)
				{
					trans_node.x = trans_node_parent.x + (j & 1)*trans_node.w;
					trans_node.y = trans_node_parent.y + (j >> 1 & 1)*trans_node.w;
					trans_node.z = trans_node_parent.z + (j >> 2 & 1)*trans_node.w;

					vec3 trans = vec3(trans_node.x, trans_node.y, trans_node.z) + vec3(trans_node.w / 2);

					for (int i = 0; i < sample_size; ++i)
					{
						min_dis[i] = distance_to_target<DCM>(temp_source_cloud.pnt(i) + trans);

						if (max_rot_distance_list)
							min_dis[i] -= max_rot_distance_list[i];

						if (min_dis[i] < 0)
						{
							min_dis[i] = 0;
						}
					}

					if (do_trim)
					{
						sort(min_dis.begin(), min_dis.end());
					}

					float lower_bound = 0;
					float upper_bound = 0;
					for (int i = 0; i < inlier_num; ++i)
					{
						upper_bound += min_dis[i] * min_dis[i];
						float dis = min_dis[i] - max_trans_dis;
						if (dis > 0)
							lower_bound += dis * dis;
					}

					if (upper_bound < opt_trans_err)
					{
						opt_trans_err = upper_bound;
						if (trans_node_out)
							*trans_node_out = trans_node;
					}

					if (lower_bound >= opt_trans_err)
					{
						continue;
					}

					trans_node.ub = upper_bound;
					trans_node.lb = lower_bound;
					tnodes.push(trans_node);
				}
			}

			return opt_trans_err;
		}

		template<GoICP::DistanceComputationMode DCM>
		inline void GoICP::outerBnB()
		{
			static const double PI = 3.141592653589793238462643383279502884L;
			float lower_bound, upper_bound, error, dis;
			rotation_node rot_node;
			translation_node trans_node;
			priority_queue<rotation_node> rotation_queue;
			optimal_error = 0;

			for (int i = 0; i < source_cloud->get_nr_points(); i++)
			{
				min_dis[i] = distance_to_target<DCM>(source_cloud->pnt(i));
			}
			if (do_trim)
			{
				sort(min_dis.begin(), min_dis.end());
			}
			for (int i = 0; i < inlier_num; i++)
			{
				optimal_error += min_dis[i] * min_dis[i];
			}

			mat3 rot_icp = optimal_rotation;
			vec3 trans_icp = optimal_translation;

			// Run ICP
			error = icp<DCM>(rot_icp, trans_icp);
			if (error < optimal_error)
			{
				optimal_error = error;
				optimal_rotation = rot_icp;
				optimal_translation = trans_icp;
			}

			rotation_queue.push(init_rot_node);
			//explore rotation space until convergence is achieved

			while (!rotation_queue.empty()) {
				rotation_node rot_node_parent = rotation_queue.top();
				rotation_queue.pop();

				if ((optimal_error - rot_node_parent.lb) <= sse_threshhold) break;

				rot_node.w = rot_node_parent.w / 2;
				rot_node.l = rot_node_parent.l + 1;

				for (int j = 0; j < 8; ++j)
				{
					mat3 R;

					// calculate new first corner of the sub cube
					rot_node.a = rot_node_parent.a + (j & 1)*rot_node.w;
					rot_node.b = rot_node_parent.b + (j >> 1 & 1)*rot_node.w;
					rot_node.c = rot_node_parent.c + (j >> 2 & 1)*rot_node.w;

					// max point of the sub cube
					vec3 v = vec3(rot_node.a, rot_node.b, rot_node.c) + vec3(rot_node.w / 2);

					float t = v.length();
					// ignore subcubes outside the pi ball (rotations > 180 deg)
					if (t - sqrt(3) * rot_node.w / 2 > PI) continue;

					// build rotation matrix from axis angle representation
					if (t > 0)
					{
						v /= t;

						float c = cos(t);
						float C = 1 - c;
						float s = sin(t);

						float xyC = v.x() * v.y()*C; float zs = v.z() * s;
						float xzC = v.x() * v.z()*C; float ys = v.y() * s;
						float yzC = v.y() * v.z()*C; float xs = v.x() * s;

						R(0, 0) = c + v.x()*v.x()*C;	R(0, 1) = xyC - zs;			R(0, 2) = xzC + ys;
						R(1, 0) = xyC + zs;			R(1, 1) = c + v.y()*v.y()*C;	R(1, 2) = yzC - xs;
						R(2, 0) = xzC - ys;			R(2, 1) = yzC + xs;			R(2, 2) = c + v.z() * v.z()*C;

						temp_source_cloud.resize(sample_size);

						for (int i = 0; i < sample_size; i++)
						{
							const vec3& p = source_cloud->pnt(i);
							temp_source_cloud.pnt(i) = R * p;
						}
					}
					else
					{
						temp_source_cloud.clear();
						temp_source_cloud.append(*source_cloud);
					}

					upper_bound = innerBnB<DCM>(nullptr, &trans_node);

					if (upper_bound < optimal_error)
					{
						optimal_error = upper_bound;
						optimal_rot_node = rot_node;
						optimal_trans_node = trans_node;

						optimal_rotation = R;
						optimal_translation.x() = optimal_trans_node.x + optimal_trans_node.w / 2;
						optimal_translation.y() = optimal_trans_node.y + optimal_trans_node.w / 2;
						optimal_translation.z() = optimal_trans_node.z + optimal_trans_node.w / 2;

						// Run ICP
						mat3 R_icp = optimal_rotation;
						vec3 t_icp = optimal_translation;
						error = icp<DCM>(R_icp, t_icp);

						if (error < optimal_error)
						{
							optimal_error = error;
							optimal_rotation = R_icp;
							optimal_translation = t_icp;
						}

						priority_queue<rotation_node> new_rotation_queue;
						while (!rotation_queue.empty())
						{
							rotation_node node = rotation_queue.top();
							rotation_queue.pop();
							if (node.lb < optimal_error)
								new_rotation_queue.push(node);
							else
								break;
						}
						rotation_queue = new_rotation_queue;
					}

					lower_bound = innerBnB<DCM>(max_rot_dis[rot_node.l], nullptr);

					if (lower_bound >= optimal_error)
					{
						continue;
					}

					rot_node.ub = upper_bound;
					rot_node.lb = lower_bound;
					rotation_queue.push(rot_node);
				}
			}

		}

		template<GoICP::DistanceComputationMode DCM>
		inline float GoICP::icp(mat3 & R_icp, vec3 & t_icp)
		{
			icp_obj.reg_icp(R_icp, t_icp);

			// Transform the source point cloud and use the distance transform to determine the error
			float error = 0;
			for (int i = 0; i < source_cloud->get_nr_points(); i++)
			{
				const vec3& p = source_cloud->pnt(i);
				vec3 t = R_icp * p + t_icp;

				if (!do_trim)
				{
					float dis = distance_to_target<DCM>(t);
					error += dis * dis;
				}
				else
				{
					min_dis[i] = distance_to_target<DCM>(t);
				}
			}
			// do outlier elimination
			if (do_trim)
			{
				sort(min_dis.begin(), min_dis.end());

				for (int i = 0; i < inlier_num; i++)
				{
					error += min_dis[i] * min_dis[i];
				}
			}
			return error;
		}
	}
}
#include <cgv/config/lib_end.h>