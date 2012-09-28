#pragma once

#include "vec.h"
#include "mat.h"
#include "perm_mat.h"

namespace cgv {
	namespace math {

//lower triangular matrix type
template <typename T>
class low_tri_mat 
{

protected:
	//data storage
	vec<T> _data;
	//matrix dimension
	unsigned _dim;
	

public:
	
	//standard constructor of a lower triangular matrix
	low_tri_mat()
	{
		_dim=0;
	}

	//creates a dim x dim lower triangular matrix
	low_tri_mat(unsigned dim)
	{
		resize(dim);
		fill(0);
	}

	//copy constructor
	low_tri_mat(const low_tri_mat& m)
	{
		resize(m.dim());
		memcpy(_data,m._data,size()*sizeof(T));
	}

	//create a dim x dim lower triangular matrix with all non-zero elements set to c
	low_tri_mat(unsigned dim, const T& c)
	{
		resize(dim);
		fill(c);
	}

	//destructor
	virtual ~low_tri_mat()
	{		
	}

	//resize the matrix to an n x n matrix
	void resize(unsigned n)
	{
		_dim = n;
		_data.resize(n*(n+1)/2);
	}

	//cast into full storage matrix
	operator mat<T>()
	{
		mat<T> m(_dim,_dim);
		for(unsigned i =0; i < _dim;i++)
		{
			for(unsigned j = 0; j < _dim;j++)
			{
				if(i<j)
					m(i,j)=0;
				else
					m(i,j)=operator()(i,j);
			}
			
		}
		return m;
	}

	///assignment of a matrix with the same element type
	low_tri_mat<T>& operator = (const low_tri_mat<T>& m) 
	{  
		
		resize(m.dim());
		_data=m._data;
		return *this;
	}

	//cast into const full storage matrix
	operator const mat<T>() const
	{
		mat<T> m(_dim,_dim);
		for(unsigned i =0; i < _dim;i++)
		{
			for(unsigned j = 0; j < _dim;j++)
			{
				if(i<j)
					m(i,j)=0;
				else
					m(i,j)=operator()(i,j);
			}
			
		}
		return m;
	}


	//return number of stored elements
	unsigned size() const
	{
		return _data.size();
	}

	//return dimension of the matrix
	unsigned dim() const
	{
		return _dim;
	}

	//returns the number of columns / rows of the matrix
	unsigned nrows() const
	{
		return _dim;
	}

	unsigned ncols() const
	{
		return _dim;
	}

	//fills all lower triangular elements with c
	void fill(const T& c)
	{
		for(unsigned i=0;i < size(); i++)
		{
			_data[i]=c;
		}
	}

	//access to the element (i,j)
	T& operator() (const unsigned i,const unsigned j) 
	{
		assert( i >= j && i < _dim);
		return _data[i*(i+1)/2+j]; 
	}
	
	//const access to the element (i,j)
	const T& operator() (unsigned i, unsigned j) const 
	{
		assert( i >= j && i < _dim);
		return _data[i*(i+1)/2+j]; 
	}

	//return true if (i,j) is a valid index pair
	bool valid_ind(int i, int j) const
	{
		return ( i >= j && i < (int)_dim && i >= 0 && j >= 0);
	}

	template < typename S>
	const mat<T> operator*(const mat<S>& m2) 
	{
		assert(m2.nrows() == nrows());
		unsigned M = m2.ncols();
		mat<T> r(nrows(),M,(T)0);
		for(unsigned i = 0; i < nrows(); i++)
			for(unsigned j = 0; j < M;j++)
				for(unsigned k = 0; k <= i; k++)
					r(i,j) += operator()(i,k) * (T)(m2(k,j)); 

		return r;
	}


};

///output of a lower triangular matrix onto an ostream
template <typename T>
std::ostream& operator<<(std::ostream& out, const low_tri_mat<T>& m)
{
	
	for(unsigned i =0;i < m.nrows() ;++i)
	{
		unsigned j = 0;
		for (; j<i; ++j)
			out << m(i,j)<<"\t";
		out << m(i,j)<<"\n";		
	}

	return out;

}

///multiplication of two lower triangular matrices
template <typename T, typename S>
const mat<T> operator*(const low_tri_mat<T>& m1, const low_tri_mat<S>& m2) 
{
	assert(m2.nrows() == m1.nrows());
	unsigned M = m2.ncols();
	mat<T> r(m1.nrows(),M,(T)0);
	for(unsigned i = 0; i < m1.nrows(); i++)
		for(unsigned j = 0; j <= i;j++)
			for(unsigned k = j; k <= i; k++)
				r(i,j) += m1(i,k) * (T)(m2(k,j)); 

	return r;
}



template <typename T, typename S>
const vec<T> operator*(const low_tri_mat<T>& m1, const vec<S>& v) 
{
	assert(m1.ncols() == v.size());
	unsigned M = v.size();
	vec<T> r(M,(T)0);
	for(unsigned i = 0; i < m1.nrows(); i++)
			for(unsigned k = 0; k <= i; k++)
				r(i) += m1(i,k) * (T)(v(k)); 

	return r;
}

template <typename T, typename S>
const mat<T> operator*(const mat<S>& m1, const low_tri_mat<T>& m2) 
{
	assert(m1.ncols() == m2.dim());
	unsigned M = m2.dim();
	mat<T> r(m1.nrows(),M,0);
	for(unsigned i = 0; i < m1.nrows(); i++)
		for(unsigned j = 0; j < M;j++)
			for(unsigned k = j; k < m1.ncols(); k++)
				r(i,j) += m1(i,k) * (T)(m2(k,j)); 

	return r;
}


//multiplies a permutation matrix from left to apply a rows permutation
template<typename T>
mat<T> operator*(const perm_mat& p, const low_tri_mat<T>& m)
{
	mat<T> r=m;
	return p*r;
}

//multiplies a permutation matrix from right to apply a rows permutation
template<typename T>
mat<T> operator*(const low_tri_mat<T>& m,const perm_mat& p)
{
	mat<T> r=m;
	return r*p;
}

}
}
