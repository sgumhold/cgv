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
			distance_transform_size = 200;
			distance_transform_expand_factor = 2.0;
			max_icp_iterations = 50;

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

			dc_mode = DCM_NONE;

		}

		GoICP::~GoICP()
		{
			clear();
		}

		void GoICP::buildDistanceTransform()
		{
			//build distance transform
			distance_transform = make_shared<DT3D>();
			vector<double> x, y, z;
			for (int i = 0; i < target_cloud->get_nr_points(); i++)
			{
				x.emplace_back(target_cloud->pnt(i).x());
				y.emplace_back(target_cloud->pnt(i).y());
				z.emplace_back(target_cloud->pnt(i).z());
			}
			distance_transform->size = distance_transform_size;
			distance_transform->expandFactor = distance_transform_expand_factor;
			distance_transform->build(x.data(), y.data(), z.data(), target_cloud->get_nr_points());
		}

		void GoICP::buildKDTree()
		{
			neighbor_tree = make_shared<ann_tree>();
			neighbor_tree->build(*target_cloud);
			
		}


		float GoICP::registerPointcloud()
		{
			switch (dc_mode) {
			case DCM_DISTANCE_TRANSFORM:
				outerBnB<DCM_DISTANCE_TRANSFORM>();
				break;
			case DCM_ANN_TREE:
				outerBnB<DCM_ANN_TREE>();
				break;
			}

			return optimal_error;
		}

		void GoICP::initializeRegistration(const point_cloud &inputCloud)
		{
			source_cloud = &inputCloud;
			sample_size = source_cloud->get_nr_points();

			float sigma, max_angle;
			assert(sample_size >= 0 && sample_size <= source_cloud->get_nr_points());

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

			// set parameters of ICP
			icp_obj.set_source_cloud(*source_cloud);
			icp_obj.eps = mse_threshhold / 1000.0;
			icp_obj.maxIterations = max_icp_iterations;
			icp_obj.set_num_random(0);
			
			// initial rotation and translation
			optimal_rot_node = init_rot_node;
			optimal_trans_node = init_trans_node;
			optimal_rotation.identity();
			optimal_translation.zeros();

			// triming
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

		void GoICP::initializeDistanceComputation(const point_cloud &inputCloud)
		{
			target_cloud = &inputCloud;
			switch (dc_mode) {
			case DCM_DISTANCE_TRANSFORM:
				buildDistanceTransform();
				icp_obj.set_target_cloud(*target_cloud);
				// ICP only uses the more precise ann tree based distance computation
				icp_obj.build_ann_tree();
				break;
			case DCM_ANN_TREE:
				buildKDTree();
				// uses own ann tree also for ICP
				icp_obj.set_target_cloud(*target_cloud, neighbor_tree);
				break;
			default:
				cerr << "GoICP::initializeDistanceComputation : invalid computation mode set!\n";
				assert(false);
			}
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