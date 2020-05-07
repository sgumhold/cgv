#include "GoICP.h"
#include <queue>
#include <cassert>

using namespace std;
using namespace cgv;
using namespace cgv::pointcloud;

#define PI 3.141592653589793238462643383279502884L


namespace cgv {
	namespace pointcloud {

		GoICP::GoICP()
		{
			source_cloud = nullptr;
			target_cloud = nullptr;

			max_rot_dis = nullptr;

			//settings
			mse_threshhold = 0.0001;
			trim_fraction = 0.0;
			distance_transform_size = 300;
			distance_transform_expand_factor = 2.0;

			init_rot_node.a = -PI;
			init_rot_node.b = -PI;
			init_rot_node.c = -PI;
			init_rot_node.w = 2 * PI;
			init_rot_node.l = 0;
			init_rot_node.lb = 0;
			init_trans_node.x = -1.0;
			init_trans_node.y = -1.0;
			init_trans_node.z = -1.0;
			init_trans_node.w = 2.0;
			init_trans_node.lb = 0;
			do_trim = false;
			norm_data = vector<float>();

		}

		GoICP::~GoICP()
		{
			clear();
		}

		void GoICP::buildDistanceTransform()
		{
			vector<double> x, y, z;
			for (int i = 0; i < target_cloud->get_nr_points(); i++)
			{
				x.emplace_back(target_cloud->pnt(i).x());
				y.emplace_back(target_cloud->pnt(i).y());
				z.emplace_back(target_cloud->pnt(i).z());
			}
			distance_transform.size = distance_transform_size;
			distance_transform.expandFactor = distance_transform_expand_factor;
			distance_transform.build(x.data(), y.data(), z.data(), target_cloud->get_nr_points());
		}

		float GoICP::icp(mat3 & R_icp, vec3 & t_icp)
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
					float dis = distance_transform.distance(t.x(), t.y(), t.z());
					error += dis * dis;
				}
				else
				{
					min_dis[i] = distance_transform.distance(t.x(), t.y(), t.z());
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

		void GoICP::initialize()
		{
			float sigma, max_angle;
			assert(sample_size >= 0 && sample_size <= source_cloud->get_nr_points());

			// build distance transform
			buildDistanceTransform();

			// calculate norm of each point in the source cloud to coordinate system origin
			norm_data.resize(sample_size);

			for (int i = 0; i < sample_size; ++i)
			{
				norm_data[i] = source_cloud->pnt(i).length();
			}

			max_rot_dis = new float*[max_rot_level];
			for (int i = 0; i < max_rot_level; i++)
			{
				max_rot_dis[i] = new float[sample_size];

				sigma = init_rot_node.w / pow(2.0, i) / 2.0; // Half-side length of each rotation subcube
				max_angle = sqrt(3.f) * sigma;

				if (max_angle > PI)
					max_angle = PI;
				for (int j = 0; j < sample_size; j++)
					max_rot_dis[i][j] = 2 * sin(max_angle / 2)*norm_data[j];
			}

			min_dis = vector<float>(sample_size);

			//initialize ICP
			icp_obj.set_target_cloud(*target_cloud);
			icp_obj.set_source_cloud(*source_cloud);
			icp_obj.eps = mse_threshhold / 1000;
			//icp_obj.maxIterations = 10000;
			icp_obj.maxIterations = 100;
			icp_obj.initialize();
			
			//initial rotation and translation
			optimal_rot_node = init_rot_node;
			optimal_trans_node = init_trans_node;
			optimal_rotation.identity();
			optimal_translation.zeros();

			//triming
			if (do_trim)
			{
				inlier_num = (int)(sample_size * (1 - trim_fraction));
			}
			else
			{
				inlier_num = sample_size;
			}
			sse_threshhold = mse_threshhold * inlier_num;
		}

		void cgv::pointcloud::GoICP::outerBnB()
		{
			float lower_bound, upper_bound, error, dis;
			rotation_node rot_node;
			translation_node trans_node;
			priority_queue<rotation_node> rotation_queue;
			optimal_error = 0;

			for (int i = 0; i < source_cloud->get_nr_points(); i++)
			{
				const vec3 p = source_cloud->pnt(i);
				min_dis[i] = distance_transform.distance(p.x(), p.y(), p.z());
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
			error = icp(rot_icp, trans_icp);
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
					vec3 v = vec3(rot_node.a, rot_node.b, rot_node.c)+ vec3(rot_node.w / 2);

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

						R(0,0) = c+v.x()*v.x()*C;	R(0,1) = xyC - zs;			R(0,2) = xzC + ys;
						R(1,0) = xyC + zs;			R(1,1) = c + v.y()*v.y()*C;	R(1,2) = yzC - xs;
						R(2,0) = xzC - ys;			R(2,1) = yzC + xs;			R(2,2) = c + v.z() * v.z()*C;

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

					upper_bound = innerBnB(nullptr , &trans_node);

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
						error = icp(R_icp, t_icp);

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

					lower_bound = innerBnB(max_rot_dis[rot_node.l], nullptr);

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

		float cgv::pointcloud::GoICP::innerBnB(float * max_rot_distance_list, translation_node * trans_node_out)
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

					vec3 trans = vec3(trans_node.x, trans_node.y, trans_node.z) + vec3(trans_node.w/2);

					for (int i = 0; i < sample_size; ++i)
					{
						min_dis[i] = distance_transform.distance(
								temp_source_cloud.pnt(i).x() + trans.x(),
								temp_source_cloud.pnt(i).y() + trans.y(),
								temp_source_cloud.pnt(i).z() + trans.z());

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

		void cgv::pointcloud::GoICP::clear()
		{
			if (max_rot_dis != nullptr){
				for (int i = 0; i < max_rot_level; ++i)
				{
					delete(max_rot_dis[i]);
				}
				delete(max_rot_dis);
				max_rot_dis = nullptr;
			}
		}


	}

}