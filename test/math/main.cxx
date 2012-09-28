#include <test/math/test_chol.h>
#include <test/math/test_det.h>
#include <test/math/test_inv.h>
#include <test/math/test_lin_solve.h>
#include <test/math/test_low_tri_mat.h>
#include <test/math/test_lu.h>
#include <test/math/test_mat.h>
#include <test/math/test_perm_mat.h>
#include <test/math/test_point_operations.h>
#include <test/math/test_polar.h>
#include <test/math/test_qr.h>
#include <test/math/test_quat.h>
#include <test/math/test_random.h>
#include <test/math/test_svd.h>
#include <test/math/test_transformations.h>
#include <test/math/test_up_tri_mat.h>
#include <test/math/test_vec.h>
#include <test/math/test_eig.h>
#include <test/math/test_gaussj.h>
#include <test/math/test_sparse_mat.h>
#include <test/math/test_polynomial.h>
#include <test/math/test_bi_polynomial.h>
#include <test/math/test_model_comp.h>
#include <test/math/test_distance_transform.h>
#include <test/math/test_fibo_heap.h>
#include <test/math/test_statistics.h>





int main()
{

	test_chol();//complete
	test_det();//complete
	test_inv();//complete	
	test_lu();//complete
	test_polar();//complete
	test_qr();//complete
	test_svd();//complete
	//test_vec();//complete
	test_eig();//complete
	test_mat();//complete
	test_gaussj();//
	test_statistics();
	//test_lin_solve();
	/*test_low_tri_mat();
	test_transformations();
	test_up_tri_mat();
	test_quat();
	//test_random();
	test_perm_mat();
	test_point_operations();
	//test_polynomial();
	test_bi_polynomial();
	//test_distance_transform();
//	test_model_comparison();
//	test_sparse_mat();
	//test_fibo_heap();//complete*/
	std::cout << "all tests successful!\n";
	return 0;
}