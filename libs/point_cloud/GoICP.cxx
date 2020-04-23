#include "GoICP.h"

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

			//settings
			mse_threshhold = 0.001;
			trim_fraction = 0.0;
			distance_transform_size = 300;
			distance_transform_expand_factor = 2.0;

			init_rot_node.a = -PI;
			init_rot_node.b = -PI;
			init_rot_node.c = -PI;
			init_rot_node.w = 2 * PI;
			init_trans_node.x = -0.5;
			init_trans_node.y = -0.5;
			init_trans_node.z = -0.5;
			init_trans_node.w = 1.0;
			do_trim = true;
			norm_data = vector<float>();

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
			distance_transform.SIZE = distance_transform_size;
			distance_transform.expandFactor = distance_transform_expand_factor;
			distance_transform.build(x.data(), y.data(), z.data(), target_cloud->get_nr_points());
		}

		float GoICP::icp(mat3 & R_icp, vec3 & t_icp)
		{
			
			icp_obj.reg_icp(R_icp, t_icp);
			
			int i;
			float error, dis;

			temp_icp_cloud.resize(source_cloud->get_nr_points());

			// Transform point cloud and use DT to determine the L2 error
			error = 0;
			for (i = 0; i < source_cloud->get_nr_points(); i++)
			{
				const vec3& p = source_cloud->pnt(i);
				vec3& t = temp_icp_cloud.pnt(i);
				t = R_icp * p + t_icp;

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
			//outlier elimination
			if (do_trim)
			{
				sort(min_dis.begin(), min_dis.end());
				
				for (i = 0; i < icp_inlier_num; i++)
				{
					error += min_dis[i] * min_dis[i];
				}
			}
			return error;
		}

		void GoICP::initialize()
		{
			int i, j;
			float sigma, maxAngle;
			int Nd = source_cloud->get_nr_points();
			int Nm = target_cloud->get_nr_points();

			// Precompute the rotation uncertainty distance (maxRotDis) for each point in the data and each level of rotation subcube

			// Calculate L2 norm of each point in data cloud to origin
			norm_data.resize(Nd);

			for (i = 0; i < Nd; i++)
			{
				norm_data[i] = source_cloud->pnt(i).length();
			}

			max_rot_dis = new float*[max_rot_level];
			for (i = 0; i < max_rot_level; i++)
			{
				max_rot_dis[i] = (float*)malloc(sizeof(float*)*Nd);

				sigma = init_rot_node.w / pow(2.0, i) / 2.0; // Half-side length of each level of rotation subcube
				maxAngle = sqrt(3.f) * sigma;

				if (maxAngle > PI)
					maxAngle = PI;
				for (j = 0; j < Nd; j++)
					max_rot_dis[i][j] = 2 * sin(maxAngle / 2)*norm_data[j];
			}

			// Temporary Variables
			min_dis = vector<float>(Nd);

			// TODO Build ICP kdtree with model dataset
			//icp_obj.build();

			//init rotation and translation
			optimal_rot_node = init_rot_node;
			optimal_trans_node = init_trans_node;
			// Initialise so-far-best rotation and translation matrices
			optimal_rotation.identity();
			optimal_translation.zeros();

			//triming
			if (do_trim)
			{
				// Calculate number of inlier points
				icp_inlier_num = (int)(Nd * (1 - trim_fraction));
			}
			else
			{
				icp_inlier_num = Nd;
			}
			sse_threshhold = mse_threshhold * icp_inlier_num;
		}

		void cgv::pointcloud::GoICP::outerBnB()
		{
		}

		float cgv::pointcloud::GoICP::innerBnB(float * maxRotDisL, translation_node * nodeTransOut)
		{
			return 0.0f;
		}

		void cgv::pointcloud::GoICP::clear()
		{
		}


	}

}