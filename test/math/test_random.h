#pragma once
#include <cgv/math/random.h>

void test_random()
{
	using namespace cgv::math;

	random rangen;
	
	int i;
	rangen.uniform(i);
	std::cout <<"random int:"<<i<<std::endl;
	
	unsigned int ui;
	rangen.uniform(ui);
	std::cout <<"random unsigned int:"<<ui<<std::endl;

	long li;
	rangen.uniform(li);
	std::cout <<"random long:"<<li<<std::endl;

	long long lli;
	rangen.uniform(lli);
	std::cout <<"random long long:"<<lli<<std::endl;

	float f;
	rangen.uniform(f);
	std::cout <<"random float:"<<f<<std::endl;

	double d;
	rangen.uniform(d);
	std::cout <<"random double:"<<d<<std::endl;
}