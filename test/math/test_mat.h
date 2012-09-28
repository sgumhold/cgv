#pragma once
#include <cgv/math/mat.h>


void test_mat()
{
	using namespace cgv::math;

	//creates an empty matrix of zero size
	mat<double> m1;
	assert(m1.size() == 0 && m1.nrows() == 0 && m1.ncols() == 0);

	//creates an 3x4 matrix
	mat<double> m2(3,4);
	assert(m2.size() == 12 && m2.nrows() == 3 && m2.ncols()==4);

	//creates an 4x3 matrix with initial values == 1
	mat<double> m3(3,2,1.0);
	assert(m3.size() == 6 && m3.nrows() == 3 && m3.ncols() == 2);
	assert(m3(0,0) == 1.0 && m3(0,1) == 1.0 && 
		   m3(1,0) == 1.0 && m3(1,1) == 1.0 &&
		   m3(2,0) == 1.0 && m3(2,1) == 1.0 );

	//creates a matrix from an array
	double arr[6]={ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 }; 
	//column major
	mat<double> m4(2,3, arr,true);
	assert(m4.size() == 6 && m4.nrows() == 2 && m4.ncols() == 3);
	assert(m4(0,0) == 1.0 && m4(0,1) == 3.0 && m4(0,2) == 5.0 && 
		   m4(1,0) == 2.0 && m4(1,1) == 4.0 && m4(1,2) == 6.0 );

	//row major
	mat<double> m5(2,3, arr,false);
	assert(m5.size() == 6 && m5.nrows() == 2 && m5.ncols() == 3);
	assert(m5(0,0) == 1.0 && m5(0,1) == 2.0 && m5(0,2) == 3.0 && 
		   m5(1,0) == 4.0 && m5(1,1) == 5.0 && m5(1,2) == 6.0 );

	//copy constructor
	mat<double> m6 = m5;
	assert(m6.size() == m5.size() && m6.nrows() == m5.nrows() && m6.ncols() == m5.ncols());
	assert(m6(0,0) == 1.0 && m6(0,1) == 2.0 && m6(0,2) == 3.0 && 
		   m6(1,0) == 4.0 && m6(1,1) == 5.0 && m6(1,2) == 6.0 );

	mat<float> m7 = m5;
	assert(m7.size() == m5.size() && m7.nrows() == m5.nrows() && m7.ncols() == m5.ncols());
	assert(m7(0,0) == 1.0f && m7(0,1) == 2.0f && m7(0,2) == 3.0f && 
		   m7(1,0) == 4.0f && m7(1,1) == 5.0f && m7(1,2) == 6.0f );

	//assignment
	mat<double>  m8(2,3);
	m8(0,0) = 1.0; m8(0,1) = 2.0; m8(0,2) = 3.0;  
	m8(1,0) = 4.0; m8(1,1) = 5.0; m8(1,2) = 6.0;
	mat<double> m9(2,2);
	m9 = m8;
	assert(m9.size() == m8.size() && m9.nrows() == m8.nrows() && m9.ncols() == m8.ncols());
	assert(m9(0,0) == 1.0 && m9(0,1) == 2.0 && m9(0,2) == 3.0 && 
		   m9(1,0) == 4.0 && m9(1,1) == 5.0 && m9(1,2) == 6.0 );
	mat<float> m10;
	m10 = m8;
	assert(m10.size() == m8.size() && m10.nrows() == m8.nrows() && m10.ncols() == m8.ncols());
	assert(m10(0,0) == 1.0f && m10(0,1) == 2.0f && m10(0,2) == 3.0f && 
		   m10(1,0) == 4.0f && m10(1,1) == 5.0f && m10(1,2) == 6.0f );

	//cast into array
	double* ptr = (double*)m9;
	const double* ptrc = (const double*)m9;

	//resize matrix
	mat<double> m11;
	m11.resize(100,50);
	assert(m11.size() == 5000 && m11.nrows() == 100 && m11.ncols() == 50);
	m11.resize(1,1);
	assert(m11.size() == 1 && m11.nrows() == 1 && m11.ncols() == 1);

	//filling /assigning a scalar
	mat<double> m12(2,3);
	m12.fill(3.0);
	assert(m12(0,0) == 3.0f && m12(0,1) == 3.0f && m12(0,2) == 3.0f && 
		   m12(1,0) == 3.0f && m12(1,1) == 3.0f && m12(1,2) == 3.0f );
	m12 =2.0;
	assert(m12(0,0) == 2.0f && m12(0,1) == 2.0f && m12(0,2) == 2.0f && 
		   m12(1,0) == 2.0f && m12(1,1) == 2.0f && m12(1,2) == 2.0f );

	//is square test
	assert(m12.is_square() == false);
	m12.resize(4,4);
	assert(m12.is_square() == true);

	//test equality/inequality
	mat<double> m13(2,3);
	m13.fill(1.0);
	mat<double> m14(2,3);
	m14.fill(2.0);
	mat<double> m15(2,3);
	m15.fill(1.0);
	assert(m13 != m14);
	assert(m13 == m15);

	//multiplication by scalar
	mat<double> m16(2,3);
	m16(0,0) = 2.0;  m16(0,1) = 3.0; m16(0,2) = 4.0;
	m16(1,0) = 2.0;  m16(1,1) = 1.0; m16(1,2) = 2.0;

	mat<double> m17 = m16*2.0;
	mat<double> m18 = 2.0*m16;
	mat<double> m19 = m16;
	m19*=2.0;
	assert(m17(0,0) == 4.0 && m17(0,1) == 6.0 && m17(0,2) == 8.0 && 
		   m17(1,0) == 4.0 && m17(1,1) == 2.0 && m17(1,2) == 4.0 );
	assert(m18 == m17);
	assert(m19 == m17);

	//division by scalar
	mat<double> m20(2,3);
	m20(0,0) = 4.0;  m20(0,1) = 8.0; m20(0,2) = 4.0;
	m20(1,0) = 2.0;  m20(1,1) = 4.0; m20(1,2) = -4.0;
	mat<double> m21 = m20/2.0;
	mat<double> m22 = m20;
	m22/=2.0;
	assert(m21(0,0) == 2.0 && m21(0,1) == 4.0 && m21(0,2) == 2.0 && 
		   m21(1,0) == 1.0 && m21(1,1) == 2.0 && m21(1,2) == -2.0 );
	assert(m22 == m21);

	//addition of scalar
	mat<double> m23(2,3);
	m23(0,0) = 2.0;  m23(0,1) = 3.0; m23(0,2) = 4.0;
	m23(1,0) = 2.0;  m23(1,1) = 1.0; m23(1,2) = 2.0;

	mat<double> m24 = m23+1.0;
	mat<double> m25 = m23;
	m25+=1.0;

	assert(m24(0,0) == 3.0 && m24(0,1) == 4.0 && m24(0,2) == 5.0 && 
		   m24(1,0) == 3.0 && m24(1,1) == 2.0 && m24(1,2) == 3.0 );
	assert(m25 == m24);

	//subtraction of scalar
	mat<double> m26(2,3);
	m26(0,0) = 2.0;  m26(0,1) = 3.0; m26(0,2) = 4.0;
	m26(1,0) = 2.0;  m26(1,1) = 1.0; m26(1,2) = 2.0;

	mat<double> m27 = m26-1.0;
	mat<double> m28 = m26;
	m28-=1.0;
	assert(m27(0,0) == 1.0 && m27(0,1) == 2.0 && m27(0,2) == 3.0 && 
		   m27(1,0) == 1.0 && m27(1,1) == 0.0 && m27(1,2) == 1.0 );
	assert(m27 == m28);

	//multiplication matrix x matrix
	mat<double> m29(3,3);
	m29(0,0) = 1.0;  m29(0,1) = 2.0; m29(0,2) = 3.0;
	m29(1,0) = 4.0;  m29(1,1) = 5.0; m29(1,2) = 6.0;
	m29(2,0) = 7.0;  m29(2,1) = 8.0; m29(2,2) = 9.0;
	m29*=m29;
	assert(m29(0,0) ==  30.0 && m29(0,1) ==  36.0 && m29(0,2) == 42.0 && 
		   m29(1,0) ==  66.0 && m29(1,1) ==  81.0 && m29(1,2) == 96.0 &&
		   m29(2,0) == 102.0 && m29(2,1) == 126.0 && m29(2,2) == 150.0 );
	mat<double> m30(2,3);
	m30(0,0) = 1.0;  m30(0,1) = 2.0; m30(0,2) = 3.0;
	m30(1,0) = 4.0;  m30(1,1) = 5.0; m30(1,2) = 6.0;
	mat<double> m31(3,2);
	m31(0,0) = 1.0;  m31(0,1) = 2.0; 
	m31(1,0) = 3.0;  m31(1,1) = 4.0;
	m31(2,0) = 5.0;  m31(2,1) = 6.0;
	mat<double> m32 = m30 * m31;
	assert(m32.ncols() == 2 && m32. nrows() == 2);
	assert(m32(0,0) ==  22.0 && m32(0,1) ==  28.0 && 
		   m32(1,0) ==  49.0 && m32(1,1) ==  64.0 );

	//matrix times vector
	mat<double> m33(2,3);

	m33(0,0) = 1.0;  m33(0,1) = 2.0; m33(0,2) = 3.0;
	m33(1,0) = 4.0;  m33(1,1) = 5.0; m33(1,2) = 6.0;
	
	vec<double> v1(1.0,2.0,3.0);
	vec<double> v2 = m33*v1;
	assert(v2.size() == 2);
	assert(v2(0) == 14.0 && v2(1) == 32.0);

	//dyadic product
	vec<double> v3(2.0,3.0,4.0);
	mat<double> m34 = dyad(v3,v3);
	assert(m34.nrows() == 3 && m34.ncols() ==3);
	assert(m34(0,0) ==  4.0 && m34(0,1) ==  6.0 && m34(0,2) == 8.0 && 
		   m34(1,0) ==  6.0 && m34(1,1) ==  9.0 && m34(1,2) == 12.0 &&
		   m34(2,0) ==  8.0 && m34(2,1) == 12.0 && m34(2,2) == 16.0 );


	//sub_mat/row/col/set_row/set_col
	mat<double> m35(3,4);
	m35(0,0) = 1; m35(0,1) = 2;  m35(0,2) = 3; m35(0,3) = 4; 
	m35(1,0) = 5; m35(1,1) = 6;  m35(1,2) = 7; m35(1,3) = 8;
	m35(2,0) = 9; m35(2,1) = 10; m35(2,2) = 11; m35(2,3) = 12;
	
	mat<double> m36 = m35.sub_mat(1,2,2,2);
	assert(m36.nrows() == 2 && m36.ncols() == 2);
	assert(m36(0,0) == 7.0 && m36(0,1) == 8 &&
		   m36(1,0) == 11.0 && m36(1,1) == 12);

	vec<double> v4 = m35.row(2);
	assert(v4.size() == 4);
	assert(v4(0) == 9.0 && v4(1) == 10.0 && v4(2) == 11.0 && v4(3) == 12.0 );

	vec<double> v5 = m35.col(2);
	assert(v5.size() == 3);
	assert(v5(0) == 3.0 && v5(1) == 7.0 && v5(2) == 11.0 );

	vec<double> v6(2.0,3.0,2.0,3.0);
	m35.set_row(0,v6);
	assert(m35.row(0) == v6);

	vec<double> v7(2.0,3.0,2.0);
	m35.set_col(2,v7);
	assert(m35.col(2) == v7);

	//copy/paste
	mat<double> m37(3,4);
	m37(0,0) = 1; m37(0,1) = 2;  m37(0,2) = 3; m37(0,3) = 4; 
	m37(1,0) = 5; m37(1,1) = 6;  m37(1,2) = 7; m37(1,3) = 8;
	m37(2,0) = 9; m37(2,1) = 10; m37(2,2) = 11; m37(2,3) = 12;

	mat<double> m38(2,2);
	m37.copy(0,1, 2,2, m38);
	assert(m38(0,0) == 2.0 && m38(0,1) == 3.0&&
		m38(1,0) == 6.0 && m38(1,1) == 7.0);

	m37.paste(1,2,m38);
	assert(m37(1,2) == 2.0 && m37(1,3) == 3.0 &&
		m37(2,2) == 6.0 && m37(2,3) == 7.0);


	//swap rows/columns/main diagonal entries
	mat<double> m39(3,3);
	m39(0,0) = 1; m39(0,1) = 2; m39(0,2) = 3;
	m39(1,0) = 4; m39(1,1) = 5; m39(1,2) = 6;
	m39(2,0) = 7; m39(2,1) = 8; m39(2,2) = 9;

	m39.swap_rows(0, 1);
	assert(m39(0,0) == 4.0 && m39(0,1) ==5.0 && m39(0,2) == 6.0 &&
		   m39(1,0) == 1.0 && m39(1,1) ==2.0 && m39(1,2) == 3.0);

	mat<double> m40(3,3);
	m40(0,0) = 1; m40(0,1) = 2; m40(0,2) = 3;
	m40(1,0) = 4; m40(1,1) = 5; m40(1,2) = 6;
	m40(2,0) = 7; m40(2,1) = 8; m40(2,2) = 9;

	m40.swap_columns(1, 2);
	assert(m40(0,1) == 3.0 && m40(1,1) ==6.0 && m40(2,1) == 9.0 &&
		   m40(0,2) == 2.0 && m40(1,2) ==5.0 && m40(2,2) == 8.0 );


	mat<double> m41(3,3);
	m41(0,0) = 1; m41(0,1) = 2; m41(0,2) = 3;
	m41(1,0) = 4; m41(1,1) = 5; m41(1,2) = 6;
	m41(2,0) = 7; m41(2,1) = 8; m41(2,2) = 9;
	m41.swap_diagonal_elements(0,2);
	assert(m41(0,0) == 9.0 && m41(2,2) == 1.0);


	//compute the trace
	mat<double> m42(3,3);
	m42(0,0) = 1; m42(0,1) = 2; m42(0,2) = 3;
	m42(1,0) = 4; m42(1,1) = 5; m42(1,2) = 6;
	m42(2,0) = 7; m42(2,1) = 8; m42(2,2) = 9;
	assert(m42.trace() == 15.0);

	//compute the transpose of a matrix
	mat<double> m43 = transpose(m42);
	assert(m43(0,0) == 1.0 && m43(0,1) ==4.0 && m43(0,2) == 7.0 &&
		   m43(1,0) == 2.0 && m43(1,1) ==5.0 && m43(1,2) == 8.0 &&
		   m43(2,0) == 3.0 && m43(2,1) ==6.0 && m43(2,2) == 9.0 ); 


	//flip left right 
	mat<double> m44 = m42;
	m44.fliplr();
	assert(m44(0,0) == 3.0 && m44(0,1) ==2.0 && m44(0,2) == 1.0 &&
		   m44(1,0) == 6.0 && m44(1,1) ==5.0 && m44(1,2) == 4.0 &&
		   m44(2,0) == 9.0 && m44(2,1) ==8.0 && m44(2,2) == 7.0 ); 

	m44 =m42;
	//flip up down
	m44.flipud();
	assert(m44(0,0) == 7.0 && m44(0,1) ==8.0 && m44(0,2) == 9.0 &&
		   m44(1,0) == 4.0 && m44(1,1) ==5.0 && m44(1,2) == 6.0 &&
		   m44(2,0) == 1.0 && m44(2,1) ==2.0 && m44(2,2) == 3.0 );

	//ceil/floor/round

	mat<double> m45(3,3);
	m45(0,0) = 1.1; m45(0,1) = 2.5; m45(0,2) = 3.7;
	m45(1,0) = 4.1; m45(1,1) = 5.5; m45(1,2) = 6.7;
	m45(2,0) = 7.1; m45(2,1) = 8.5; m45(2,2) = 9.7;

	mat<double> m46 =m45;
	mat<double> m47 =m45;
	mat<double> m48 =m45;
	m46.floor();
	assert(m46(0,0) == 1.0 && m46(0,1) ==2.0 && m46(0,2) == 3.0 &&
		   m46(1,0) == 4.0 && m46(1,1) ==5.0 && m46(1,2) == 6.0 &&
		   m46(2,0) == 7.0 && m46(2,1) ==8.0 && m46(2,2) == 9.0 );

	m47.ceil();
	assert(m47(0,0) == 2.0 && m47(0,1) ==3.0 && m47(0,2) == 4.0 &&
		   m47(1,0) == 5.0 && m47(1,1) ==6.0 && m47(1,2) == 7.0 &&
		   m47(2,0) == 8.0 && m47(2,1) ==9.0 && m47(2,2) == 10.0 );

	m48.round();
	assert(m48(0,0) == 1.0 && m48(0,1) ==3.0 && m48(0,2) == 4.0 &&
		   m48(1,0) == 4.0 && m48(1,1) ==6.0 && m48(1,2) == 7.0 &&
		   m48(2,0) == 7.0 && m48(2,1) ==9.0 && m48(2,2) == 10.0 );


	//compute frobenius norm
	mat<double> m49(3,3);
	m49(0,0) = 1; m49(0,1) = 2; m49(0,2) = 3;
	m49(1,0) = 4; m49(1,1) = -5; m49(1,2) = 6;
	m49(2,0) = 7; m49(2,1) = -8; m49(2,2) = 9;
	assert(fabs(m49.frobenius_norm()-sqrt(285.0)) < 0.0001);

	//identity/zeros/ones
	m49.identity();
	assert(m49(0,0) == 1.0 && m49(0,1) ==0.0 && m49(0,2) == 0.0 &&
		   m49(1,0) == 0.0 && m49(1,1) ==1.0 && m49(1,2) == 0.0 &&
		   m49(2,0) == 0.0 && m49(2,1) ==0.0 && m49(2,2) == 1.0 );

	m49.identity(2);
	assert(m49(0,0) == 1.0 && m49(0,1) ==0.0 &&
		   m49(1,0) == 0.0 && m49(1,1) ==1.0 );

	m49.zeros();
	assert(m49(0,0) == 0.0 && m49(0,1) ==0.0 &&
		   m49(1,0) == 0.0 && m49(1,1) ==0.0 );

	m49.ones(1,2);
	assert(m49(0,0) == 1.0 && m49(0,1) ==1.0 );


	//horzcat/vertcat
	mat<double> m50 = horzcat(ones<double>(2,2),zeros<double>(2,2));
	assert(m50.ncols() == 4 && m50.nrows()==2);

	assert(m50(0,0) == 1.0 && m50(0,1) == 1.0 && m50(0,2) == 0.0 && m50(0,3) == 0.0 &&
		   m50(1,0) == 1.0 && m50(1,1) == 1.0 && m50(1,2) == 0.0 && m50(1,3) == 0.0 );

	mat<double> m51 = vertcat(ones<double>(2,2),zeros<double>(2,2));
	assert(m51.ncols() == 2 && m51.nrows()==4);

	assert(m51(0,0) == 1.0 && m51(0,1) == 1.0 &&
		   m51(1,0) == 1.0 && m51(1,1) == 1.0 &&
		   m51(2,0) == 0.0 && m51(2,1) == 0.0 && 
		   m51(3,0) == 0.0 && m51(3,1) == 0.0 );



	//some matrix helper functions
	mat<double> m52(3,3);
	m52(0,0) = 1; m52(0,1) = 2; m52(0,2) = 3;
	m52(1,0) = 4; m52(1,1) = -5; m52(1,2) = 6;
	m52(2,0) = 7; m52(2,1) = -8; m52(2,2) = 9;
	
	mat<double> m53;
	//m53 = transpose(m52)*m52;
	AtA(m52, m53);
	assert(transpose(m52)*m52 == m53);

	//m53 = m52*transpose(m52);
	AAt(m52,m53);
	assert(m52*transpose(m52) == m53);

	mat<double> m54(3,3);
	m54(0,0) = 1; m54(0,1) = 2; m54(0,2) = 3;
	m54(1,0) = 4; m54(1,1) = 5; m54(1,2) = -6;
	m54(2,0) = -7; m54(2,1) = 8; m54(2,2) = -9;

	//m53 = transpose(m52)*m54;
	AtB(m52,m54,m53);
	assert(m53 == transpose(m52)*m54);

	//x2 = transpose(m52)*x2;
	vec<double> x(1.0,2.0,3.0);
	vec<double> x2;
	Atx(m52,x, x2);
	assert(transpose(m52)*x == x2);

	m52 +=3;

	//output
	//std::cout << m52 <<std::endl;


}