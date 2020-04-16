#include "GoICP.h"

GoICP::GoICP() 
{
	sourceCloud = nullptr;
	targetCloud = nullptr;

	//settings
	mse_threshhold = 0.001;
}

void GoICP::build_distance_transform()
{

}

float GoICP::icp(mat & R_icp, mat & t_icp)
{
	return 0.0f;
}
