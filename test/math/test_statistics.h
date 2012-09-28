#pragma once
#include <cgv/math/random.h>
#include <cgv/math/statistics.h>



void test_statistics()
{
	using namespace cgv::math;

	random r;
	vec<double> data1(10000);
	vec<double> data2(10000);
	for(unsigned i = 0; i < 10000;i++)
	{
		r.normal(data1(i));
		r.uniform(data2(i));
		if(i % 10 ==0 && i > 0)
		{
			std::cout <<"normal  "<< norm_ks_test(0.0,1.0,data1.sub_vec(0,i))<<std::endl;
			std::cout <<"uniform "<< norm_ks_test(0.0,1.0,data2.sub_vec(0,i))<<std::endl<<std::endl;
		}
	}


//	std::cout << norm_ks_test(0.0,1.0,data1)<<std::endl;
//	std::cout <<"should be smaller"<< norm_ks_test(0.0,1.0,data2)<<std::endl;

		
	

}