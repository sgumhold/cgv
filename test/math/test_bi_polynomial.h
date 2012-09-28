#include <cgv/math/random.h>
#include <cgv/math/bi_polynomial.h>


void test_bi_polynomial()
{
	using namespace cgv::math;

	mat<double> m(2,2);
	m(0,0) =1; m(0,1) =2; 
	m(1,0) =3; m(1,1) =4; 
	std::cout << bipoly_val(m,1.0,2.0)<<std::endl;

	random rangen;
	vec<double> X(10);
	vec<double> Y(10);
	vec<double> Z(10);
	rangen.uniform(-0.5,0.5,X);
	rangen.uniform(-0.5,0.5,Y);
	for(unsigned i = 0; i < 10;i++)
	{
		Z(i) = sin(X(i));
	}

	
	//std::cout << poly_fit<double>(3,Y,Z);

	mat<double> bp(2,3);
	bp(0,0)=1;bp(0,1)=1;bp(0,2)=1;
	bp(1,0)=1;bp(1,1)=1;bp(1,2)=1;
	

	std::cout << bp<<std::endl<<std::endl;
	std::cout << bipoly_der_x2(bipoly_int_x2<double>(bp,4));


	//mat<double> bp = bipoly_fit<double>(3,1,X,Y,Z);
	//for(unsigned i = 0; i < 10;i++)
	//std::cout << bipoly_val(bp,X(i),Y(i)) <<":" <<Z(i)<<std::endl;


	
	//std::cout <<"\n"<< 1*1.0*2.0   + 2*2.0   + 3*1.0     + 4 <<std::endl;
	
	

}