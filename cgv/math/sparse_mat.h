#pragma once
#include <cgv/math/vec.h>
#include <cgv/math/mat.h>

namespace cgv {
namespace math {



template <typename T>
class sparse_mat;

template <typename T>
void Ax(const sparse_mat<T>& A, const vec<T>&v, vec<T>& r);



/*
* A sparse matrix column compressed form.
*/
template <typename T>
class sparse_mat 
{
		
private:
	
	//number of rows
	unsigned _nrows;
	//number of columns
	unsigned _ncols; 

	//column start indices in compressed form 
	vec<unsigned> _cols;
	//row indices of data
	vec<unsigned> _rows;
	//values of matrix entries
	vec<T> _data;

public:
	sparse_mat(const mat<T>& m, T eps=0)
	{
		compress(m,eps);
	}

	//return number of rows
	unsigned nrows() const
	{
		return _nrows;
	}

	//returns number of columns
	unsigned ncols() const
	{
		return _ncols;
	}

	//return number of non zero elements
	unsigned num_non_zeros() const
	{
		return _data.size();
	}

	//cast conversion into full matrix
	operator mat<T>()
	{
		mat<T> m(_nrows,_ncols);
		m.zeros();
		for(unsigned j = 0; j < _ncols; j++)
			for(unsigned i = _cols(j);i < _cols(j+1); i++)
				m( _rows(i),j)=_data(i);
		return m;
	}


	//compress 
	void compress(const mat<T>& m, T eps=0)
	{
		_nrows = m.nrows();
		_ncols = m.ncols();
		_cols.resize(m.ncols()+1);

		unsigned nz = 0;
		for(unsigned i =0; i < m.nrows(); i++)
		{
			for(unsigned j =0; j < m.ncols(); j++)
			{
				if(m(i,j) > eps)
					nz++;
			}
		}

		_rows.resize(nz);
		_data.resize(nz);

		nz=0;
		for(unsigned j =0; j < m.ncols(); j++)
		{
		
			_cols[j]=nz;
			for(unsigned i =0; i < m.nrows(); i++)
			{
			
				if(m(i,j) > eps)
				{
					_data(nz) = m(i,j);
					_rows(nz) = i;
					nz++;	
				}
			}
		}
		_cols[m.ncols()]=nz;
		
	}

	
	///matrix vector product
	vec<T> operator*(const vec<T>& v)
	{
		assert(_ncols == v.size());
		vec<T> r;
		r.zeros(_nrows);
		unsigned c;
		for(c = 0; c < _ncols;c++)
			for(unsigned i = _cols(c); i < _cols(c+1);i++)
				r(_rows(i)) += _data(i)*v(c);

		
			
		return r;
	}


	//in place multiplication with scalar s
	sparse_mat<T>& operator*=(const T& s)
	{
		_data*=s;		
			
		return *this;
	}

	sparse_mat<T> operator*(const T& s)
	{
		sparse_mat<T> m = *this;
		m*=s;		
		return m;
	}

	sparse_mat<T>& operator/=(const T& s)
	{
		_data*=s;		
			
		return *this;
	}

	sparse_mat<T> operator/(const T& s)
	{
		sparse_mat<T> m = *this;
		m/=s;		
		return m;
	}


	///transpose matrix
	void transpose()
	{
		vec<unsigned> _colsnew(_nrows+1);
		_colsnew.zeros();
		vec<unsigned> _rowsnew(_data.size());
		vec<T>_datanew(_data.size());
		vec<unsigned>_colsnew2(_nrows+1);
		vec<unsigned> _rowsnew2(_data.size());
		
		for(unsigned c = 0; c < _ncols; c++)
			for(unsigned i = _cols[c]; i < _cols[c+1]; i++)
			{
				_colsnew(_rows(i))++;
				_rowsnew(i) = c;
			}

		unsigned sum = 0;
		for(unsigned i = 0; i < _colsnew.size();i++)
		{
			unsigned temp = _colsnew(i);
			_colsnew(i) = sum;
			_colsnew2(i) = sum;
			
			sum+=temp;

		}

		_datanew = _data;
		_cols= _colsnew;
	

		for(unsigned i = 0; i < _data.size();i++)
		{
			unsigned idx =_colsnew(_rows(i));

			_data(idx)= _datanew(i);
			_rowsnew2(idx)= _rowsnew(i);
			_colsnew(_rows(i))++;
		}

		_rows = _rowsnew2;
		
		unsigned t = _nrows;
		_nrows = _ncols;
		_ncols = t;
	
	}



	friend std::ostream& operator<< <T>(std::ostream& out, sparse_mat& sm);
	
	friend void Ax<T>(const sparse_mat<T>& A,const vec<T>&v, vec<T>& r);

	friend void Atx<T>(const sparse_mat<T>& A,const vec<T>&v, vec<T>& r);

	friend bool low_tri_solve(const sparse_mat<T>& L,const vec<T>& b, vec<T>& x);

	

	
	
};

template <typename T>
std::ostream& operator<<(std::ostream& out,sparse_mat<T>& sm)
{
		out << sm._nrows <<" "<< sm._ncols << std::endl;
		out << sm._cols << std::endl;
		out << sm._rows << std::endl;
		out << sm._data << std::endl;
		return out;
}

template <typename T>
void Ax(const sparse_mat<T>& A, const vec<T>&v, vec<T>& r)
{
	assert(A._ncols == v.size());
	r.zeros(A._nrows);
	unsigned c;
	for(c = 0; c < A._ncols;c++)
		for(unsigned i = A._cols(c); i < A._cols(c+1);i++)
			r(A._rows(i)) += A._data(i)*v(c);

	
};


template <typename T>
void Atx(const sparse_mat<T>& A, const vec<T>&v, vec<T>& r)
{
	assert(A._nrows == v.size());
	r.zeros(A._ncols);
	unsigned c;
	for(c = 0; c < A._ncols;c++)
		for(unsigned i = A._cols(c); i < A._cols(c+1);i++)
			r(c) += A._data(i)*v(A._rows(i));
	
}

template <typename T>
sparse_mat<T> transpose(const sparse_mat<T>& sm)
{
	sparse_mat<T> m = sm;
	
	m.transpose();
	return m;
}


template <typename T>
sparse_mat<T> operator*(const T& s,const sparse_mat<T>& m)
{		
	return m*s;
}

template <typename T>
bool low_tri_solve(const sparse_mat<T>& L,const vec<T>& b, vec<T>& x)
{
	x=b;
	for(unsigned j = 0; j < x.size(); j++)
	{
		//not lower triangular or singular
		if(L._data(L.rows(L._cols(j))) != j )
			return false;

		x(j) =x(j) / L._data(_cols(j));
		for(unsigned i= L._cols(j)+1; i < L._cols(j+1);i++)
			x(i) = x(i) - L._data(i)*x(j);
		
	}
	return true;
}





}
}

