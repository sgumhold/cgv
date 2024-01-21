#pragma	once

#include "fvec.h"
#include <cassert>
#include <initializer_list>

namespace cgv {
namespace math {

//!matrix of fixed size dimensions
/*!Template arguments are
   - \c T ... coordinate type
   - \c N ... number of rows
   - \c M ... number of columns
   Matrix elements can be accessed with the \c (i,j)-\c operator with 0-based
   indices. For example \c A(i,j) accesses the matrix element in the (i+1)th 
   row and the (j+1)th column.

   The matrix inherits the functionality of a \c N*M dimensional vector
   and is stored in column major format. This means that \c A(i,j)=A(j*M+i). */
template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
class fmat : public fvec<T,N*M>
{
public:
	///base type is a vector with sufficent number of elements
	typedef fvec<T,N*M> base_type;
	///base type is a vector with sufficent number of elements
	typedef fmat<T,N,M> this_type;
	///standard constructor 
	fmat() {}
	///construct from individual components using list-initialization syntax
	fmat(std::initializer_list<T> components) : fvec<T,N*M>((cgv::type::uint32_type)components.size(), components.begin()) {}
	///construct from column vectors using list-initialization syntax
	fmat(std::initializer_list<fvec<T,N>> cols) : fvec<T,N*M>(N*(cgv::type::uint32_type)cols.size(), (T*)cols.begin()) {}
	///construct a matrix with all elements set to c
	fmat(const T& c) : base_type(c) {}
	///creates a matrix from an array a of given dimensions - by default in column major format - and fills missing entries from identity matrix
	fmat(cgv::type::uint32_type n, cgv::type::uint32_type m, const T* a, bool column_major = true) {
		cgv::type::uint32_type j;
		for (j = 0; j < std::min(m, M); ++j) {
			cgv::type::uint32_type i;
			for (i = 0; i < std::min(n, N); ++i)
				(*this)(i, j) = a[column_major ? j * n + i : i * m + j];
			for (; i < N; ++i)
				(*this)(i, j) = (i == j) ? T(1) : T(0);
		}
		for (; j < M; ++j)
			for (cgv::type::uint32_type i = 0; i < N; ++i)
				(*this)(i, j) = (i == j) ? T(1) : T(0);
	}
	///creates a matrix from an array a of given dimensions but different type - by default in column major format
	template <typename S>
	fmat(cgv::type::uint32_type n, cgv::type::uint32_type m, const S* a, bool column_major = true) {
		for (cgv::type::uint32_type j = 0; j < std::min(m, M); ++j)
			for (cgv::type::uint32_type i = 0; i < std::min(n, N); ++i)
				(*this)(i, j) = (T)a[column_major ? j * n + i : i * m + j];
	}
	///copy constructor for matrix with different element type
	template <typename S>
	fmat(const fmat<S,N,M>& m) : base_type(m) {}
	///construct from outer product of vector v and w
	template <typename T1, typename T2>
	fmat(const fvec<T1,N>& v, const fvec<T2,M>& w) { 
		for(unsigned i = 0; i < N; i++)
			for(unsigned j = 0; j < M; j++)
				(*this)(i,j) = (T)(v(i)*w(j)); 
	}
	///number of rows
	constexpr static unsigned nrows() { return N; }
	///number of columns
	constexpr static unsigned ncols() { return M; }
	///assignment of a matrix with a different element type
	template <typename S> 
	fmat<T,N,M>& operator = (const fmat<S,N,M>& m) {
		base_type::operator = (m);
		return *this;
	}
	///assignment of a scalar s to each element of the matrix
	this_type& operator  = (const T& s) { 
		fill (s);
		return *this; 
	}
	///returns true if matrix is a square matrix
	bool is_square() const { return N == M; }
	///access to the element in the ith row in column j
	T& operator () (unsigned i, unsigned j) {
		assert(i < N && j < M);
		return base_type::v[j*N+i];
	}
	///const access to the element in the ith row on column j 
	const T& operator () (unsigned i, unsigned j) const {
		assert(i < N && j < M);
		return base_type::v[j*N+i];
	}
	//in place scalar multiplication
	this_type& operator *= (const T& s)	{ base_type::operator *= (s); return *this; } 
	///scalar multiplication  
	this_type operator * (const T& s) const { this_type r=(*this); r*=(T)s; return r; }
	///in place division by a scalar
	fmat<T,N,M>& operator /= (const T& s) { base_type::operator /= (s); return *this; }
	/// division by a scalar
	const fmat<T,N,M> operator / (const T& s) const	{ this_type r=(*this); r/=(T)s; return r; }
	///in place addition by a scalar
	fmat<T,N,M>& operator += (const T& s) { base_type::operator += (s); return *this; }
	///componentwise addition of a scalar
	const fmat<T,N,M> operator + (const T& s) { this_type r=(*this); r+=(T)s; return r; }
	///in place substraction of a scalar
	fmat<T,N,M>& operator -= (const T& s) { base_type::operator -= (s); return *this; }
	/// componentwise subtraction of a scalar
	const fmat<T,N,M> operator - (const T& s) { this_type r=(*this); r-=(T)s; return r; }
	///negation operator
	const fmat<T,N,M> operator-() const { return (*this)*(-1); }
	///in place addition of matrix
	template <typename S> 
	fmat<T,N,M>& operator += (const fmat<S,N,M>& m) { base_type::operator += (m); return *this; } 
	///in place subtraction of matrix
	template <typename S> 
	fmat<T,N,M>& operator -= (const fmat<S,N,M>& m) { base_type::operator -= (m); return *this; } 
	///matrix addition
	template <typename S>
	const fmat<T,N,M> operator+(const fmat<S,N,M> m2)const { fmat<T,N,M> r=(*this); r += m2; return r; }
	///matrix subtraction
	template <typename S>
	const fmat<T,N,M> operator-(const fmat<S,N,M> m2)const { fmat<T,N,M> r=(*this); r -= m2; return r; }
	///in place matrix multiplication with  a ncols x ncols matrix m2
	template <typename S>
	const fmat<T,N,M> operator*=(const fmat<S,N,N>& m2) 
	{
		assert(N == M);
		fmat<T,N,N> r(T(0));	
		for(unsigned i = 0; i < N; i++)
			for(unsigned j = 0; j < N;j++)
				for(unsigned k = 0; k < N; k++)
					r(i,j) += operator()(i,k) * (T)(m2(k,j)); 
		(*this)=r;
		return *this;
	}
	///multiplication with a ncols x M matrix m2
	template <typename S, cgv::type::uint32_type L>
	const fmat<T,N,L> operator*(const fmat<S,M,L>& m2) const
	{
		fmat<T,N,L> r; r.zeros();	
		for(unsigned i = 0; i < N; i++)
			for(unsigned j = 0; j < L;j++)
				for(unsigned k = 0; k < M; k++)
					r(i,j) += operator()(i,k) * (T)(m2(k,j)); 
		return r;
	}
	///multiplication with (N-1)x(N-1) matrix, assuming the first operand represents an affine
	///or perspective transformation to be combined with the linear transformation represented by the
	///second operand (which will be treated as if lifted to a homogenous transformation matrix)
	template <typename S>
	const fmat<T,N,N> mul_h (const fmat<S,N-1,N-1>& m2) const
	{
		static_assert(N == M, "Number of rows and columns must be equal");
		static const auto vzero = fvec<T, N-1>(0);
		fvec<T,N> rows[N]; // extracting a row takes linear time so we only want to do it once for each row
		for (unsigned i=0; i<N; i++)
			rows[i] = row(i);
		fmat<T,N,N> r;
		// (1) multiply with implied N x (N-1) matrix that is assumed to have an all-zero last row
		for (unsigned j=0; j<N-1; j++)
			for (unsigned i=0; i<N; i++)
				r(i,j) = dot_dir(rows[i], m2.col(j));
		// (2) assume homogeneous zero position vector in last column of 2nd operand for calculating last result column
		for (unsigned i=0; i<N; i++)
			r(i,N-1) = dot_pos(rows[i], vzero);
		return r;
	}

	///matrix vector multiplication
	template <typename S>
	const fvec<S,N> operator * (const fvec<S,M>& v) const {
		fvec<S,N> r;
		for(unsigned i = 0; i < N; i++)
			r(i) = dot(row(i),v);
		return r;
	}
	///multiplication with M-1 dimensional position vector which will be implicitly homogenized
	template <typename S>
	const fvec<S,N> mul_pos (const fvec<S,M-1>& v) const {
		fvec<S,N> r;
		for(unsigned i = 0; i < N; i++)
			r(i) = dot_pos(row(i), v);
		return r;
	}
	///multiplication with M-1 dimensional direction vector which will be implicitly homogenized
	template <typename S>
	const fvec<S,N> mul_dir (const fvec<S,M-1>& v) const {
		fvec<S,N> r;
		for(unsigned i = 0; i < N; i++)
			r(i) = dot_dir(row(i), v);
		return r;
	}

	///extract a row from the matrix as a vector, this takes time linear in the number of columns
	fvec<T,M> row(unsigned i) const {
		fvec<T,M> r;
		for(unsigned j = 0; j < M; j++) 
			r(j)=operator()(i,j);
		return r;
	}
	///set row i of the matrix to vector v
	void set_row(unsigned i,const fvec<T,M>& v) {
		for(unsigned j = 0; j < M;j++) 
			operator()(i,j)=v(j);		
	}
	///reference a column of the matrix as a vector
	fvec<T,N>& col(unsigned j) {	
		return reinterpret_cast<fvec<T,N>*>(this)[j];
	}
	///read-only reference a column of the matrix as a vector
	const fvec<T,N>& col(unsigned j) const {	
		return reinterpret_cast<const fvec<T,N>*>(this)[j];
	}
	///set column j of the matrix to vector v
	void set_col(unsigned j,const fvec<T,N>& v) {
		reinterpret_cast<fvec<T,N>*>(this)[j] = v;
	}

	///returns the trace 
	T trace() const {
		assert(N == M);
		T t = 0;
		for(unsigned i = 0; i < N;i++)
			t+=operator()(i,i);
		return t;
	}

	///transpose matrix
	void transpose() {
		assert(N == M);
		for(unsigned i = 1; i < N; i++)
			for(unsigned j = 0; j < i; j++)
				std::swap(operator()(i,j), operator()(j,i));
	}
	///returns the frobenius norm of matrix m
	T frobenius_norm() const { return base_type::length(); }
	///set identity matrix
	void identity()
	{
		base_type::zeros();
		for(unsigned i = 0; i < M && i < N; ++i)
			operator()(i,i)=1;
	}
};

/// return the transposed of a square matrix
template <typename T, cgv::type::uint32_type N>
fmat<T,N,N> transpose(const fmat<T,N,N>& m)
{
	fmat<T,N,N> m_t(m);
	m_t.transpose();
	return m_t; 
}

///return the product of a scalar s and a matrix m
template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
fmat<T,N,M> operator * (const T& s, const fmat<T,N,M>& m)
{ 
	return m*s; 
}

/// multiply a row vector from the left to matrix m and return a row vector
template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
fvec<T, M> operator * (const fvec<T, N>& v_row, const fmat<T, N, M>& m)
{
	fvec<T, M> r_row;
	for (unsigned i = 0; i < M; i++)
		r_row(i) = dot(m.col(i), v_row);
	return r_row;
}

///output of a matrix onto an ostream
template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
std::ostream& operator<<(std::ostream& out, const fmat<T,N,M>& m)
{
	for (unsigned i=0;i<N;++i) {
		for(unsigned j =0;j < M-1;++j)
			out << m(i,j) << " ";
		out << m(i,M-1);
		if(i != N-1)
			out <<"\n";
	}
	return out;
}

///input of a matrix onto an ostream
template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
std::istream& operator>>(std::istream& in, fmat<T,N,M>& m)
{
	for (unsigned i=0;i<m.nrows();++i)
		for(unsigned j =0;j < m.ncols();++j)
			in >> m(i,j);
	return in;
}

///returns the outer product of vector v and w
template <typename T, cgv::type::uint32_type N, typename S, cgv::type::uint32_type M>
fmat<T, N, M> dyad(const fvec<T,N>& v, const fvec<S,M>& w)
{
	fmat<T, N, M> m;
	for (unsigned i = 0; i < N; i++)
		for (unsigned j = 0; j < M; j++)
			m(i, j) = v(i)*(T)w(j);
	return m;
}

///returns the determinant of a 2x2 matrix
template <typename T>
T det(const fmat<T, 2, 2>& m) {
	return m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);
}

///returns the determinant of a 3x3 matrix
template <typename T>
T det(const fmat<T, 3, 3>& m) {
	T a = m(0, 0) * m(1, 1) * m(2, 2) + m(0, 1) * m(1, 2) * m(2, 0) + m(0, 2) * m(1, 0) * m(2, 1);
	T b = -m(2, 0) * m(1, 1) * m(0, 2) - m(2, 1) * m(1, 2) * m(0, 0) - m(2, 2) * m(1, 0) * m(0, 1);
	return a + b;
}

///linear interpolation returns (1-t)*m1 + t*m2
template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
const fmat<T, N, M> lerp(const fmat<T, N, M>& m1, const fmat<T, N, M>& m2, T t)
{
	fmat<T, N, M> m;
	for(unsigned i = 0; i < N; i++)
		for(unsigned j = 0; j < M; j++)
			m(i, j) = ((T)1 - t)*m1(i, j) + t * m2(i, j);
	return m;
}

///linear interpolation returns (1-t)*m1 + t*m2
template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
const fmat<T, N, M> lerp(const fmat<T, N, M>& m1, const fmat<T, N, M>& m2, fmat<T, N, M> t)
{
	fmat<T, N, M> m;
	for(unsigned i = 0; i < N; i++)
		for(unsigned j = 0; j < M; j++)
			m(i, j) = ((T)1 - t(i, j))*m1(i, j) + t(i, j)*m2(i, j);
	return m;
}

} // namespace math

/// @name Predefined Types
/// @{

/// declare type of 2x2 matrices
typedef cgv::math::fmat<float, 2, 2> mat2;
/// declare type of 2x3 matrices used to store camera matrix
typedef cgv::math::fmat<float, 2, 3> mat23;
/// declare type of 3x3 matrices
typedef cgv::math::fmat<float, 3, 3> mat3;
/// declare type of 4x4 matrices
typedef cgv::math::fmat<float, 4, 4> mat4;
/// declare type of 3x4 matrices which are often used to store a pose
typedef cgv::math::fmat<float, 3, 4> mat34;

/// declare type of 2x2 matrices
typedef cgv::math::fmat<double, 2, 2> dmat2;
/// declare type of 3x3 matrices
typedef cgv::math::fmat<double, 3, 3> dmat3;
/// declare type of 4x4 matrices
typedef cgv::math::fmat<double, 4, 4> dmat4;
/// declare type of 3x4 matrices which are often used to store a pose
typedef cgv::math::fmat<double, 3, 4> dmat34;

/// @}

}// namespace cgv
