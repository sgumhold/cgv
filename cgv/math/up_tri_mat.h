#pragma once

#include "vec.h"
#include "mat.h"
#include "low_tri_mat.h"

namespace cgv {
	namespace math {

template <typename T>
class up_tri_mat 
{

protected:
	vec<T> _data;
	
	unsigned _dim;
	

public:
	
	//standard constructor of a upper triangular matrix
	up_tri_mat()
	{
		_dim=0;
	}

	//constructs a upper triangular matrix from the upper triangular part of m
	up_tri_mat(const mat<T>& m)
	{
		assert(m.ncols()==m.nrows());
		resize(m.ncols());

		for(unsigned i =0; i < _dim;i++)
		{
			for(unsigned j = i; j < _dim;j++)
			{
				operator()(i,j)=m(i,j);
			}	
		}	
	}

	//creates a dim x dim upper triangular matrix
	up_tri_mat(unsigned dim)
	{
		resize(dim);
		fill(0);
	}

	up_tri_mat(const up_tri_mat& m)
	{
		resize(m.dim());
		memcpy(_data,m._data,size()*sizeof(T));
	}

	//create a dim x dim upper triangular matrix with all non-zero elements set to c
	up_tri_mat(unsigned dim, const T& c)
	{
		resize(dim);
		fill(c);
	}

	virtual ~up_tri_mat()
	{
	}

	//resize the matrix to an n x n matrix
	void resize(unsigned n)
	{
		_dim = n;
		_data.resize(n*(n+1)/2);
			
	}

	operator mat<T>() 
	{
		mat<T> m(_dim,_dim);
		for(unsigned i =0; i < _dim; i++)
		{
			for(unsigned j = 0; j < _dim; j++)
			{
				if(i>j)
					m(i,j)=0;
				else
					m(i,j)=operator()(i,j);
		}
			
		}
		return m;
	}

	operator const mat<T>() const
	{
		mat<T> m(_dim,_dim);
		for(unsigned i =0; i < _dim;i++)
		{
			for(unsigned j = 0; j < _dim;j++)
			{
				if(i>j)
					m(i,j)=0;
				else
					m(i,j)=operator()(i,j);
		}
			
		}
		return m;
	}

	///assignment of a matrix with the same element type
	up_tri_mat<T>& operator = (const up_tri_mat<T>& m) 
	{  
		_dim = m.dim();
		_data = m._data;
		return *this;
	}

	//return number of stored elements
	unsigned size() const
	{
		return _data.size();
	}

	//return number of stored elements
	unsigned dim() const
	{
		return _dim;
	}

	//returns the number of columns 
	unsigned ncols() const
	{
		return _dim;
	}

	//returns the number of columns 
	unsigned nrows() const
	{
		return _dim;
	}

	//fills all upper triangular elements with c
	void fill(const T& c)
	{
		for(unsigned i=0;i < size();i++)
		{
			_data[i]=c;
		}
	}

	//access to the element (i,j)
	T& operator() (unsigned i, unsigned j) 
	{
		assert( j >= i && j < _dim);
		return _data[j*(j+1)/2+i]; 
	}
	
	//const access to the element (i,j)
	const T& operator() (unsigned i, unsigned j) const 
	{
		assert( j >= i && j < _dim);
		return _data[j*(j+1)/2+i]; 
	}

	///in place  division by a scalar
	up_tri_mat<T>& operator /= (const T& s)			
	{ 
		T val = (T)s;
		for(unsigned i = 0; i < size(); i++)
			
				_data[i] /= val; 
		return *this; 
	}

	/// division by a scalar
	up_tri_mat<T> operator / (const T& s)			
	{ 
		up_tri_mat<T> r=(*this);
		r/=s;
		return r;
	}

	
	const mat<T> operator*( const up_tri_mat<T>& m2) 
	{
		assert(m2.nrows() == nrows());
		unsigned M = m2.ncols();
		mat<T> r(nrows(),M,(T)0);
		for(unsigned i = 0; i < nrows(); i++)
			for(unsigned j = i; j < M;j++)
				for(unsigned k = i; k <= j; k++)
					r(i,j) += operator()(i,k) * (T)(m2(k,j)); 

		return r;
	}

	
	const mat<T> operator*( const mat<T>& m2) 
	{
		assert(m2.nrows() == nrows());
		unsigned M = m2.ncols();
		mat<T> r(nrows(),M,(T)0);
		for(unsigned i = 0; i < nrows(); i++)
			for(unsigned j = 0; j < M;j++)
				for(unsigned k = i; k < nrows(); k++)
					r(i,j) += operator()(i,k) * (T)(m2(k,j)); 

		return r;
	}


};

//transpose of a matrix m
template <typename T>
const low_tri_mat<T> transpose(const up_tri_mat<T> &m)
{
	low_tri_mat<T> r(m.nrows());
	for(unsigned j = 0; j < m.ncols();j++)
		for(unsigned i = j; i < m.nrows();i++)
			r(i,j) = m(j,i);
		

	
	return r;
}

//transpose of a matrix m
template <typename T>
const up_tri_mat<T> transpose(const low_tri_mat<T> &m)
{
	up_tri_mat<T> r(m.nrows());
	for(unsigned j = 0; j < m.ncols();j++)
		for(unsigned i = j; i < m.nrows();i++)
			r(j,i) = m(i,j);
		

	
	return r;
}






template <typename T, typename S>
const vec<T> operator*(const up_tri_mat<T>& m1, const vec<S>& v) 
{
	assert(m1.ncols() == v.size());
	unsigned M = v.size();
	vec<T> r(M,(T)0);
	for(unsigned i = 0; i < m1.nrows(); i++)
			for(unsigned k = i; k < m1.ncols(); k++)
				r(i) += m1(i,k) * (T)(v(k)); 

	return r;
}


template <typename T, typename S>
const mat<T> operator*(const mat<S>& m1, const up_tri_mat<T>& m2) 
{
	assert(m1.ncols() == m2.nrows());
	unsigned M = m2.nrows();
	mat<T> r(m1.nrows(),M,(T)0);
	for(unsigned i = 0; i < m1.nrows(); i++)
		for(unsigned j = 0; j < M;j++)
			for(unsigned k = 0; k <= j; k++)
				r(i,j) += m1(i,k) * (T)(m2(k,j)); 

	return r;
}

//product of a lower and an upper triangular matrix
template <typename T>
const mat<T> operator*(const low_tri_mat<T>& m1, const up_tri_mat<T>& m2) 
{
	assert(m1.nrows() == m2.nrows());
	
	mat<T> r(m1.nrows(),m2.nrows(),(T)0);
	for(unsigned i = 0; i < m1.nrows(); i++)
		for(unsigned j = 0; j < m1.nrows();j++)
		{
			unsigned h = std::min(i,j);
			for(unsigned k = 0; k <= h; k++)
				r(i,j) += m1(i,k) * (T)(m2(k,j)); 
		}

	return r;
}


//multiplies a permutation matrix from left to apply a rows permutation
template<typename T>
mat<T> operator*(const perm_mat& p, const up_tri_mat<T>& m)
{
	mat<T> r=m;
	return p*r;
}

//multiplies a permutation matrix from right to apply a rows permutation
template<typename T>
mat<T> operator*(const up_tri_mat<T>& m,const perm_mat& p)
{
	mat<T> r=m;
	return r*p;
}


///output of a upper triangular matrix onto an ostream
template <typename T>
std::ostream& operator<<(std::ostream& out, const up_tri_mat<T>& m)
{
	
	for(unsigned i =0;i< m.nrows() ;++i)
	{
		unsigned j = 0;
		for (; j<m.ncols(); ++j)
			if(j <i)
				out << "\t";
			else
				out << m(i,j) <<"\t";
		if(i < m.nrows()-1)
			out <<"\n";		
	}

	return out;

}

}

}
