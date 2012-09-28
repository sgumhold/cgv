#pragma once
#include <cgv/math/vec.h>
#include <cgv/math/mat.h>

namespace cgv {
	namespace math {


/**
* tridiagonal matrix (square)
*/
template<typename T>
class tri_diag_mat
{	
protected:
	vec<T> _data;

	unsigned _dim;

public:
	///standard constructor
	tri_diag_mat()
	{
		_dim=0;
	}

	tri_diag_mat(unsigned n)
	{
		resize(n);
	}


	tri_diag_mat(unsigned n,const T& c)
	{
		resize(n);
		fill(c);
	}

	///copy constructor
	tri_diag_mat(const tri_diag_mat<T>& m)	
	{
		_data = m._data;
		_dim = m._dim;
	}

	///assignment of a matrix with the same element type
	tri_diag_mat<T>& operator = (const tri_diag_mat<T>& m) 
	{
		_data = m._data;
		_dim  = m._dim;
		return *this;
	}

	///cast into const full storage matrix
	operator const mat<T>() const
	{
		mat<T> m;
		m.zeros(_dim,_dim);

		for(int b = -1;b <= 1;b++)
			for(unsigned i =0; i < _dim;i++)
			{
				if(b== -1 && i==0)
					continue;
				if(b== 1 && i==_dim-1)
					continue;

				m(i,b+i)=operator()(i,b);
			}
		
		return m;
	}

	///set identity matrix
	void identity()
	{
		
		for(unsigned i = 0; i < 3*_dim; i++)
		{
			if( i < _dim || i > 2*_dim)
				_data(i)=0;
			else
				_data(i)=1;
		}

	}

	///cast into array of element type
	operator T*()
	{
		return (T*)_data;
	}

	///cast into array of const element type
	operator const T*() const
	{
		return (const T*)_data;
	}

	void fill(const T& c)
	{
		_data.fill(c);
		_data(0) = 0;		
		_data(3*_dim-1) =0;
	}

	///returns true 
	bool is_square()
	{
		return true;
	}

	//transpose
	void transpose()
	{

	}

	void zeros()
	{
		_data.zeros(); 
	}

	void zeros(unsigned n)
	{
		resize(n);
		zeros();
	}


	void resize(unsigned n)
	{
		_dim=n;
		_data.resize(3*n);
		_data(0) = 0;		
		_data(3*n-1) =0;
	}

	///return number of rows 
	unsigned nrows()
	{
		return _dim;
	}
		
	///return number of columns
	unsigned ncols()
	{
		return _dim;
	}

	///return storage size in elements
	unsigned size()
	{
		return 3*_dim;
	}

	///return matrix element in row r of band b
	T& operator()(unsigned r,int  b)
	{
		assert(b >= -1 && b <=1);	
		return _data[ (b+1)*_dim + r];
	} 

	///return matrix element in row r of band b
	const T& operator()(unsigned r,int  b) const
	{
		assert(b >= -1 && b <=1);	
		return _data[ (b+1)*_dim + r];
	} 

	
	vec<T> band(int b) const
	{	
		return _data.sub_vec((b+1)*_dim,_dim);
	}
	

	vec<T> operator*(const vec<T>& v) const
	{
		vec<T> r;
		r.zeros(v.dim());
				
		for(unsigned i = 0; i < v.dim(); i++)
			for(int b = -1; b <= 1;b++)
			{
				if((i+b >= 0) && (i + b<  v.dim()))
					r(i) += operator()(i,b)*v(i+b);
			}

		return r;
	}



};

	}
}

