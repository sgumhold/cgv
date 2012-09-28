#pragma once

#include "mat.h"
#include "vec.h"


namespace cgv {
	namespace math {
/**
* a permutation matrix type
*/
struct perm_mat
{
	///internal storage of the permutation
	vec<unsigned> _data;
	
	///standard constructor
	perm_mat(){}
	
	///create an nxn identity permutation matrix (storage is only n)
	explicit perm_mat(unsigned n)
	{
		resize(n);
		identity();
	}

	///creates a permutation from a given permutation vector
	perm_mat(vec<unsigned> p)
	{
		_data = p;
	}

	///number of stored elements
	unsigned size() const 
	{
		return _data.size();
	}

	///number of rows
	unsigned nrows() const
	{
		return _data.size();
	}

	///number of rows
	unsigned ncols() const
	{
		return _data.size();
	}




	///resize the permutation matrix
	void resize(unsigned n)
	{
		_data.resize(n);
		for(unsigned i = 0; i < n; i++)
			_data[i] = i;

	}

	///compute the transpose permutation matrix (equal to inverse permutation)
	void transpose()
	{
		math::vec<unsigned> temp(_data.size());
		for(unsigned i = 0; i < _data.size(); i++)
			temp[_data[i]] = i;
		_data=temp;
	}

	template <typename T>
	operator const mat<T>() const
	{
		mat<T> m;
		m.zeros(size(),size());

		for(unsigned i =0; i < size();i++)
			m(i,_data[i])=(T)1.0;
		
		return m;
	}

	///product with another permutation matrix
	perm_mat operator*(const perm_mat& m)
	{
		assert(size() == m.size());
		perm_mat r(size());
		
		for(unsigned i = 0; i < size();i++)
		{
			r(i) = m(operator()(i));
		}
		
		return r;
	}

	


	///set to identity matrix
	void identity()
	{
		for(unsigned i = 0; i < _data.size(); i++)
			_data[i] = i;
	}

	//add permutation of the entries i and j
	void swap(unsigned i, unsigned j)
	{
		
		std::swap(_data[i],_data[j]);
	}

	unsigned& operator()(unsigned i) 
	{
		return _data[i];
	}


	unsigned& operator[](unsigned i) 
	{
		return _data[i];
	}
	
	
	unsigned operator()(unsigned i) const
	{
		return _data[i];
	}


	unsigned operator[](unsigned i) const
	{
		return _data[i];
	}
	
	

};


//multiplies a permutation matrix from left to apply a rows permutation
template<typename T>
mat<T> operator*(const perm_mat& p, const mat<T>& m)
{
	mat<T> r(m.nrows(),m.ncols());

	for(unsigned i = 0; i < m.nrows(); i++)
	{
		for(unsigned j = 0; j < m.ncols(); j++)
			r(p(i),j)=m(i,j);
	}
	return r;
}




//multiplies a permutation matrix from right to perform a column permutation
template<typename T>
mat<T> operator*(const mat<T>& m, const perm_mat& p)
{
	mat<T> r(m.nrows(),m.ncols());

	for(unsigned i = 0; i < m.nrows(); i++)
	{
		for(unsigned j = 0; j < m.ncols(); j++)
			r(i,p(j)) = m(i,j);
	}
	return r;
}





//multiplies a permutation matrix from left to perform an element permutation on vector v
template<typename T>
vec<T> operator*(const perm_mat& p, const vec<T>& v)
{
	vec<T> r(v.size());
	for(unsigned i = 0; i < v.size(); i++)
	{
	
		r(p[i]) = v(i);
		
	}
	return r;
}


inline perm_mat transpose(const perm_mat &p)
{
	perm_mat r=p;
	r.transpose();
	return r;
}




inline std::ostream& operator<<(std::ostream& out,const  perm_mat& m)
{
	
	for (int i=0;i<(int)m.size();++i)
		out << i << "->"<< m(i)<<"\n";

	return out;
}




}

}