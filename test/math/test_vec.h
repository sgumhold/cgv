#include <cgv/math/vec.h>



void test_vec()
{
	using namespace cgv::math;
	//creates an empty vector of zero size
	vec<double> v1;
	assert(v1.size() == 0);

	//creates a vector with 10 elements
	vec<double> v2(10);
	assert(v2.size() == 10);

	//creates a vector with 3 elements from a given array
	double ar[3]={1.0,2.0,3.0};
	vec<double> v4(3,ar);
	assert(v4.size() == 3);
	assert(v4(0) == 1.0 && v4(1) == 2.0 && v4(2) == 3.0);

	//copy constructor test with same value type
	vec<double> v5 = v4;
	assert(v5.size() == v4.size());
	assert(v5(0) == 1.0 && v5(1) == 2.0 && v5(2) == 3.0);

	//copy constructor test with different value type
	vec<float> v6 = v5;
	assert(v6.size() == v5.size());
	assert(v6(0) == 1.0f && v6(1) == 2.0f && v6(2) == 3.0f);

	//creates a 2d vector directly from given elements
	vec<double> v7(0.5,1.5);
	assert(v7.size() == 2);
	assert(v7(0) == 0.5 && v7(1) == 1.5);

	//creates a 3d vector directly from given elements
	vec<double> v8(0.5,1.5,2.5);
	assert(v8.size() == 3);
	assert(v8(0) == 0.5 && v8(1) == 1.5 && v8(2) == 2.5);

	//creates a 4d vector directly from given elements
	vec<double> v9(0.5,1.5,2.5,3.5);
	assert(v9.size() == 4);
	assert(v9(0) == 0.5 && v9(1) == 1.5 && v9(2) == 2.5 && v9(3) == 3.5);

	//set entries of a 2d vector
	vec<double> v10(2);
	v10.set(1.0,2.0);
	assert(v10(0) == 1.0 && v10(1) == 2.0);

	//set entries of a 3d vector
	vec<double> v11(3);
	v11.set(1.0,2.0,3.0);
	assert(v11(0) == 1.0 && v11(1) == 2.0 && v11(2) == 3.0);
	
	//set entries of a 4d vector
	vec<double> v12(4);
	v12.set(1.0,2.0,3.0,4.0);
	assert(v12(0) == 1.0 && v12(1) == 2.0 && v12(2) == 3.0 && v12(3) == 4.0);

	//cast into double*
	double* ptr = (double*)v12;

	//cast into double*
	const double* ptr2 = (const double*)v12;

	//assignments of a vector with same value type
	vec<double> v13(3.0,3.0);
	vec<double> v14(2.0,2.0,2.0);
	vec<double> v15;
	v13 = v14;
	assert(v13.size() == 3);
	assert(v13(0) == 2.0 && v13(1) == 2.0 && v13(2) == 2.0);
	v15 = v14;
	assert(v15.size() == 3);
	assert(v15(0) == 2.0 && v15(1) == 2.0 && v15(2) == 2.0);

	//assignments of a vector with different value type
	vec<double> v16(1.0,1.0);
	vec<float> v17(2.0f,2.0f,2.0f);
	vec<double> v18;
	v16 = v17;
	assert(v16.size() == 3);
	assert(v16(0) == 2.0 && v16(1) == 2.0 && v16(2) == 2.0);
	v18 = v17;
	assert(v18.size() == 3);
	assert(v18(0) == 2.0 && v18(1) == 2.0 && v18(2) == 2.0);

	//in-place addition of scalar
	vec<double> v19(1.0,2.0);
	v19+= 1.0;
	assert(v19(0) ==2.0 && v19(1) == 3.0);

	//in-place addition of scalar
	vec<double> v20(1.0,2.0);
	v20-= 1.0;
	assert(v20(0) ==0.0 && v20(1) == 1.0);

	//in-place multiplication of scalar
	vec<double> v21(1.0,2.0);
	v21*= 2.0;
	assert(v21(0) ==2.0 && v21(1) == 4.0);

	//in-place division of scalar
	vec<double> v22(1.0,2.0);
	v22 /= 2.0;
	assert(v22(0) == 0.5 && v22(1) == 1.0);


	//in-place addition of vector
	vec<double> v23(1.0,2.0);
	vec<double> v24(2.0,1.0);
	v23+=v24;
	assert(v23(0) == 3.0 && v23(1) == 3.0);

	//in-place subtraction of vector
	vec<double> v25(1.0,2.0);
	vec<double> v26(2.0,1.0);
	v25-=v26;
	assert(v25(0) == -1.0 && v25(1) == 1.0);

	//in-place componentwise multiplication of vector
	vec<double> v23a(1.0,2.0);
	vec<double> v24a(2.0,1.0);
	v23a*=v24a;
	assert(v23a(0) == 2.0 && v23a(1) == 2.0);

	//in-place subtraction of vector
	vec<double> v25a(4.0,2.0);
	vec<double> v26a(2.0,1.0);
	v25a/=v26;
	assert(v25a(0) == 2.0 && v25a(1) == 2.0);


	//addition/subtraction/negation of vectors
	vec<double> v27(1.0,2.0);
	vec<double> v28(1.0,2.0);
	vec<double> v29 = v27+v28;
	assert(v29(0) = 2.0 && v29(1) == 4.0);
	vec<double> v30 = v27-v28;
	assert(v30(0) == 0.0 && v30(1) == 0.0);
	vec<double> v31 = -v28;
	assert(v31(0) == -1.0 && v31(1) == -2.0);

	//multiplication with scalar 
	vec<double> v32(-1.0,2.0);
	vec<double> v33 = 3.0*v32;
	assert(v33(0) == -3.0 && v33(1) == 6.0);
	vec<double> v34 = v32*4.0;
	assert(v34(0) == -4.0 && v34(1) == 8.0);

	//division by scalar
	vec<double> v35(4.0,2.0,8.0);
	vec<double> v36 = v35/2.0;
	assert(v36(0) == 2.0 && v36(1) == 1.0 && v36(2) == 4.0);

	//filling and clearing a vector 
	vec<double> v37(2);
	v37.zeros();
	assert(v37(0) == 0.0 && v37(1) == 0.0);
	v37.fill(-1.0);
	assert(v37(0) == -1.0 && v37(1) == -1.0);
	v37.zeros(3);
	assert(v37.size() == 3);
	assert(v37(0) == 0.0 && v37(1) == 0.0 && v37(2) == 0.0);

	//resize
	vec<double> v38(3),v39;
	v38.resize(10);
	assert(v38.size());
	v39.resize(10);
	assert(v39.size() == 10);

	//comparison
	vec<double> v40(1.0,2.0,3.0);
	vec<double> v41(1.0,2.0,3.0);
	vec<double> v42(1.0,2.0,4.0);
	assert(v40 == v41);
	assert(v40 != v42);

	//length and square length
	vec<double> v43(1.0,2.0,3.0);
	assert(fabs(v43.length()-sqrt(14.0)) < 0.000001);
	assert(fabs(v43.sqr_length()-14.0) < 0.000001);
	assert(fabs(length(v43)-sqrt(14.0)) < 0.000001);
	assert(fabs(sqr_length(v43)-14.0) < 0.000001);

	//normalization
	assert(fabs(normalize(v43).length() -1.0) < 0.000001);
	v43.normalize();
	assert(fabs(v43.length() -1.0) < 0.000001);

	//subvec 
	vec<double> v44(1.0,2.0,3.0,4.0);
	vec<double> v45 = v44.sub_vec(2,2);
	assert(v45.size() == 2);
	assert(v45(0) == 3.0 && v45(1) == 4.0);

	//copy
	vec<double> v46(2);
	v44.copy(1,2,v46);
	assert(v46(0) == 2.0 && v46(1) == 3.0);

	//paste
	vec<double> v47(5);
	v47.fill(1.0);
	v47.paste(1,v44);
	assert(v47(0) == 1.0 && v47(1) == 1.0&& v47(2) == 2.0&& v47(3) == 3.0
		&& v47(4) == 4.0);

	//dot product
	vec<double> v48(1.0,2.0,3.0);
	vec<double> v49(4.0,5.0,6.0);
	double r1 = dot(v48,v49);
	assert(fabs(r1 - 32.0) < 0.00001);


	//cross product
	vec<double> v50 = cross(v48,v49);
	assert(v50(0) == -3.0 && v50(1) == 6.0 && v50(2) ==-3);

	//double cross product 
	vec<double> v51(7,8,9);
	vec<double> v52= dbl_cross(v48,v49,v51);
	assert(v52(0) == -24.0 && v52(1) == -6.0 && v52(2) ==12);

	//spat product 
	double r2 = spat(v48,v49,v51);
	assert(r2 == 0);

	

	// test mean/median/std/var/max/min
	vec<double> v56(1.0,3.0,5.0);
	assert(mean_value(v56) == 3.0);
	assert(median_value(v56) == 3.0);
	assert(std_value(v56) == 2.0);
	assert(var_value(v56) == 4.0);
	assert(max_value(v56) == 5.0);
	assert(min_value(v56) == 1.0);
	assert(max_index(v56)  == 2);
	assert(min_index(v56) == 0);

	//sort entries descending
	sort_values(v56,false);
	assert(v56(0) == 5.0  && v56(1) ==  3.0 && v56(2) == 1.0);
	sort_values(v56);
	assert(v56(0) == 1.0  && v56(1) ==  3.0 && v56(2) == 5.0);

	//componentwise absolute values
	vec<double> v57(1.0,-2.0,-3.0);
	vec<double> v58 = abs(v57);
	assert(v58(0) == 1.0 && v58(1) == 2.0 && v58(2) == 3.0);
	v57.abs();
	assert(v57(0) == 1.0 && v57(1) == 2.0 && v57(2) == 3.0);

	//test pnorm/infnorm/length/sqr_length/normalize
	assert(p_norm(v57,1.0) ==6.0);
	assert(p_norm(v57,2.0) -sqrt(14.0) < 0.00001);
	assert(p_norm(v57,3.0) -pow(36.0,1.0/3.0) < 0.00001);
	assert(inf_norm(v57) == 3.0);
	assert(v57.length() == p_norm(v57,2.0));
	assert(v57.length() == sqrt(v57.sqr_length()));
	v57.normalize();
	assert(v57.length() -1.0 < 0.00001);

	//test floor/ceil/round
	vec<double> v59(1.2,1.5,-2.7);
	vec<double> v60 = floor(v59);
	assert(v60(0) == 1.0 && v60(1) == 1.0 && v60(2) == -3.0);
	vec<double> v61 = ceil(v59);
	assert(v61(0) == 2.0 && v61(1) == 2.0 && v61(2) == -2.0);
	vec<double> v62 = round(v59);
	assert(v62(0) == 1.0 && v62(1) == 2.0 && v62(2) == -3.0);

	//test project/reflect/refract
	vec<double> v63(2.0,0.0,0.0);
	vec<double> v64(1.0,1.0,1.0);
	vec<double> v65 = project(v64,v63);
	assert(v65(0) == 1.0 && v65(1) == 0.0 && v65(2) == 0.0);
	vec<double> v66 = reflect(v64,v63);
	assert(v66(0) == -1.0 && v66(1) == 1.0 && v66(2) == 1.0);

	//test linspace and logspace
	vec<double> v67 = lin_space<double>(3.0,1.0,3);
	assert(v67(0) == 3 && v67(1) == 2 && v67(2) == 1);

	vec<double> v68 = log_space<double>(1,3,3);
	assert(v68(0) == 10 && v68(1) ==100 && v68(2) == 1000);

	vec<double> v69 = cheb_points<double>(0,1,3);
	std::cout << "cheb"<<v69<<std::endl;
	

	
	//output
	//std::cout << v70 <<std::endl;



}









