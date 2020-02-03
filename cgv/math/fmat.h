#pragma	once

#include "fvec.h"
#include <cassert>

namespace cgv {
	/// namespace with classes and algorithms for mathematics
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
	///construct a matrix with all elements set to c
	fmat(const T& c) : base_type(c) {}
	///creates a matrix from an array a of given dimensions - by default in column major format
	fmat(cgv::type::uint32_type n, cgv::type::uint32_type m, const T* a, bool column_major = true) {
		for (cgv::type::uint32_type j = 0; j < std::min(m, M); ++j)
			for (cgv::type::uint32_type i = 0; i < std::min(n, N); ++i)
				(*this)(i, j) = a[column_major ? j * N + i : i * M + j];
	}
	///creates a matrix from an array a of given dimensions but different type - by default in column major format
	template <typename S>
	fmat(cgv::type::uint32_type n, cgv::type::uint32_type m, const S* a, bool column_major = true) {
		for (cgv::type::uint32_type j = 0; j < std::min(m, M); ++j)
			for (cgv::type::uint32_type i = 0; i < std::min(n, N); ++i)
				(*this)(i, j) = (T)a[column_major ? j * N + i : i * M + j];
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
	static unsigned nrows() { return N; }
	///number of columns
	static unsigned ncols() { return M; }
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
	bool is_square() const { N == M; }
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

	///matrix vector multiplication
	template < typename S>
	const fvec<T,N> operator * (const fvec<S,M>& v) const {
		fvec<T,N> r;
		for(unsigned i = 0; i < N; i++)
			r(i) = dot(row(i),v);
		return r;
	}
	///extract a row from the matrix as a vector, this is done by a type cast
	const fvec<T,M> row(unsigned i) const {
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
	///extract a column of the matrix as a vector
	const fvec<T,N>& col(unsigned j) const {	
		return *(const fvec<T,N>*)(&operator()(0,j));
	}
	///set  column j of the matrix to vector v
	void set_col(unsigned j,const fvec<T,N>& v) {
		for(unsigned i = 0; i < N;i++) 
			operator()(i,j)=v(i);		
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
template < typename T, cgv::type::uint32_type N, typename S, cgv::type::uint32_type M>
fmat<T, N, M> dyad(const fvec<T,N>& v, const fvec<S,M>& w)
{
	fmat<T, N, M> m;
	for (unsigned i = 0; i < N; i++)
		for (unsigned j = 0; j < M; j++)
			m(i, j) = v(i)*(T)w(j);
	return m;
}


	}

}
