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
			original_source_cloud = nullptr;
			original_target_cloud = nullptr;
			target_cloud = point_cloud();
			source_cloud = point_cloud();

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
			init_trans_node.x = -0.5;
			init_trans_node.y = -0.5;
			init_trans_node.z = -0.5;
			init_trans_node.w = 1.0;
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
			for (int i = 0; i < target_cloud.get_nr_points(); i++)
			{
				x.emplace_back(target_cloud.pnt(i).x());
				y.emplace_back(target_cloud.pnt(i).y());
				z.emplace_back(target_cloud.pnt(i).z());
			}
			distance_transform.size = distance_transform_size;
			distance_transform.expandFactor = distance_transform_expand_factor;
			distance_transform.build(x.data(), y.data(), z.data(), target_cloud.get_nr_points());
		}

		float GoICP::icp(mat3 & R_icp, vec3 & t_icp)
		{
			icp_obj.reg_icp(R_icp, t_icp);
			
			int i;
			float error, dis;
			
			// Transform the source point cloud and use the distance transform to determine the error
			error = 0;
			for (i = 0; i < source_cloud.get_nr_points(); i++)
			{
				const vec3& p = source_cloud.pnt(i);
				 vec3 t = R_icp * p + t_icp;

				if (!do_trim)
				{
					dis = distance_transform.distance(t.x(), t.y(), t.z());
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
				
				for (i = 0; i < inlier_num; i++)
				{
					error += min_dis[i] * min_dis[i];
				}
			}
			return error;
		}

		void GoICP::initialize()
		{
			float sigma, max_angle;
			assert(sample_size >= 0 && sample_size <= original_source_cloud->get_nr_points());

			// scale input pointclouds to fit in a box with side length = 1.0 and move center to origin

			cgv::media::axis_aligned_box<float, 3> compound_aabb = original_source_cloud->box();
			auto target_aabb = original_target_cloud->box();
			compound_aabb.add_point(target_aabb.get_max_pnt());
			compound_aabb.add_point(target_aabb.get_min_pnt());
			
			//cloud_scale = compound_aabb.get_extent()[compound_aabb.get_max_extent_coord_index()];
			cloud_scale = 1.0;

			target_cloud.resize(original_target_cloud->get_nr_points());
			source_cloud.resize(original_source_cloud->get_nr_points());

			//cloud_offset = compound_aabb.get_center();
			cloud_offset = vec3(0);
			
			for (int i = 0; i < source_cloud.get_nr_points(); ++i) {
				source_cloud.pnt(i) = (original_source_cloud->pnt(i) - cloud_offset) / cloud_scale;
			}
			for (int i = 0; i < target_cloud.get_nr_points(); ++i) {
				target_cloud.pnt(i) = (original_target_cloud->pnt(i) - cloud_offset) / cloud_scale;
			}

			// build distance transform
			buildDistanceTransform();

			// calculate norm of each point in the source cloud to coordinate system origin
			norm_data.resize(sample_size);

			for (int i = 0; i < sample_size; ++i)
			{
				norm_data[i] = source_cloud.pnt(i).length();
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

			// Temporary variables
			min_dis = vector<float>(sample_size);

			//initialize ICP
			icp_obj.set_target_cloud(target_cloud);
			icp_obj.set_source_cloud(source_cloud);
			icp_obj.eps = mse_threshhold / 10000;
			//icp_obj.maxIterations = 10000;
			icp_obj.maxIterations = 50;
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
			
		}

		float cgv::pointcloud::GoICP::innerBnB(float * maxRotDisL, translation_node * nodeTransOut)
		{
			return 0;
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