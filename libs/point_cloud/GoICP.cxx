#include "GoICP.h"
#include <vector>

using namespace std;
using namespace cgv;
using namespace cgv::pointcloud;

#define PI 3.141592653589793238462643383279502884L;

GoICP::GoICP() 
{
	source_cloud = nullptr;
	target_cloud = nullptr;

	//settings
	mse_threshhold = 0.001;

	init_rot.a = -PI;
	init_rot.b = -PI;
	init_rot.c = -PI;
	init_rot.w = 2 * PI;
	init_rot.l = 0;
	init_rot.lb = 0;
	init_trans.lb = 0;
}

void GoICP::build_distance_transform()
{
	vector<double> x,y,z;
	for (int i = 0; i < target_cloud->get_nr_points(); i++)
	{
		x.emplace_back(target_cloud->pnt(i).x());
		y.emplace_back(target_cloud->pnt(i).y());
		z.emplace_back(target_cloud->pnt(i).z());
	}
	distance_transform.build(x.data(), y.data(), z.data(), target_cloud->get_nr_points());
}

float GoICP::icp(mat & R_icp, mat & t_icp)
{
	return 0.0f;
}
