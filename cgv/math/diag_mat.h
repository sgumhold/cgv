#pragma once

#include "vec.h"
#include "mat.h"
#include "up_tri_mat.h"

namespace cgv{
	namespace math{

/**
 * A diagonal matrix type which internally stores the 
 * elements on the main diagonal in a vector.
 **/
template <typename T>
struct diag_mat 
{

public:
	///pointer to data storage
	vec<T> _data;
public:
	typedef typename vec<T>::value_type value_type;
	typedef typename vec<T>::reference reference;
	typedef typename vec<T>::const_reference const_reference;
	typedef typename vec<T>::pointer pointer;
	typedef typename vec<T>::const_pointer const_pointer;

	typedef typename vec<T>::iterator iterator;
	typedef typename vec<T>::const_iterator const_iterator;
	typedef typename vec<T>::reverse_iterator reverse_iterator;
	typedef typename vec<T>::const_reverse_iterator const_reverse_iterator;

	typedef iterator  diag_iterator;
	typedef const diag_iterator const_diag_iterator;
	typedef std::reverse_iterator<diag_iterator> reverse_diag_iterator;
	typedef std::reverse_iterator<const_diag_iterator> const_reverse_diag_iterator;
	
	iterator begin(){return _data.begin();}
	iterator end(){return _data.end();}
	const_iterator begin() const{return _data.begin();}
	const_iterator end() const{return _data.end();}
	reverse_iterator rbegin(){return _data.rbegin();}
	reverse_iterator rend(){return _data.rend();}
	const_reverse_iterator rbegin() const{return _data.rbegin();}
	const_reverse_iterator rend() const {return _data.rend();}

	diag_iterator diag_begin(){return _data.begin();}
	diag_iterator diag_end(){return _data.end();}
	const_diag_iterator diag_begin() const{return _data.begin();}
	const_diag_iterator diag_end() const{return _data.end();}
	reverse_diag_iterator diag_rbegin(){return _data.rbegin();}
	reverse_diag_iterator diag_rend(){return _data.rend();}
	const_reverse_diag_iterator diag_rbegin() const{return _data.rbegin();}
	const_reverse_diag_iterator diag_rend() const {return _data.rend();}

	///size of storage
	unsigned size() const 
	{
		return _data.size();
	}

	///number of rows
	unsigned nrows() const
	{
		return size();
	}

	///number of columns
	unsigned ncols() const
	{
		return size();
	}

	///standard constructor
	diag_mat()
	{
	}

	///creates nxn diagonal matrix
	explicit diag_mat(unsigned n):_data(n)
	{
	}

	///creates nxn diagonal matrix and set all diagonal elements to c
	diag_mat(unsigned n, const T& c)
	{
		resize(n);
		fill(c);
	}

	//creates nxn diagonal matrix from array containing the diagonal elements
	diag_mat(unsigned n, const T* marray):_data(n,marray)
	{
	}

	///creates a diagonal matrix and set the diagonal vector to dv
	diag_mat(const vec<T>& dv):_data(dv)
	{
	}

	///creates a diagonal matrix and set the diagonal vector to diagonal entries of m
	diag_mat(const mat<T>& m)
	{
		int n = std::min(m.ncols(),m.nrows());
		resize(n);
		for(int i =0; i < n; i++)
			_data[i]=m(i,i);
	}

	///copy constructor
	diag_mat(const diag_mat<T>& m)
	{
		_data=m._data;
	}

	

	///destructor
	virtual ~diag_mat()
	{
	}

	///cast into array of element type
	T* data()
	{
		return _data.data();
	}

	///cast into array of const element type
	const T* data() const
	{
		return _data.data();
	}

	///create sub diagonal matrix d(top_left)...d(top_left+size)
	diag_mat<T> sub_mat(unsigned top_left, unsigned size) const
	{
		diag_mat<T> D(size);
		for (unsigned int i=0; i<size; ++i)
			D(i) = (*this)(i+top_left);
		return D;
	}
	///cast into const full storage matrix
	operator const mat<T>() const
	{
		mat<T> m;
		m.zeros(size(),size());

		for(unsigned i =0; i < size();i++)
					m(i,i)=operator()(i);
		
		return m;
	}
	

	//resize the diagonal matrix to nxn
	void resize(unsigned n)
	{
		_data.resize(n);
	}

	///set diagonal matrix to identity
	void identity()
	{
		fill((T)1);
	}

	///fills all diagonal entries with c
	void fill(const T& c)
	{
		for(unsigned i=0;i < size();i++)
		{
			_data[i]=c;
		}
	}
	///fills all diagonal entries with zero
	void zeros()
	{
		fill((T)0);
	}

	///exchange diagonal elements i and j
	void exchange_diagonal_elements(unsigned i, unsigned j)
	{
		std::swap(_data[i],_data[j]);
	}

	///const access to the ith diagonal element
	const T& operator() (unsigned i) const 
	{
		return _data[i];
	}

	///non const access to the ith diagonal element
	T& operator() (unsigned i) 
	{
		return _data[i];
	}

	///const access to the ith diagonal element
	const T& operator[] (unsigned i) const 
	{
		return _data[i];
	}

	///non const access to the ith diagonal element
	T& operator[] (unsigned i) 
	{
		return _data[i];
	}


	///returns true because diagonal matrices are always square
	bool is_square()
	{
		return true;
	}

	//transpose (does nothing)
	void transpose(){}


	///assignment of a matrix with the same element type
	diag_mat<T>& operator = (const diag_mat<T>& m) 
	{
		_data  =m._data;
		return *this;
	}

	///assignment of a matrix with a different element type
	template <typename S> 
	diag_mat<T>& operator = (const diag_mat<S>& m) 
	{  
		resize(m.size());
		for(unsigned i = 0; i < size(); i++)
				_data[i]=(T)m(i); 
		return *this;
	}

	///assignment of a vector with a vector
	///to set the diagonal
	template <typename S> 
	diag_mat<T>& operator = (const vec<S>& v) 
	{  
		_data=v; 
		return *this;
	}

	///assignment of a scalar s to each element of the matrix
	diag_mat<T>& operator  = (const T& s) 
	{ 
		fill (s);
		return *this; 
	}
	///returns the frobenius norm of matrix m
	T frobenius_norm() const
	{
		T n=0;
		for(int i =0; i < size();i++)
			n+=_data[i]*_data[i];
	
		return (T)sqrt((double)n);
	}


	
	
	///set dim x dim identity matrix 
	void identity(unsigned dim)
	{
		resize(dim);
		for(unsigned i = 0; i < size();++i)
			_data[i]=1;
	}

	///set zero matrix
	void zero()
	{
		fill(0);
	}

	///in place addition of diagonal matrix
	diag_mat<T>& operator+=(const diag_mat<T>& d) 
	{
		assert(d.nrows() == nrows());
		_data += d._data;
		return *this;
	}

	///in place subtraction of diagonal matrix
	diag_mat<T>& operator-=(const diag_mat<T>& d)
	{
		assert(d.nrows() == nrows());
		_data -= d._data;
		return *this;
	}

	///in place multiplication with scalar s
	diag_mat<T>& operator*=(const T& s)
	{
		_data *=s;
		return *this;
	}

	///multiplication with scalar s
	diag_mat<T> operator*(const T& s) const
	{
		diag_mat<T> r=*this;
		r*=s;
		return r;
	}

	///addition of diagonal matrix
	diag_mat<T> operator+(const diag_mat<T>& d) const
	{
		diag_mat<T> r=*this;
		r += d;
		return r;
	}

	///subtraction of diagonal matrix
	diag_mat<T> operator-(const diag_mat<T>& d) const
	{
		diag_mat<T>r = *this;
		r-= d;
		return r;
	}

	

	///multiplication with vector
	vec<T> operator*(const vec<T>& v) const
	{
		assert(v.dim() == ncols());
		vec<T> r(size());
		for(unsigned i = 0; i < size();i++)
			r(i) = _data[i]*v(i);
		return r;
	}

	///matrix multiplication  of matrix m by a diagonal matrix s
	const up_tri_mat<T> operator*( const up_tri_mat<T>& m) 
	{		
		assert(m.nrows() == size());

		up_tri_mat<T> r(size());

		for(unsigned i = 0; i < size(); i++)
			for(unsigned j = 0; j < m.ncols();j++)
				r(i,j) = operator()(i)*m(i,j);

		return r;
	}

	///cast into full matrix type
	operator mat<T>()
	{
		mat<T> m(size(),size(),(T)0);
		for(unsigned i = 0; i < size();i++)
		{
			m(i,i)=_data[i];
		}
		return m;
	}

};

//matrix multiplication  of matrix m by a diagonal matrix s
template <typename T, typename S>
const mat<T> operator*(const mat<T>& m,const diag_mat<S>& s) 
{		
	assert(m.ncols() == s.size());
	mat<T> r(m.nrows(),s.size());
	for(unsigned i = 0; i < m.nrows(); i++)
		for(unsigned j = 0; j < s.size();j++)
			r(i,j) = s(j)*m(i,j);

	return r;
}

///multiplication of scalar s and diagonal matrix m
template <typename T>
const diag_mat<T> operator*(const T& s, const diag_mat<T>& m)
{
	return m*s;
}

///matrix multiplication  of diagonal matrix s and m
template <typename T, typename S>
const diag_mat<T> operator*(const diag_mat<S>& s, const diag_mat<T>& m) 
{		
	assert(m.size() == s.size());

	diag_mat<T> r(s.size());

	for(unsigned i = 0; i < s.size(); i++)
			r(i) = s(i)*m(i);

	return r;
}
///multiplies a permutation matrix from left to apply a rows permutation
template<typename T>
mat<T> operator*(const perm_mat& p, const diag_mat<T>& m)
{
	mat<T> r=m;
	return p*r;
}

///multiplies a permutation matrix from right to apply a rows permutation
template<typename T>
mat<T> operator*(const diag_mat<T>& m,const perm_mat& p)
{
	mat<T> r=m;
	return r*p;
}

///matrix multiplication  of matrix m by a diagonal matrix s
template <typename T>
const mat<T> operator*(const diag_mat<T>& s, const mat<T>& m) 
{		
	assert(m.nrows() == s.size());

	mat<T> r(s.size(),m.ncols());

	for(unsigned i = 0; i < s.size(); i++)
		for(unsigned j = 0; j < m.ncols();j++)
			r(i,j) = s(i)*m(i,j);

	return r;
}

///matrix multiplication  of upper triangular matrix m by a diagonal matrix s
template <typename T>
const up_tri_mat<T> operator*(const up_tri_mat<T>& m,const diag_mat<T>& s) 
{		
	assert(m.ncols() == s.size());
	up_tri_mat<T> r(s.size());
	for(unsigned i = 0; i < m.nrows(); i++)
		for(unsigned j = i; j < s.size();j++)
			r(i,j) = s(j)*m(i,j);

	return r;
}





///output of a diagonal matrix onto an ostream
template <typename T>
std::ostream& operator<<(std::ostream& out, const diag_mat<T>& m)
{
	
	for (int i=0;i<(int)m.size();++i)
		out << m(i)<<" ";

	return out;
}

///input of a diagonal matrix from an istream
template <typename T>
std::istream& operator>>(std::istream& in,  diag_mat<T>& m)
{
	assert(m.size() > 0);
	for (int i=0;i<(int)m.size();++i)
		in >> m(i);

	return in;
}


}
}
