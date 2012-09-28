#pragma once
#include <cgv/math/distance_transform.h>

void test_distance_transform()
{
	cgv::math::mat<double> img_in;
	img_in.zeros(10,12);
	for(unsigned x = 0;x < 5;x++)
	{
		img_in(x,3+x)=1.0;
		img_in(x,8-x)=1.0;

	}

	std::cout <<img_in<<std::endl;

	cgv::math::mat<double> img_out;
	cgv::math::dist_transf_2d<double>(img_in,img_out,1.0);
	std::cout <<img_out<<std::endl;
	


}