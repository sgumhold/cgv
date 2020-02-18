#pragma once

#include <cgv/math/lin_solve.h>
#include <cgv/math/fmat.h>

namespace cgv {
	namespace math {

///returns the inverse of a diagonal matrix
template <typename T>
diag_mat<T> inv(const diag_mat<T>& m) 
{
	diag_mat<T> im(m.size());
	for(unsigned i = 0; i < m.nrows();i++)
		im(i) = (T)(1.0/m(i));
	return im;
}

///returns the inverse of a lower triangular matrix
template <typename T>
low_tri_mat<T> inv(const low_tri_mat<T>& m) 
{
	unsigned N = m.nrows();
	low_tri_mat<T> im(N,(T)0.0);
	T sum;
	for(unsigned k = 0; k < N;k++)
	{	
		for(unsigned i = k; i < N;i++)
		{
			sum =0;
			for(unsigned j = k;j < i; j++) 
				sum += m(i,j)*im(j,k);
			assert(m(i,i) != 0);// not invertible
			if(i == k) 
				im(i,k) = (1 - sum)/m(i,i);
			else
				im(i,k) = ( -sum)/m(i,i);
		}
	}
	
	
	return im;
}

///returns the inverse of an upper triangular matrix
template <typename T>
up_tri_mat<T> inv(const up_tri_mat<T>& m) 
{
	up_tri_mat<T> im(m.nrows(),(T)0.0);
	unsigned N = m.nrows();
	vec<double> x;
	vec<double> b(N);
	x.resize(N);
	T sum;
	for(unsigned k = 0; k < N;k++)
	{
		for(int i = k; i >= 0;i--)
		{
			sum =0;
			for(unsigned j = i+1;j <= k; j++) 
				sum += m(i,j)*im(j,k);
			assert(m(i,i) != 0);
				
			
			if(k == i)
				im(i,k) = ((T)1.0 - sum)/m(i,i);
			else
				im(i,k) = ( - sum)/m(i,i);
		}
	}
	return im;
}

///returns the inverse of a square matrix
template <typename T>
mat<T> inv(const mat<T>& m)
{
	assert(m.nrows() == m.ncols());
	mat<T> im(m.nrows(), m.nrows());
	solve(m, identity<T>(m.ncols()), im);
	return im;
}

///compute inverse of 2x2 matrix
template <typename T>
mat<T> inv_22(const mat<T>& m)
{
	mat<T> im(2, 2);
	T t4 = 1.0 / (-m(0, 0) * m(1, 1) + m(0, 1) * m(1, 0));
	im(0, 0) = -m(1, 1) * t4;
	im(1, 0) = m(1, 0) * t4;
	im(0, 1) = m(0, 1) * t4;
	im(1, 1) = -m(0, 0) * t4;

	return im;
}

///compute inverse of 3x3 matrix
template <typename T>
mat<T> inv_33(const mat<T>& m)
{
	mat<T> im(3, 3);
	T t4 = m(2, 0) * m(0, 1);
	T t6 = m(2, 0) * m(0, 2);
	T t8 = m(1, 0) * m(0, 1);
	T t10 = m(1, 0) * m(0, 2);
	T t12 = m(0, 0) * m(1, 1);
	T t14 = m(0, 0) * m(1, 2);
	T t17 = (T)1.0 / (t4 * m(1, 2) - t6 * m(1, 1) - t8 * m(2, 2) + t10 * m(2, 1) + t12 * m(2, 2) - t14 * m(2, 1));
	im(0, 0) = (m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1)) * t17;
	im(0, 1) = -(m(0, 1) * m(2, 2) - m(0, 2) * m(2, 1)) * t17;
	im(0, 2) = (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1)) * t17;
	im(1, 0) = -(-m(2, 0) * m(1, 2) + m(1, 0) * m(2, 2)) * t17;
	im(1, 1) = (-t6 + m(0, 0) * m(2, 2)) * t17;
	im(1, 2) = -(-t10 + t14) * t17;
	im(2, 0) = (-m(2, 0) * m(1, 1) + m(1, 0) * m(2, 1)) * t17;
	im(2, 1) = -(-t4 + m(0, 0) * m(2, 1)) * t17;
	im(2, 2) = (-t8 + t12) * t17;
	return im;
}

///compute inverse of 4x4 matrix
template <typename T>
mat<T> inv_44(const mat<T>& m) 
{
	mat<T> im(4,4);
	T t1 = m(3,3) * m(1,1);
	T t3 = m(3,2) * m(1,1);
	T t7 = m(3,1) * m(1,2);
	T t9 = m(3,1) * m(1,3);
	T t11 = m(3,2) * m(2,1);
	T t14 = m(0,0) * m(1,1);
	T t19 = m(0,0) * m(3,3);
	T t20 = m(1,2) * m(2,1);
	T t22 = m(3,1) * m(0,0);
	T t23 = m(1,2) * m(2,3);
	T t25 = m(1,3) * m(2,2);
	T t27 = m(3,2) * m(0,0);
	T t28 = m(2,1) * m(1,3);
	T t30 = m(1,1) * m(3,0);
	T t31 = m(0,3) * m(2,2);
	T t33 = m(2,0) * m(0,3);
	T t35 = m(0,2) * m(2,3);
	T t37 = m(2,0) * m(0,2);
	T t39 = m(3,0) * m(0,2);
	T t41 = m(3,1) * m(1,0);
	T t43 = t14 * m(3,3) * m(2,2) - t14 * m(3,2) * m(2,3) - t19 * t20 + 
		t22 * t23 - t22 * t25 + t27 * t28 - t30 * t31 + t3 * t33 + t30 * t35 
		- t1 * t37 - t39 * t28 - t41 * t35;
	T t45 = m(3,0) * m(0,1);
	T t47 = m(1,0) * m(3,3);
	T t50 = m(2,0) * m(3,3);
	T t51 = m(0,1) * m(1,2);
	T t53 = m(3,2) * m(1,0);
	T t56 = m(0,2) * m(2,1);
	T t58 = m(3,0) * m(0,3);
	T t63 = m(3,2) * m(2,0);
	T t64 = m(0,1) * m(1,3);
	T t66 = m(1,0) * m(0,3);
	T t68 = -t7 * t33 - t45 * t23 - t47 * m(0,1) * m(2,2) + t50 * t51 + t53 *
		m(0,1) * m(2,3) + t47 * t56 + t58 * t20 + t9 * t37 + t41 * t31 + t45 *
		t25 - t63 * t64 - t11 * t66;
	T t70 = (T)1.0 / (t43 + t68);
	T t72 = m(3,3) * m(0,1);
	T t74 = m(3,2) * m(0,1);
	T t78 = m(0,3) * m(3,1);
	T t108 = m(2,0) * m(1,2);
	T t111 = m(1,3) * m(3,0);
	T t131 = m(0,0) * m(1,2);
	T t135 = m(1,0) * m(0,2);
	T t148 = m(3,1) * m(2,0);
	T t150 = m(1,0) * m(2,1);
	T t156 = m(0,0) * m(2,1);
	T t158 = m(0,0) * m(2,3);
	T t161 = m(2,0) * m(0,1);
	im(0,0) = (t1 * m(2,2) - t3 * m(2,3) - m(3,3) * m(1,2) * m(2,1) + 
		t7 * m(2,3) - t9 * m(2,2) + t11 * m(1,3)) * t70;
	im(0,1) = -(t72 * m(2,2) - t74 * m(2,3) - t56 * m(3,3) + t35 * m(3,1) - 
		t78 * m(2,2) + m(0,3) * m(3,2) * m(2,1)) * t70;
	im(0,2) = (t72 * m(1,2) - t74 * m(1,3) - t1 * m(0,2) + m(0,2) * m(3,1) *
		m(1,3) + t3 * m(0,3) - t78 * m(1,2)) * t70;
	im(0,3) = -(t51 * m(2,3) - t64 * m(2,2) - m(1,1) * m(0,2) * m(2,3) + t56 *
		m(1,3) + m(1,1) * m(0,3) * m(2,2) - m(0,3) * m(1,2) * m(2,1)) * t70;
	im(1,0) = -(t47 * m(2,2) - t53 * m(2,3) + m(1,3) * m(3,2) * m(2,0) - t108 *
		m(3,3) + t23 * m(3,0) - t111 * m(2,2)) * t70;
	im(1,1) = (t19 * m(2,2) - t27 * m(2,3) - t58 * m(2,2) + t63 * m(0,3) + t39 *
		m(2,3) - t50 * m(0,2)) * t70;
	im(1,2) = -(t19 * m(1,2) - t27 * m(1,3) - t47 * m(0,2) - t58 * m(1,2) + t111 *
		m(0,2) + t66 * m(3,2)) * t70;
	im(1,3) = (t131 * m(2,3) - m(0,0) * m(1,3) * m(2,2) - t135 * m(2,3) - t108 *
		m(0,3) + m(1,3) * m(2,0) * m(0,2) + t66 * m(2,2)) * t70;
	im(2,0) = (-m(1,1) * m(2,0) * m(3,3) + m(1,1) * m(2,3) * m(3,0) - t28 *
		m(3,0) + t148 * m(1,3) + t150 * m(3,3) - m(2,3) * m(3,1) * m(1,0)) * t70;
	im(2,1) = -(t156 * m(3,3) - t158 * m(3,1) + t33 * m(3,1) - t161 * m(3,3) - m(2,1) *
		m(3,0) * m(0,3) + m(2,3) * m(3,0) * m(0,1)) * t70;
	im(2,2) = (t19 * m(1,1) - t22 * m(1,3) - t58 * m(1,1) - t47 * m(0,1) + t41 *
		m(0,3) + t45 * m(1,3)) * t70;
	im(2,3) = -(-m(2,3) * m(1,0) * m(0,1) + t158 * m(1,1) - t33 * m(1,1) + t161 *
		m(1,3) - t156 * m(1,3) + t150 * m(0,3)) * t70;
	im(3,0) = -(-t3 * m(2,0) + t30 * m(2,2) + t11 * m(1,0) - m(3,0) * m(1,2) *
		m(2,1) - t41 * m(2,2) + t7 * m(2,0)) * t70;
	im(3,1) = (-t22 * m(2,2) + t27 * m(2,1) - t39 * m(2,1) + t148 * m(0,2) + t45 *
		m(2,2) - t63 * m(0,1)) * t70;
	im(3,2) = -(-t53 * m(0,1) + t27 * m(1,1) - t39 * m(1,1) + t41 * m(0,2) - t22 *
		m(1,2) + t45 * m(1,2)) * t70;
	im(3,3) = t70 * (t161 * m(1,2) - t37 * m(1,1) - m(1,0) * m(0,1) * m(2,2) + t135 *
		m(2,1) + t14 * m(2,2) - t131 * m(2,1));

	return im;
}

/// return the inverse of a square matrix
template <typename T, cgv::type::uint32_type N>
fmat<T, N, N> inv(const fmat<T, N, N>& m)
{
	mat<T> M(N, N, &m(0, 0));
	return fmat<T, N, N>(N, N, &(inv(M)(0, 0)));
}

///compute inverse of 2x2 matrix
template <typename T>
fmat<T, 2, 2> inv(const fmat<T, 2, 2>& m)
{
	fmat<T, 2, 2> im;
	T t4 = 1.0 / (-m(0, 0) * m(1, 1) + m(0, 1) * m(1, 0));
	im(0, 0) = -m(1, 1) * t4;
	im(1, 0) = +m(1, 0) * t4;
	im(0, 1) = +m(0, 1) * t4;
	im(1, 1) = -m(0, 0) * t4;

	return im;
}

///compute inverse of 3x3 matrix
template <typename T>
fmat<T, 3, 3> inv(const fmat<T, 3, 3>& m)
{
	T inv_det = (T)1 / (
		+m(0, 0) * (m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1))
		- m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0))
		+ m(0, 2) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0)));

	fmat<T, 3, 3> im;
	im(0, 0) = +(m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1)) * inv_det;
	im(0, 1) = -(m(0, 1) * m(2, 2) - m(0, 2) * m(2, 1)) * inv_det;
	im(0, 2) = +(m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1)) * inv_det;
	im(1, 0) = -(m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)) * inv_det;
	im(1, 1) = +(m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0)) * inv_det;
	im(1, 2) = -(m(0, 0) * m(1, 2) - m(0, 2) * m(1, 0)) * inv_det;
	im(2, 0) = +(m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0)) * inv_det;
	im(2, 1) = -(m(0, 0) * m(2, 1) - m(0, 1) * m(2, 0)) * inv_det;
	im(2, 2) = +(m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0)) * inv_det;

	return im;
}

///compute inverse of 4x4 matrix
template <typename T>
fmat<T, 4, 4> inv(const fmat<T, 4, 4>& m)
{
	T coef00 = m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2);
	T coef02 = m(2, 1) * m(3, 3) - m(2, 3) * m(3, 1);
	T coef03 = m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1);

	T coef04 = m(1, 2) * m(3, 3) - m(1, 3) * m(3, 2);
	T coef06 = m(1, 1) * m(3, 3) - m(1, 3) * m(3, 1);
	T coef07 = m(1, 1) * m(3, 2) - m(1, 2) * m(3, 1);

	T coef08 = m(1, 2) * m(2, 3) - m(1, 3) * m(2, 2);
	T coef10 = m(1, 1) * m(2, 3) - m(1, 3) * m(2, 1);
	T coef11 = m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1);

	T coef12 = m(0, 2) * m(3, 3) - m(0, 3) * m(3, 2);
	T coef14 = m(0, 1) * m(3, 3) - m(0, 3) * m(3, 1);
	T coef15 = m(0, 1) * m(3, 2) - m(0, 2) * m(3, 1);

	T coef16 = m(0, 2) * m(2, 3) - m(0, 3) * m(2, 2);
	T coef18 = m(0, 1) * m(2, 3) - m(0, 3) * m(2, 1);
	T coef19 = m(0, 1) * m(2, 2) - m(0, 2) * m(2, 1);

	T coef20 = m(0, 2) * m(1, 3) - m(0, 3) * m(1, 2);
	T coef22 = m(0, 1) * m(1, 3) - m(0, 3) * m(1, 1);
	T coef23 = m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1);

	fvec<T, 4> fac0(coef00, coef00, coef02, coef03);
	fvec<T, 4> fac1(coef04, coef04, coef06, coef07);
	fvec<T, 4> fac2(coef08, coef08, coef10, coef11);
	fvec<T, 4> fac3(coef12, coef12, coef14, coef15);
	fvec<T, 4> fac4(coef16, coef16, coef18, coef19);
	fvec<T, 4> fac5(coef20, coef20, coef22, coef23);

	fvec<T, 4> vec0(m(0, 1), m(0, 0), m(0, 0), m(0, 0));
	fvec<T, 4> vec1(m(1, 1), m(1, 0), m(1, 0), m(1, 0));
	fvec<T, 4> vec2(m(2, 1), m(2, 0), m(2, 0), m(2, 0));
	fvec<T, 4> vec3(m(3, 1), m(3, 0), m(3, 0), m(3, 0));

	fvec<T, 4> inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
	fvec<T, 4> inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
	fvec<T, 4> inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
	fvec<T, 4> inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

	fvec<T, 4> sign_a(+(T)1, -(T)1, +(T)1, -(T)1);
	fvec<T, 4> sign_b(-(T)1, +(T)1, -(T)1, +(T)1);
	fmat<T, 4, 4> im;
	im.set_col(0, inv0 * sign_a);
	im.set_col(1, inv1 * sign_b);
	im.set_col(2, inv2 * sign_a);
	im.set_col(3, inv3 * sign_b);

	fvec<T, 4> row0(im(0, 0), im(1, 0), im(2, 0), im(3, 0));

	fvec<T, 4> dot0(m.row(0) * row0);
	T dot1 = (dot0[0] + dot0[1]) + (dot0[2] + dot0[3]);

	T inv_det = (T)1 / dot1;

	return im * inv_det;
}

inline const perm_mat inv(const perm_mat &p)
{
	perm_mat r=p;
	r.transpose();
	return r;
}
	}
}
