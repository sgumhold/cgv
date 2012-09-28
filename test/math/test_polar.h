#pragma once
#include <cgv/math/polar.h>
#include <cgv/math/transformations.h>

void test_polar()
{
	using namespace cgv::math;
	mat<double> m(3,3);
	m(0,0) = 1.0; m(0,1) = 2.0;	m(0,2) = 3.0;
	m(1,0) = 2.0; m(1,1) = 5.0;	m(1,2) = 6.0;
	m(2,0) = 3.0; m(2,1) = 6.0;	m(2,2) = 2.0;

	mat<double> r,a;
	polar(m, r, a);
	assert(frobenius_norm(r*a-m) < 0.0001);

	vec<double> dir;
	double angle;

	vec<double> d(1.0,1.0,1.0);
	d.normalize();
	r=cgv::math::rotate_33<double>(1.0/sqrt(3.0),1.0/sqrt(3.0),1.0/sqrt(3.0),30.0);
	
	
	decompose_rotation(r,dir,angle);
	
	assert((r-cgv::math::rotate_33<double>(dir(0),dir(1),dir(2),angle)).frobenius_norm() < 0.0001);


}