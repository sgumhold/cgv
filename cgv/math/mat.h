#pragma	once

#include "vec.h"
#include <limits> 
#include <cassert>

namespace cgv {
namespace math {



template <typename T>
class mat;




template <typename RandomAccessIterator>
struct step_iterator
{
public:
	typedef RandomAccessIterator base_type;
	typedef typename base_type::pointer pointer;
	typedef typename base_type::reference reference;
	typedef typename base_type::value_type value_type;
	typedef typename base_type::difference_type difference_type;
	typedef typename base_type::iterator_category iterator_category;
private:
	RandomAccessIterator internal_iter;
	int step;


	friend class mat<value_type>;

	step_iterator( RandomAccessIterator begin,int step=1):internal_iter(begin),step(step) 
	{}
	
	
public:
	
	
	step_iterator():internal_iter(NULL)
	{
		step=0;
	}
    
	step_iterator(const step_iterator& other)
	{
		internal_iter=other.internal_iter;
		step=other.step;
	}

    step_iterator& operator=(const step_iterator& other)
	{
		if(*this != other)
		{
			internal_iter = other.internal_iter;
			step = other.step;
		}
		return *this;
	}

	bool operator==(const step_iterator& other) const
	{
		return internal_iter == other.internal_iter;
	}
     
	bool operator!=(const step_iterator& other) const
	{
		return !(*this==other);
	}

	reference operator*()
	{
		return *internal_iter;
	}
      
	reference operator*() const
	{
		return *internal_iter;
	}
	pointer operator->()
	 {
		  return &**this;
	 }
      
	pointer operator->() const
	 {
		  return &**this;
	 }

      step_iterator& operator ++()
	  {
		  
		  internal_iter+=step;
		  return *this;
	  }

      step_iterator operator ++(int)
	  {
		  step_iterator tmp=*this;
	      ++*this;
		  return tmp;

	  }
	  step_iterator& operator --()
	  { 
		  internal_iter-=step;
		  return *this;
	  }
      step_iterator operator --(int)
	  {
		  step_iterator tmp=*this;
	      --*this;
		  return tmp;
	  }

	 
     
	  step_iterator& operator +=(difference_type n)
	  {
		  internal_iter+=n*step;
		  return *this;
		 
	  }

	  step_iterator& operator -=(difference_type n)
	  {
		  internal_iter-=n*step;
		  return *this;
	  }

	  step_iterator operator -(difference_type n) const
	  {
		  step_iterator tmp=*this;
	      tmp-=n;
		  return tmp;
	  }
	  difference_type operator-(const step_iterator& right) const
		{	
		return (internal_iter - right.internal_iter)/step;
		}
	 

	  step_iterator operator +(difference_type n)const
	  {
		  step_iterator tmp=*this;
	      tmp+=n;
		  return tmp;
	  }

	  reference operator[](difference_type offset) const
	  {
		return (*(*this + offset));
	  }

	  bool operator <(const step_iterator& other) const
	  {
		  if(step > 0)
			return internal_iter < other.internal_iter;
		  else
			return internal_iter > other.internal_iter;
	  }
	  
	  bool operator >(const step_iterator& other) const
	  {
		   if(step > 0)
			   return internal_iter > other.internal_iter;
		   else
			   return internal_iter < other.internal_iter;
	  }

	  bool operator <=(const step_iterator& other) const
	  {
		   if(step > 0)
			   return internal_iter <= other.internal_iter;
		   else
			   return internal_iter >= other.internal_iter;
	  }
	  
	  bool operator >=(const step_iterator& other) const
	  {
		  if(step > 0)
			  return internal_iter >= other.internal_iter;
		  else
			  return internal_iter <= other.internal_iter;
	  }
	 
   
};











/**
* A matrix type (full column major storage) 
* The matrix can be loaded directly into OpenGL without need for transposing!
*/
template <typename T>
class mat 
{
protected:
	///pointer to data storage
	vec<T> _data;
	///number of columns
	unsigned _ncols;
	///number of rows
	unsigned _nrows;


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

	typedef iterator col_iterator;
	typedef const col_iterator const_col_iterator;
	typedef std::reverse_iterator<col_iterator> reverse_col_iterator;
	typedef std::reverse_iterator<const_col_iterator> const_reverse_col_iterator;

	typedef step_iterator<iterator> row_iterator;
	typedef const row_iterator const_row_iterator;
	typedef std::reverse_iterator<row_iterator> reverse_row_iterator;
	typedef std::reverse_iterator<const_row_iterator> const_reverse_row_iterator;

	typedef step_iterator<iterator > diag_iterator;
	typedef const diag_iterator const_diag_iterator;
	typedef std::reverse_iterator<diag_iterator> reverse_diag_iterator;
	typedef std::reverse_iterator<const_diag_iterator> const_reverse_diag_iterator;

	typedef step_iterator<iterator > anti_diag_iterator;
	typedef const anti_diag_iterator const_anti_diag_iterator;
	typedef std::reverse_iterator<anti_diag_iterator> reverse_anti_diag_iterator;
	typedef std::reverse_iterator<const_anti_diag_iterator> const_reverse_anti_diag_iterator;

	



	iterator begin(){return _data.begin();}
	iterator end(){return _data.end();}
	const_iterator begin() const{return _data.begin();}
	const_iterator end() const{return _data.end();}
	reverse_iterator rbegin(){return _data.rbegin();}
	reverse_iterator rend(){return _data.rend();}
	const_reverse_iterator rbegin() const{return _data.rbegin();}
	const_reverse_iterator rend() const {return _data.rend();}


	
	col_iterator col_begin(int c)
	{
		return col_iterator(_data.begin()+(c*_nrows));
	}
	col_iterator col_end(int c)
	{
		return col_iterator(_data.begin()+(c+1)*_nrows );
	}
	const_col_iterator col_begin(int c) const
	{
		return const_col_iterator(_data.begin()+(c*_nrows));
	}
	const_col_iterator col_end(int c) const
	{
		return const_col_iterator(_data.begin()+(c+1)*_nrows );
	}
	reverse_col_iterator col_rbegin(int c)
	{	
		return (reverse_col_iterator(col_end(c)));
	}
	reverse_col_iterator col_rend(int c)
	{
		return (reverse_col_iterator(col_begin(c)));
	}
	const_reverse_col_iterator col_rbegin(int c) const
	{	
		return (const_reverse_col_iterator(col_end(c)));
	}
	const_reverse_col_iterator col_rend(int c) const
	{	
		return (const_reverse_col_iterator(col_begin(c)));
	}

	row_iterator row_begin(int r)
	{
		return row_iterator(_data.begin()+r,_nrows);
	}
	row_iterator row_end(int r)
	{
		return row_iterator(_data.begin()+(r+_ncols*_nrows),_nrows);
	}
	const_row_iterator row_begin(int r) const
	{
		return const_row_iterator(_data.begin()+r,_nrows);
	}
	const_row_iterator row_end(int r) const
	{
		return const_row_iterator(_data.begin()+(r+_ncols*_nrows),_nrows);
	}

	reverse_row_iterator row_rbegin(int r)
	{	
		return (reverse_row_iterator(row_end(r)));
	}

	reverse_row_iterator row_rend(int r)
	{
		return (reverse_row_iterator(row_begin(r)));
	}

	const_reverse_row_iterator row_rbegin(int r) const
	{	
		return (const_reverse_row_iterator(row_end(r)));
	}

	const_reverse_row_iterator row_rend(int r) const
	{	
		return (const_reverse_row_iterator(row_begin(r)));
	}



	
	diag_iterator diag_begin(int d=0)
	{
		if(d <= 0)
			return diag_iterator(_data.begin()-d ,_nrows+1);
		else
			return diag_iterator(_data.begin()+d*_nrows,_nrows+1);
		
	}
	diag_iterator diag_end(int d=0)
	{	
		if(d <= 0)
		{
			int n=std::min<int>(_nrows+d,_ncols); 
			return diag_iterator(_data.begin()-d+n*(_nrows+1),_nrows+1);
		}
		else
		{
			int n = std::min<int>(_ncols-d,_nrows);
			return diag_iterator(_data.begin()+d*_nrows+n*(_nrows+1),_nrows+1) ;
		}
	}
	const_diag_iterator diag_begin(int d=0) const
	{
		if(d <= 0)
			return const_diag_iterator(begin()-d ,_nrows+1);
		else	
			return const_diag_iterator(begin()+d*_nrows,_nrows+1);
		
	}
	const_diag_iterator diag_end(int d=0) const
	{
		if(d <= 0)
		{
			int n=std::min<int>(_nrows+d,_ncols); 
			return const_diag_iterator(_data.begin()-d+n*(_nrows+1),_nrows+1);
		}
		else
		{
			int n = std::min<int>(_ncols-d,_nrows);
			return const_diag_iterator(_data.begin()+d*_nrows+n*(_nrows+1),_nrows+1) ;
		}
	}
	reverse_diag_iterator diag_rbegin(int d=0)
	{	
		return (reverse_diag_iterator(diag_end(d)));
	}
	reverse_diag_iterator diag_rend(int d=0)
	{
		return (reverse_diag_iterator(diag_begin(d)));
	}
	const_reverse_diag_iterator diag_rbegin(int d=0) const
	{	
		return (const_reverse_diag_iterator(diag_end(d)));
	}
	const_reverse_diag_iterator diag_rend(int d=0) const
	{	
		return (const_reverse_diag_iterator(diag_begin(d)));
	}

	anti_diag_iterator anti_diag_begin(int d=0)
	{
		if(d >= 0)
			return anti_diag_iterator(_data.begin()+(_ncols-1-d)*_nrows ,1-_nrows);
		else
			return anti_diag_iterator(_data.begin()+(_ncols-1)*_nrows-d ,1-_nrows);
	}

	anti_diag_iterator anti_diag_end(int d=0)
	{
		if(d >= 0)
		{
			int n=std::min(_ncols-d,_nrows);
			return anti_diag_iterator(_data.begin()+(_ncols-1-d)*_nrows +n*(1-_nrows),1-_nrows);
		}
		else
		{
			int n=std::min(_nrows+d,_ncols);
			return anti_diag_iterator(_data.begin()+(_ncols-1)*_nrows-d +n*(1-_nrows),1-_nrows);
		}
	}

	const_anti_diag_iterator anti_diag_begin(int d=0) const
	{
		if(d >= 0)
			return const_anti_diag_iterator(_data.begin()+(_ncols-1-d)*_nrows ,1-_nrows);
		else
			return const_anti_diag_iterator(_data.begin()+(_ncols-1)*_nrows-d ,1-_nrows);
	}

	const_anti_diag_iterator anti_diag_end(int d=0) const
	{
		if(d >= 0)
		{
			int n=std::min(_ncols-d,_nrows);
			return const_anti_diag_iterator(_data.begin()+(_ncols-1-d)*_nrows +n*(1-_nrows),1-_nrows);
		}
		else
		{
			int n=std::min(_nrows+d,_ncols);
			return const_anti_diag_iterator(_data.begin()+(_ncols-1)*_nrows-d +n*(1-_nrows),1-_nrows);
		}
	}

	reverse_anti_diag_iterator anti_diag_rbegin(int d=0)
	{	
		return (reverse_anti_diag_iterator(anti_diag_end(d)));
	}

	reverse_anti_diag_iterator anti_diag_rend(int d=0)
	{
		return (reverse_anti_diag_iterator(anti_diag_begin(d)));
	}

	const_reverse_anti_diag_iterator anti_diag_rbegin(int d=0) const
	{	
		return (const_reverse_anti_diag_iterator(anti_diag_end(d)));
	}

	const_reverse_anti_diag_iterator anti_diag_rend(int d=0) const
	{	
		return (const_reverse_anti_diag_iterator(anti_diag_begin(d)));
	}

	///standard constructor 
	mat()
	{
		_ncols = 0;
		_nrows = 0;
	}


	///constructor creates a nrows x ncols full matrix
	mat(unsigned nrows, unsigned ncols) : _data(nrows*ncols)
	{
		_nrows = nrows;
		_ncols = ncols;			
	}
	
	///construct a matrix with all elements set to c
	mat(unsigned nrows, unsigned ncols, const T& c) :_data(nrows*ncols)
	{
		_nrows = nrows;
		_ncols = ncols;				
		_data.fill(c);
	}


	///creates a matrix from an array 
	///if the matrix data is stored in a row major fashion set column_major to false
	mat(unsigned nrows, unsigned ncols, const T* marray,bool column_major=true) 
		:_data(nrows*ncols)
	{ 
		_nrows = nrows;
		_ncols = ncols;	
		if(column_major)
			memcpy(_data, marray, size()*sizeof(T));
		else
		{
			
			for(unsigned i = 0; i < _nrows; i++)
				for(unsigned j = 0; j < _ncols;j++)
				{
					operator()(i,j) = marray[i*_ncols +j];
				}
				
			
		}
	}



	///copy constructor for matrix with different element type
	template <typename S>
	mat(const mat<S>& m):_data(m.size())
	{ 
		_nrows = m.nrows();
		_ncols = m.ncols();	
		for(unsigned i = 0; i < _nrows; i++)
			for(unsigned j = 0; j < _ncols;j++)
			{
				operator()(i,j)=(T)m(i,j);
			}

	}
	
	/// set data pointer to an external data array
	void set_extern_data(unsigned nrows, unsigned ncols, T* data)
	{
		_data.set_extern_data(nrows*ncols,data);
		_nrows = nrows;
		_ncols = ncols;				
	}
	void destruct()
	{
		_data.destruct();
	}
	///number of stored elements
	unsigned size() const 
	{
		return _data.size();
	}

	///number of rows
	unsigned nrows() const
	{
		return _nrows;
	}

	///number of columns
	unsigned ncols() const
	{
		return _ncols;
	}

	
	
	///assignment of a matrix with a different element type
	template <typename S> 
	mat<T>& operator = (const mat<S>& m) 
	{  
		resize(m.nrows(),m.ncols());
		for(unsigned i = 0; i < _nrows; i++)
			for(unsigned j = 0; j < _ncols;j++)
			{
				operator()(i,j)=(T)m(i,j);
			}
		return *this;
	}

	///assignment of a scalar s to each element of the matrix
	mat<T>& operator  = (const T& s) 
	{ 
		fill (s);
		return *this; 
	}

	///resize the matrix, the content of the matrix will be destroyed
	void resize(unsigned rows, unsigned cols)
	{
		
		unsigned newsize = rows*cols;
		_nrows = rows;
		_ncols = cols;
		_data.resize(newsize);
	}

	///cast operator for non const array 
	operator T*()
	{
		return (T*)_data;
	}

	///cast operator const array
	operator const T*() const
	{
		return (const T*)_data;
	}

	///returns true if matrix is a square matrix
	bool is_square() const
	{
		return _ncols == _nrows;
	}

	///fills all elements of the matrix with v
	template <typename S>
	void fill(const S& v)
	{
		T val = (T)v;
		_data.fill(val); 
	}

	///access to the element in the ith row in column j
	T& operator () (unsigned i, unsigned j)  
	{
		assert(i < _nrows && j < _ncols);
		return _data[j*_nrows+i]; 
	}
	
	///const access to the element in the ith row on column j 
	const T& operator () (unsigned i, unsigned j) const 
	{
		assert(_data != NULL && i < _nrows && j < _ncols);
		return _data[j*_nrows+i]; 
	}

	///test for equality
	template <typename S> 
	bool operator == (const mat<S>& m) const
	{ 
		if(_ncols != m.ncols() || _nrows != m.nrows())
			return false;

		return _data == m._data;
	}

	///test for inequality
	template <typename S> 
	bool operator != (const mat<S>& m) const
	{ 
		if(_ncols != m.ncols() || _nrows != m.nrows())
			return false;
		return _data != m._data;
		return false; 
	}


	//in place scalar multiplication
	mat<T>& operator *= (const T& s)			
	{ 
		_data *= (T)s;
		return *this; 
	}

	///scalar multiplication  
	const mat<T> operator*(const T& s) const
	{		
		mat<T> r=(*this);
		r*=(T)s;
		return r;
	}

	///in place division by a scalar
	mat<T>& operator /= (const T& s)			
	{ 
		_data /= (T)s;
		return *this; 
	}

	/// division by a scalar
	const mat<T> operator / (const T& s)	const		
	{ 
		mat<T> r=(*this);
		r/=s;
		return r;
	}


	///in place addition by a scalar
	mat<T>& operator += (const T& s)			
	{ 
		_data += (T)s;
		return *this; 
	}

	///componentwise addition of a scalar
	const mat<T> operator + (const T& s)			
	{ 
		mat<T> r=(*this);
		r+=s;
		return r;
	}

	///in place substraction of a scalar
	mat<T>& operator -= (const T& s)			
	{ 
		_data -= (T)s;
		return *this; 
	}

	/// componentwise subtraction of a scalar
	const mat<T> operator - (const T& s)			
	{ 
		mat<T> r=(*this);
		r-=s;
		return r;
	}

	///negation operator
	const mat<T> operator-() const
	{ 
		mat<T> r=(*this)*((T)-1);
		return r;
	}

	///in place addition of matrix
	template <typename S> 
	mat<T>& operator += (const mat<S>& m) 
	{ 
		assert(_ncols == m.ncols() &&  _nrows == m.nrows());
		_data+=m._data;
		return *this; 
	}

	///in place subtraction of matrix
	template <typename S> 
	mat<T>& operator -= (const mat<S>& m) 
	{ 
		assert(_ncols == m.ncols() &&  _nrows == m.nrows());
		_data-=m._data;
		return *this; 
	}

	

	
	///matrix addition
	template <typename S>
	const mat<T> operator+(const mat<S> m2)const
	{
		mat<T> r=(*this);
		r += m2; 
		return r;
	}

	///matrix subtraction
	template <typename S>
	const mat<T> operator-(const mat<S> m2)const
	{
		mat<T> r=(*this);
		r-= m2;
		return r;
	}


	///in place matrix multiplication with  a ncols x ncols matrix m2
	template <typename S>
	const mat<T> operator*=(const mat<S>& m2) 
	{
		assert(ncols() == m2.ncols() && nrows() == m2.nrows() && ncols() == nrows());
		mat<T> r(_nrows,_ncols,(T)0);
	
		for(unsigned i = 0; i < _nrows; i++)
			for(unsigned j = 0; j < _ncols;j++)
				for(unsigned k = 0; k < _ncols; k++)
					r(i,j) += operator()(i,k) * (T)(m2(k,j)); 
		(*this)=r;
	
		return *this;
	}

	

	///multiplication with a ncols x M matrix m2
	template <typename S>
	const mat<T> operator*(const mat<S>& m2) const
	{
		assert(m2.nrows() == _ncols);
		unsigned M = m2.ncols();
		mat<T> r(_nrows,M,(T)0);
		for(unsigned i = 0; i < _nrows; i++)
			for(unsigned j = 0; j < M;j++)
				for(unsigned k = 0; k < _ncols; k++)
					r(i,j) += operator()(i,k) * (T)(m2(k,j)); 
	
		return r;
	}


	///matrix vector multiplication
	template < typename S>
	const vec<T> operator*(const vec<S>& v) const
	{
		assert(_ncols==v.size());		
		vec<T> r;
		r.zeros(_nrows);
	
		for(unsigned j = 0; j < _ncols; j++)
			for(unsigned i = 0; i < _nrows; i++)
				r(i) += operator()(i,j) * (T)(v(j)); 
	
		return r;
	}

	///create submatrix m(top,left)...m(top+rows,left+cols)
	mat<T> sub_mat(unsigned top, unsigned left, unsigned rows, unsigned cols) const
	{
		
		mat<T> mnew(rows,cols);
	
		for(unsigned i = 0; i < rows;i++)
			for(unsigned j = 0; j < cols;j++) 
				mnew(i,j)=operator()(i+top,j+left);		
		
		return mnew;
	}

	///extract a row from the matrix as a vector
	const vec<T> row(unsigned i) const
	{
		
		vec<T> mnew(_ncols);
	
		
		for(unsigned j = 0; j < _ncols;j++) 
			mnew(j)=operator()(i,j);		
		
		return mnew;
	}

	///set row i of the matrix to vector v
	void set_row(unsigned i,const vec<T>& v) 
	{
			
		for(unsigned j = 0; j < _ncols;j++) 
			operator()(i,j)=v(j);		
		
	
	}

	///extract a column of the matrix as a vector
	const vec<T> col(unsigned j) const
	{
		
		vec<T> mnew(_nrows);
	
		
		for(unsigned i = 0; i < _nrows;i++) 
			mnew(i)=operator()(i,j);		
		
		return mnew;
	}

	///set  column j of the matrix to vector v
	void set_col(unsigned j,const vec<T>& v) 
	{
			
		for(unsigned i = 0; i < _nrows;i++) 
			operator()(i,j)=v(i);		
	}




	///copy submatrix m(top,left)...m(top+rows,left+cols) into submat
	void copy(unsigned top, unsigned left, unsigned rows, unsigned cols, mat<T>& submat) const
	{
		assert(submat.nrows() == rows && submat.ncols() == cols); 
			
		for(unsigned i = 0; i < rows;i++)
			for(unsigned j = 0; j < cols;j++) 
				submat(i,j)=operator()(i+top,j+left);		
		
		
	}
	
	///paste matrix m at position: top, left
	void paste(int top, int left,const mat<T>& m)
	{
		for(unsigned i = 0; i < m.nrows(); i++)
			for(unsigned j = 0; j < m.ncols(); j++) 
				operator()(i+top,j+left)=m(i,j);

	}


	///exchange row i with row j
	void swap_rows(unsigned i, unsigned j)
	{
		assert(i < _nrows && j < _nrows);		
		for(unsigned k = 0; k < _ncols;k++)
			std::swap(operator()(i,k),operator()(j,k));
	}
	
	///exchange column i with column j
	void swap_columns(unsigned i, unsigned j)
	{
		assert(i < _ncols && j < _ncols);		
		for(unsigned k = 0; k < _nrows;k++)
			std::swap(operator()(k,i),operator()(k,j));

	}

	///exchange diagonal elements (i,i) (j,j)
	void swap_diagonal_elements(unsigned i, unsigned j)
	{
		assert(i < _ncols && j < _ncols);		
		std::swap(operator()(i,i),operator()(j,j));
	}




	///returns the trace 
	T trace() const
	{
		assert(_nrows == _ncols);
		T t = 0;
		for(unsigned i = 0; i < _nrows;i++)
			t+=operator()(i,i);
		return t;
	}
	//return the maximum of all coefficients of the matrix
	T maxcoeff() const 
	{
		T t = operator()(0, 0);
		for (unsigned i = 0; i < _nrows; i++)
			for (unsigned j = 0; j < _ncols; j++)
				if (t < operator()(i, j))
					t = operator()(i, j);
		return t;	
	}
	//return the minimum of all coefficients of the matrix
	T mincoeff() const
	{
		T t = operator()(0, 0);
		for (unsigned i = 0; i < _nrows; i++)
			for (unsigned j = 0; j < _ncols; j++)
				if (t > operator()(i, j))
					t = operator()(i, j);
		return t;
	}

	///transpose matrix
	void transpose()
	{
		mat<T> r(_ncols,_nrows);
	
		for(unsigned i = 0; i < _nrows;i++)
			for(unsigned j = 0; j < _ncols;j++)
				r(j,i) = operator()(i,j);

		*this = r;
	}

	//flip columns left-right
	void fliplr()
	{
		mat<T> r(_nrows,_ncols);
	
		for(unsigned i = 0; i < _nrows;i++)
			for(unsigned j = 0; j < _ncols;j++)
				r(i,j) = operator()(i,_ncols-j-1);

		*this = r;

	}

	//flip rows up-down
	void flipud()
	{
		mat<T> r(_nrows,_ncols);
	
		for(unsigned i = 0; i < _nrows;i++)
			for(unsigned j = 0; j < _ncols;j++)
				r(i,j) = operator()(_nrows-i-1,j);

		*this = r;

	}

	///ceil all components of the matrix
	void ceil()
	{
		for(unsigned i = 0; i < _nrows;i++)
			for(unsigned j = 0; j < _ncols;j++)
				operator()(i,j) =::ceil(operator()(i,j));
	}

	///floor all components of the matrix
	void floor()
	{
		for(unsigned i = 0; i < _nrows;i++)
			for(unsigned j = 0; j < _ncols;j++)
				operator()(i,j) =::floor(operator()(i,j));
	}
	
	///round to integer 
	void round()
	{
		for(unsigned i = 0; i < _nrows;i++)
			for(unsigned j = 0; j < _ncols;j++)
				operator()(i,j) =::floor(operator()(i,j)+(T)0.5);

	}
	

	
	///returns the frobenius norm of matrix m
	T frobenius_norm() const
	{
		T n=0;
		for(unsigned i =0; i < _nrows;i++)
			for(unsigned j=0;j <_ncols;j++)
				n+=operator()(i,j)*operator()(i,j);
	
		return (T)sqrt((double)n);
	}


	///set identity matrix
	void identity()
	{
		assert(_ncols == _nrows);
		zeros();
		for(unsigned i = 0; i < _ncols;++i)
			operator()(i,i)=1;
	}
	
	///set dim x dim identity matrix 
	void identity(unsigned dim)
	{
		zeros(dim,dim);
		for(unsigned i = 0; i < _ncols;++i)
			operator()(i,i)=1;
	}

	///set zero matrix
	void zeros()
	{
		fill(0);
	}

	///resize and fill matrix with zeros
	void zeros(unsigned rows, unsigned cols)
	{
		resize(rows,cols);
		fill((T)0);
	}

	///resize and fill matrix with ones
	void ones(unsigned rows, unsigned cols)
	{
		resize(rows,cols);
		fill((T)1);
	}




};

template <typename T>
mat<T> zeros(unsigned rows, unsigned cols)
{
	mat<T> m;
	m.zeros(rows,cols);
	return m;
}

template <typename T>
mat<T> ones(unsigned rows, unsigned cols)
{
	mat<T> m(rows,cols);
	m.fill((T)1);
	return m;
}

template<typename T>
mat<T> identity(unsigned dim)
{
	mat<T> m;
	m.identity(dim);
	return m;
}

//return frobenius norm
template<typename T>
T frobenius_norm(const mat<T>& m)
{
	return m.frobenius_norm();
}

//return trace of matrix
template<typename T>
T trace(const mat<T>& m)
{
	return m.trace();
}


//concatenate two matrices horizontally
template<typename T, typename S>
const mat<T> horzcat(const mat<T>& m1, const mat<S>& m2)
{
	
	assert(m1.nrows() == m2.nrows());
	mat<T> r(m1.nrows(),m1.ncols()+m2.ncols());
	for(unsigned i = 0; i < m1.nrows();i++)
	{
		for(unsigned j = 0; j < m1.ncols(); j++)
			r(i,j) = m1(i,j);
		for(unsigned j = 0; j < m2.ncols(); j++)
			r(i,j+m1.ncols())=(T)m2(i,j);
	}
	return r;
}

//concatenates a matrix and a column vector horizontally
template<typename T, typename S>
const mat<T> horzcat(const mat<T>& m1, const vec<S>& v)
{
	
	assert(m1.nrows() == v.size());
	mat<T> r(m1.nrows(),m1.ncols()+1);
	for(unsigned i = 0; i < m1.nrows();i++)
	{
		for(unsigned j = 0; j < m1.ncols(); j++)
			r(i,j) = m1(i,j);
		
		r(i,m1.ncols())=(T)v(i);
	}
	return r;
}

//concatenates two vectors horizontally
template<typename T, typename S>
const mat<T> horzcat(const vec<T>& v1, const vec<S>& v2)
{
	
	assert(v1.size() == v2.size());
	mat<T> r(v1.size(),2);
	for(unsigned i = 0; i < v1.size();i++)
	{
		r(i,0) = v1(i);
		r(i,1) = v2(i);
	}
	return r;
}

//concatenates two column vectors vertically
template<typename T, typename S>
const vec<T> vertcat(const vec<T>& v1, const vec<S>& v2)
{
	
	vec<T> r(v1.size()+v2.size());
	unsigned off = v1.size();
	for(unsigned i = 0; i < v1.size();i++)
		r(i) = v1(i);
	for(unsigned i = 0; i < v2.size();i++)
		r(i+off) = v2(i);
	
	return r;
}


//concatenates two matrices vertically
template<typename T, typename S>
const mat<T> vertcat(const mat<T>& m1, const mat<S>& m2)
{
	
	assert(m1.ncols() == m2.ncols());
	mat<T> r(m1.nrows()+m2.nrows(),m1.ncols());
	for(unsigned i = 0; i < m1.nrows();i++)
	{
		for(unsigned j = 0; j < m1.ncols(); j++)
			r(i,j) = m1(i,j);
	}

	for(unsigned i = 0; i < m2.nrows();i++)
	{
		for(unsigned j = 0; j < m1.ncols(); j++)
			r(i+m1.nrows(),j) = m2(i,j);
	}

	return r;
}




//transpose of a matrix m
template <typename T>
const mat<T> transpose(const mat<T> &m)
{
	mat<T> r = m;
	r.transpose();
	return r;
}
//transpose of a column vector
template <typename T>
const mat<T> transpose(const vec<T> &v)
{
	mat<T> r(1, v.size());
	for(unsigned i = 0; i < v.size(); i++)
		r(0,i)= v(i);

	return r;
}

///ceil all components of the matrix
template <typename T>
const mat<T> ceil(const mat<T> &m)
{
	mat<T> r = m;
	r.ceil();
	return r;	
}

///floor all components of the matrix
template <typename T>
const mat<T> floor(const mat<T> &m)
{
	mat<T> r = m;
	r.floor();
	return r;	
}
	
///round all components of the matrix
template <typename T>
const mat<T> round(const mat<T> &m)
{
	mat<T> r = m;
	r.round();
	return r;	
}




///return the product of a scalar s and a matrix m
template <typename T> 
mat<T>  operator * (const T& s, const mat<T>& m)
{ 
	return m*(T)s; 
}



///output of a matrix onto an ostream
template <typename T>
std::ostream& operator<<(std::ostream& out, const mat<T>& m)
{
	
	for (unsigned i=0;i<m.nrows();++i)
	{
		for(unsigned j =0;j < m.ncols()-1;++j)
			out << m(i,j)<<" ";
		out << m(i,m.ncols()-1);
		if(i != m.nrows()-1)
			out <<"\n";
	}

	return out;

}


///input of a matrix onto an ostream
template <typename T>
std::istream& operator>>(std::istream& in, mat<T>& m)
{
	assert(m.size() > 0);
	for (unsigned i=0;i<m.nrows();++i)
		for(unsigned j =0;j < m.ncols();++j)
			in >> m(i,j);
	

	return in;

}






//compute transpose(A)*A
template <typename T>
void AtA(const mat<T>& a, mat<T>& ata)
{
	ata.resize(a.ncols(),a.ncols());
	ata.zeros();
	for(unsigned r = 0; r < a.nrows();r++)
	{
		for(unsigned i = 0; i < a.ncols();i++)
		{
			for(unsigned j = 0; j < a.ncols();j++)
			{
				ata(i,j)+=a(r,i)*a(r,j);
			}
		}
	}
}
//compute A*transpose(A)
template <typename T>
void AAt(const mat<T>& a, mat<T>& aat)
{
	aat.resize(a.nrows(),a.nrows());
	aat.zeros();
	for(unsigned c = 0; c < a.ncols();c++)
	{
		for(unsigned i = 0; i < a.nrows();i++)
		{
			for(unsigned j = 0; j < a.nrows();j++)
			{
				aat(i,j)+=a(i,c)*a(j,c);
			}
		}
	}
	
}

template <typename T>
mat<T> reshape(const vec<T>& v,unsigned r,unsigned c)
{
	assert(v.size() == r*c);
	return mat<T>(r,c,(const T*)v);
}

template <typename T>
mat<T> reshape(const mat<T>& m,unsigned r,unsigned c)
{
	assert(m.size() == r*c);
	return mat<T>(r,c,(const T*)m);
}


template <typename T>
void AtB(const mat<T>& a,const mat<T>& b, mat<T>& atb)
{
	atb.resize(a.ncols(),b.ncols());
	atb.zeros();
	
	for(unsigned i = 0; i < a.ncols(); i++)
		for(unsigned j = 0; j < b.ncols();j++)
			for(unsigned k = 0; k < a.nrows(); k++)
				atb(i,j) += a(k,i)*b(k,j); 
	
}

///multiply A^T*x 
/// A is a matrix and x is a vector
template <typename T>
void Atx(const mat<T>& a,const vec<T>& x, vec<T>& atx)
{
	atx.resize(a.ncols());
	atx.zeros();
	
	for(unsigned i = 0; i < a.ncols(); i++)
		for(unsigned j = 0; j < a.nrows(); j++)
			atx(i) += a(j,i) * (T)(x(j)); 
	
	
}


///returns the outer product of vector v and w
template < typename T, typename S>
mat<T> dyad(const vec<T>& v, const vec<S>& w) 
{ 
	mat<T> m(v.size(),w.size());

	for(unsigned i = 0; i < v.size();i++)
		for(unsigned j = 0; j < w.size();j++)
			m(i,j) =v(i)*(T)w(j); 
	
	return m;
}

} // namespace math

/// @name Predefined Types
/// @{

/// declare type of matrices of varying dimensions
typedef cgv::math::mat<float> matn;
/// declare type of matrices of varying dimensions
typedef cgv::math::mat<double> dmatn;

/// @}

} // namespace cgv
