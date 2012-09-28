#include <cgv/math/vec.h>
#include <cgv/math/polynomial.h>



void test_polynomial()
{
	using namespace cgv::math;

	//evaluate polynomial using horner scheme 3.0*x^2 + 2*x + 1 at x=5.0
	vec<double> v69(3.0,2.0,1.0);
	assert(poly_val(v69,5.0) == 86.0);

	//evaluate polynomial at x=5.0,x=7.0 and x = 9.0
	vec<double> v70(5.0, 7.0, 9.0);
	vec<double> v71 =poly_val(v69,v70);
	assert(v71.size() == 3 && v71(0) == 86.0 &&v71(1) == 162.0 && v71(2) == 262); 

	//convolution/polynomial multiplication
	vec<double> v53(1.0,2.0,3.0,4.0);
	vec<double> v54(1.0,2.0,1.0,3.0);
	vec<double> v55 = poly_mult(v53,v54);
	assert(v55.size() == 7);
	assert(v55(0) == 1  && v55(1) ==  4 && v55(2) == 8 
		&& v55(3) == 15 && v55(4) == 17 && v55(5) == 13
		&& v55(6) == 12);

	vec<double> p1(1.0,2.0,0.0,4.0);
	vec<double> p2 = poly_der(p1);
	assert(p2.size() == 3);
	assert(p2(0) == 3.0 && p2(1)==4.0 && p2(2)== 0.0);

	vec<double> p3 =poly_add(p1,p2);
	assert(p3.size() == 4);
	assert(p3(0) == 1.0 && p3(1)==5.0 && p3(2)== 4.0&& p3(3)== 4.0);
	
	vec<double> p4 =poly_add(p2,p1);
	assert(p4.size() == 4);
	assert(p4(0) == 1.0 && p4(1)==5.0 && p4(2)== 4.0&& p4(3)== 4.0);

	//std::cout << vander(cgv::math::lin_space<double>(1.0,3.0,5));
	vec<double> p5(3.0,-2.0,4.0,-3.0);
	vec<double> p6(1.0,3.0,3.0);
	vec<double> q,r;
	poly_div(p5,p6,q,r);
	
	assert( q(0) == 3 && q(1) == -11);
	//std::cout<< q <<" should be: 3,  -11"<<std::endl;
	assert(r(0) == 28 && r(1) == 30);
	//std::cout << r<<" should be: 28, +30"<<std::endl;

	vec<double> p7(3);
	p7.set(1,1,2);
	std::cout << poly_int<double>(p7,4)<<std::endl;
	

	//output
	//std::cout << v70 <<std::endl;



}









