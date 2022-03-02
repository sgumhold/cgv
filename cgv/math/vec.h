#pragma once

#include <iostream>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <string.h>
#include <vector>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace cgv {
	namespace math { 


/// A column vector class.
template <typename T>
class vec
{	

protected:
	///pointer to _data storage
	T *_data;
	///number or elements
	unsigned _size;
	/// store whether data is not owned by vector
	bool data_is_external;
public:
	typedef T              value_type;
	typedef T&             reference;
    typedef const T&       const_reference;
    typedef std::size_t    size_type;
	typedef std::ptrdiff_t difference_type;
	typedef T*             pointer;
	typedef const T*       const_pointer;

    typedef T*             iterator;
    typedef const T*       const_iterator;
    
   
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	iterator begin()
	{
		return _data;
	}

	iterator end()
	{
		return _data+_size;
	}

	const_iterator begin() const
	{
		return _data;
	}

	const_iterator end() const
	{
		return _data+_size;
	}

	reverse_iterator rbegin()
	{
		return reverse_iterator(end());
	}

	reverse_iterator rend()
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator rbegin() const
	{
		return const_reverse_iterator(end());
	}

	const_reverse_iterator rend() const
	{
		return const_reverse_iterator(begin());
	}




	///number of elements
	unsigned size() const
	{
		return _size;
	}

	///number of elements
	unsigned dim() const
	{
		return _size;
	}
	
	///standard constructor
	vec()
	{
		_data = NULL;
		_size = 0;
		data_is_external = false;
	}

	///creates a vector with dim elements
	explicit vec(unsigned dim, const T& value = T(0))
	{
		_size=dim;
		if (dim > 0) {
			_data = new T[_size];
			std::fill(_data, _data + _size, value);
		}
		else
			_data = NULL;
		data_is_external = false;
	}


	///creates a vector with dim elements from an array
	vec(unsigned dim, const T* marray)
	{
		_size = dim;
		if(dim > 0)
		{		
			_data = new T[_size];
			memcpy(_data,marray,_size*sizeof(T));
		}else
			_data = NULL;
		data_is_external = false;
	}

	///copy constructor for vectors with equal element type	
	vec(const vec<T>&v)
	{
		_size = v.size();
		if (v._data) {
			_data = new T[v.size()];
			memcpy(_data,v._data,_size*sizeof(T));
		}
		else
			_data=NULL;
		data_is_external = false;
	}

	///copy constructor for vectors with different element type
	template <typename S>
	vec(const vec<S>& v)
	{
		_size = v.size();
		if(v.size() > 0) {
			_data = new T[v.size()];
			for(unsigned i = 0; i < _size;i++)
				_data [i]=(T)v(i);
		}
		else
			_data =NULL;
		data_is_external = false;
	}

	///creates a 3d vector (c0,c1,c2)^T
	vec(const T& c0, const T& c1)
	{
		_size = 2;
		_data = new T[2];
		_data[0] = c0; 
		_data[1] = c1; 	
		data_is_external = false;
	}

	
	///creates a 3d vector (c0,c1,c2)^T
	vec(const T& c0, const T& c1, const T& c2)
	{
		_size = 3;
		_data = new T[3];
		_data[0] = c0; 
		_data[1] = c1; 
		_data[2] = c2;
		data_is_external = false;
	}

	///creates a 4d vector (c0,c1,c2,c3)^T
	vec(const T& c0, const T& c1, const T& c2, const T& c3)
	{
		_size = 4;
		_data = new T[4];
		_data[0] = c0; 
		_data[1] = c1; 
		_data[2] = c2;
		_data[3] = c3;
		data_is_external = false;
	}

	///set entries of a 2d vector
	void set(const T& c0, const T& c1) 
	{
		assert(_size == 2);
		_data[0] = c0; 
		_data[1] = c1; 
	}

	///set entries of a 3d vector
	void set(const T& c0, const T& c1, const T& c2) 
	{
		assert(_size == 3);
		_data[0] = c0; 
		_data[1] = c1; 
		_data[2] = c2;
	}

	///set entries of a 4d vector
	void set(const T& c0, const T& c1, const T& c2, const T& c3) 
	{
		assert(_size == 4);
		_data[0] = c0; 
		_data[1] = c1; 
		_data[2] = c2;
		_data[3] = c3;
	}
	/// set data pointer to an external data array
	void set_extern_data(unsigned dim, T* data)
	{
		destruct();
		_size = dim;
		_data = data;
		data_is_external = true;
	}
	///destructor
	virtual ~vec()
	{
		destruct();
	}

	void destruct()
	{
		if(_data && !data_is_external)
		{
			delete[] _data;
			_data=NULL;
			_size=0;
		}
	}

	///cast into non const array 
	operator T*()
	{
		return _data;
	}

	///cast into const array
	operator const T*() const
	{
		return _data;
	}

	///assignment of  a vector v
	vec<T>& operator = (const vec<T>& v) 
	{ 
		if(v.size() == 0)
			destruct();
		else {
			if (size() != v.size())
				resize(v.size());
			memcpy(_data,v._data,_size*sizeof(T));		
		}
		return *this; 
	}

	///assignment of  a scalar s
	vec<T>& operator = (const T& s) 
	{ 
		fill(s);
		return *this; 
	}

	///assignment of  a vector v
	template <typename S> 
	vec<T>& operator = (const vec<S>& v) 
	{ 
		resize(v.size());
		for (unsigned i=0;i<_size;++i) _data[i]=(T)v(i); 
		return *this; 
	}

	///element accessor
	T& operator () (unsigned i)  
	{
		assert(i < _size);
		return _data[i]; 
	}

	///const element accessor
	const T& operator () (unsigned i) const 
	{
		assert(i < _size);
		return _data[i]; 
	}

	///element accessor for the first element
	T& first()   
	{
		assert( _size > 0);
		return _data[0]; 
	}

	///const element accessor for the first element
	const T& first()  const 
	{
		assert( _size > 0);
		return _data[0]; 
	}



	///element accessor for the flast element
	T& last()   
	{
		assert( _size > 0);
		return _data[_size-1]; 
	}

	///const element accessor for the last element
	const T& last()  const 
	{
		assert( _size > 0);
		return _data[_size-1]; 
	}
	
	///element accessor for the first element
	T& x()   
	{
		assert( _size > 0);
		return _data[0]; 
	}

	///const element accessor for the first element
	const T& x()  const 
	{
		assert( _size > 0);
		return _data[0]; 
	}

	///element accessor for the second element
	T& y()   
	{
		assert( _size > 1);
		return _data[1]; 
	}

	///const element accessor for the second element
	const T& y()  const 
	{
		assert( _size > 1);
		return _data[1]; 
	}

	///element accessor for the third element
	T& z()   
	{
		assert( _size > 2);
		return _data[2]; 
	}

	///const element accessor for the third element
	const T& z()  const 
	{
		assert( _size > 2);
		return _data[2]; 
	}

	///element accessor for the fourth element
	T& w()   
	{
		assert( _size > 3);
		return _data[3]; 
	}

	///const element accessor for the fourth element
	const T& w()  const 
	{
		assert( _size > 3);
		return _data[3]; 
	}

	///in place addition of a scalar s
	vec<T>& operator += (const T& s)			
	{
		 for (unsigned i=0;i<_size;++i) 
			_data[i] += s; 
		return *this; 
	}

	///in place subtraction by scalar s
	vec<T>& operator -= (const T& s)
	{
		for (unsigned i=0;i<_size; ++i) 
			_data[i] -= s; 
		return *this; 
	}

	///in place multiplication with s
	vec<T>& operator *= (const T& s)
	{ 
		for (unsigned i=0;i<_size;++i) _data[i] *= s; return *this; 
	}

	///in place division by scalar s
	vec<T>& operator /= (const T& s)			
	{ 		
		for (unsigned i=0;i<_size;++i) _data[i] /= s; return *this; 
	}

	///in place vector addition
	template <typename S> 
	vec<T>& operator += (const vec<S>& v) 
	{ 
		assert(_size == v._size);
		for (unsigned i=0;i<_size;++i) _data[i] += v(i); return *this; 
	}

	///in place vector subtraction
	template <typename S> 
	vec<T>& operator -= (const vec<S>& v) 
	{
		assert(_size == v._size);
		for (unsigned i=0;i<_size;++i) _data[i] -= v(i); return *this; 
	}

	///in place componentwise vector multiplication
	template <typename S> 
	vec<T>& operator *= (const vec<S>& v) 
	{
		assert(_size == v._size);
		 for (unsigned i=0;i<_size;++i) _data[i] *= v(i); return *this; 
	}

	///in place componentwise vector division
	template <typename S> 
	vec<T>& operator /= (const vec<S>& v) 
	{
		assert(_size == v._size);
		 for (unsigned i=0;i<_size;++i) _data[i] /= v(i); return *this; 
	}
	
	///vector addition
	template <typename S> 
	const vec<T>  operator +  (const vec<S>& v) const 
	{ 
		vec<T> r = *this; r += v; return r; 
	}

	///componentwise addition of scalar
	const vec<T>  operator +  (const T& s) const 
	{ 
		vec<T> r = *this; r += s; return r; 
	}

	///componentwise subtraction of scalar
	const vec<T>  operator -  (const T& s) const 
	{ 
		vec<T> r = *this; r -= s; return r; 
	}

	///vector subtraction
	template <typename S> 
	vec<T>  operator -  (const vec<S>& v) const 
	{
		 vec<T> r = *this; r -= v; return r; 
	}

	///componentwise vector multiplication
	template <typename S> 
	const vec<T>  operator *  (const vec<S>& v) const 
	{ 
		vec<T> r = *this; r *= v; return r; 
	}

	
	///componentwise vector division
	template <typename S> 
	const vec<T>  operator / (const vec<S>& v) const 
	{ 
		vec<T> r = *this; r /= v; return r; 
	}



	///negates the vector
	vec<T>  operator-(void) const 
	{
		vec<T> r=(*this);
		r=(T)(-1)*r;
		return r; 
	}

	///multiplication with scalar s
	vec<T>  operator * (const T& s) const 
	{
		 vec<T> r = *this; r *= s; return r; 
	}

	
	///divides vector by scalar s
	vec<T>  operator / (const T& s)const 
	{ 
		vec<T> r = *this;
		r /= s;
		return r; 
	}
	

	///fill elements of vector with scalar v
	void fill(const T& v)
	{
		for (unsigned i=0; i<_size; ++i) 
			_data[i]= (T)v; 
	}

	///fill the vector with zeros
	void zeros()
	{
		fill((T)0);
	}

	///fill the vector with ones
	void ones()
	{
		fill((T)1);
	}

	///resize the vector to size n and fills the vector with zeros
	void zeros(unsigned n)
	{
		resize(n);
		fill((T)0);
	}

	///resize the vector to size n and fills thevector with ones
	void ones(unsigned n)
	{
		resize(n);
		fill((T)1);
	}

	///resize the vector
	void resize(unsigned dim)
	{
		
		if(_data)
		{
			if(dim != _size)
			{
				destruct();
				_size=dim;
				if(dim > 0)
					_data = new T[dim];	
				else
					_data = NULL;
				data_is_external = false;
			}
		}else
		{
				_size=dim;
				if(dim > 0)
					_data = new T[dim];	
				else
					_data = NULL;
				data_is_external = false;
		}		
	}

	///test for equality
	template <typename S> 
	bool operator == (const vec<S>& v) const
	{ 
		for (unsigned i=0;i<_size;++i) 
			if(operator()(i) != (T)v(i)) return false; 
		return true; 
	}

	///test for inequality
	template <typename S> 
	bool operator != (const vec<S>& v) const
	{ 
		for (unsigned i=0;i<_size;++i) 
			if(operator()(i) != (T)v(i)) return true; 
		return false; 
	}

	///length of the vector L2-Norm
	T length() const
	{
		return (T)sqrt((double)sqr_length());
	}

	///componentwise absolute values
	void abs()
	{	
		if(std::numeric_limits<T>::is_signed)
		{
			for(unsigned i = 0; i < _size;i++)
				_data[i]=(T)std::abs((double)_data[i]);
		}
	}

	///ceil componentwise
	void ceil()
	{
		for(unsigned i = 0; i < _size;i++)
			_data[i]=(T)::ceil((double)_data[i]);
	}

	///floor componentwise
	void floor()
	{
		for(unsigned i = 0; i < _size;i++)
			_data[i]=(T)::floor((double)_data[i]);
	}

	///round componentwise
	void round()
	{
		for(unsigned i = 0; i < _size;i++)
			_data[i]=(T)::floor((double)_data[i]+0.5);	
	}


	///square length of vector
	T sqr_length() const
	{
		T l=0;		
		for(unsigned i = 0; i!=_size;i++)
			l+= operator()(i)*operator()(i);
		return l;
	}

	///normalize the vector using the L2-Norm
	void normalize() 
	{
		T l = (T)1.0/length();
		for(unsigned i = 0; i<_size; i++)
			 operator()(i)=l*operator()(i);
	}

	///normalize the vector if length is not zero using the L2-Norm
	void safe_normalize()
	{
		T l = length();
		if(std::abs(l) > std::numeric_limits<T>::epsilon()) {
			l = (T)1.0 / l;
			for(unsigned i = 0; i < _size; i++)
				operator()(i) = l * operator()(i);
		}
	}

	///extracts sub vector beginning at index  ifrom with given size
	vec<T> sub_vec(unsigned ifrom, unsigned size) const
	{
		
		vec<T> vnew(size);
	
		for(unsigned i = 0; i < size; i++)
				vnew(i)=operator()(i+ifrom);		
		
		return vnew;
	}

	///copy sub vector beginning at index  ifrom with given size s into subvec
	void copy(unsigned ifrom, unsigned s,vec<T>& subvec) const
	{
		assert(subvec.size() == s); 
		assert(ifrom+s <=size());
		
		for(unsigned i = 0; i < s; i++)
				subvec(i)=operator()(i+ifrom);		
	}

	///paste v into vector beginning at index pos ifrom
	void paste(unsigned ifrom,const vec<T>& v) 
	{
		
		assert(ifrom+v.size() <= size());
		for(unsigned i = 0; i < v.size(); i++)
				operator()(i+ifrom) = v(i);		
	}



};

///returns a normalized version of v
template<typename T>
vec<T> normalize(const vec<T>& v)
{
	vec<T> r = v;
	r.normalize();
	return r;
}


///returns a normalized version of v or zero vector if length is zero
template<typename T>
vec<T> safe_normalize(const vec<T>& v)
{
	vec<T> r = v;
	r.safe_normalize();
	return r;
}


///return the p-norm of the vector default is p == 1
template <typename T,typename S>
T p_norm(const vec<T>& values,const S& p=1)
{	
	assert(p > 0);
	unsigned N = values.size();

	T n=0;	

	for(unsigned i = 0; i < N;i++)
	{
		n+=pow(fabs(values(i)),(T)p);
	}

	return pow(n,(T)(1.0/(T)p));
}

///return the infinity norm of the vector 
template <typename T>
T inf_norm(const vec<T>& values)
{
	vec<T> r = abs(values);	

	return max_value(r);
}


///returns the length of v L2-norm
template<typename T>
T length(const vec<T>& v)
{
	return v.length();
}

///returns the square length of v
template<typename T>
T sqr_length(const vec<T>& v)
{
	return v.sqr_length();
}

///output of a vector
template<typename T>
std::ostream& operator<<(std::ostream& out, const vec<T>& v)
{

	for (unsigned i=0;i<v.size()-1;++i)
	{
		out << v(i)<<" ";	
	}
	out << v(v.size()-1);
	return out;

}

///input of a vector
template<typename T>
std::istream& operator>>(std::istream& in, vec<T>& v)
{

	for (unsigned i=0;i<v.size();++i)
	{
		in >> v(i);	
	}
	
	return in;

}


///returns the product of a scalar s and vector v
template <typename T>
const vec<T> operator * (const T& s, const vec<T>& v) 
{
	vec<T> r = v; r *= s; return r; 
}

///returns the dot product of vector v and w
template <typename T>
inline T dot(const vec<T>& v, const vec<T>& w) 
{ 
	T r = 0; 
	for (unsigned i=0;i<v.size();++i) r += v(i)*(T)w(i); 
	return r; 
}

///returns the cross product of vector v and w
template < typename T>
inline vec<T> cross(const vec<T>& v, const vec<T>& w) 
{ 
	vec<T> r(3);
	r(0)=  v(1)*(T)w(2) - v(2)*(T)w(1);
	r(1)= -v(0)*(T)w(2) + v(2)*(T)w(0);
	r(2)=  v(0)*(T)w(1) - v(1)*(T)w(0);
	return r;
}

///returns the double cross product of vector a, b and c a x(b x c)
template < typename T, typename S, typename U>
vec<T> dbl_cross(const vec<T> &a, const vec<S> &b, vec<U> &c)
{
	return dot(a,c)*b - dot(a,b)*c;
}

///returns the spat product (mixed vector product) of the vectors a, b and c
template < typename T, typename S, typename U>
T spat(const vec<T> &a,const vec<S> &b,const vec<U> &c)
{
	 return dot(cross(a,b),c);
}

///calculates the projection of v onto n
template <typename T>
vec<T> project(const vec<T> &v, const vec<T> &n)
{
	return dot(v,n)/dot(n,n)*n;
}


///calculates the reflected direction of  v; n is the normal of the reflecting surface
template <typename T>
vec<T> reflect(const vec<T> &v, const vec<T> &n)
{
	return v-(T)2.0*dot(v,n)/dot(n,n)*n;
}

///calculates the refracted direction of v on a surface with normal n and refraction indices c1,c2, *the optional parameter total reflection 
///will be set true if a total reflection occured otherwise false
template <typename T>
vec<T> refract(const vec<T> &v,const vec<T> &n,T c1, T c2,bool* total_reflection=NULL)
{
	
	T NdotV =-dot(n,v)/dot(n,n);
	T c = c2/c1;

	T cosasqr = (T)1.0-(c*c)*((T)1.0-NdotV*NdotV);
	
	//total reflection
	if(cosasqr < 0)
	{
		if(total_reflection)
			*total_reflection=true;
		return reflect(v,n);
	}
	else
	{
		if(total_reflection)
			*total_reflection=false;
		return	c*v + (c*NdotV - sqrt(cosasqr)/dot(n,n) )*n;
		 
	}

}



///create a zero vector
template <typename T>
vec<T> zeros(const unsigned dim)
{
	vec<T> v;
	v.zeros(dim);
	return v;
}

///create a one vector
template <typename T>
vec<T> ones(const unsigned dim)
{
	vec<T> v;
	v.ones(dim);
	return v;
}


///vector of componentwise floor values
template <typename T>
vec<T> floor(const vec<T> &v)
{
	vec<T> r(v.size());
	for(unsigned i = 0; i < v.size();i++)
		r(i)=::floor((T)v(i));
	
	return r;
}


///vector of componentwise ceil values
template <typename T>
vec<T> ceil(const vec<T> &v)
{
	vec<T> r(v.size());
	for(unsigned i = 0; i < v.size();i++)
		r(i)=::ceil(v(i));
	
	return r;
}

///vector of componentwise ceil values
template <typename T>
vec<T> round(const vec<T> &v)
{
	vec<T> r(v.size());
	for(unsigned i = 0; i < v.size();i++)
		r(i)=::floor(v(i)+0.5);
	
	return r;
}


///vector of componentwise absolute values
template <typename T>
vec<T> abs(const vec<T> &v)
{
	vec<T> r(v.size());
	for(unsigned i = 0; i < v.size();i++)
		r(i)=std::abs(v(i));

	return r;
}

///returns the minimal entry
template <typename T>
T min_value(const vec<T> &v)
{
	return *(std::min_element(&v(0),&v(v.size()-1)+1));
}



///returns the index of the smallest value
template <typename T>
unsigned min_index(const vec<T> &v)
{
	return (unsigned) (std::min_element(&v(0),&v(v.size()-1)+1)-&v(0));
}

///return the index of the largest entry
template <typename T>
unsigned max_index(const vec<T> &v)
{
	return (unsigned) (std::max_element(&v(0),&v(v.size()-1)+1)-&v(0));
}

///return the value of the largest entry
template <typename T>
T max_value(const vec<T> &v)
{
	return *(std::max_element(&v(0),&v(v.size()-1)+1));
}


///compute the mean of all values
template<typename T>
T mean_value(const vec<T>& values)
{

	unsigned N = values.size();

	T mu=0;

	for(unsigned i = 0; i < N;i++)
	{
		mu+=values(i);
	}
	mu/=(T)N;
	return mu;
}

///compute the variance of all values,
///normalization is done with n-1 (unbiased estimator)
template<typename T>
T var_value(const vec<T>& values)
{
	unsigned N = values.size();	

	T mu=mean_value(values);
	T v = 0;

	for(unsigned i = 0; i < N;i++)
	{
		v+=(values(i)-mu)*(values(i)-mu);	
	}

	if(N > 1)
		v/=(T)(N-1);

	return v;

}

///compute the range of all values 
template<typename T>
T range_value(const vec<T>& values)
{
	return max_value(values)-min_value(values);
}

///compute the median absolut deviation MAD
template<typename T>
T mad_value(const vec<T>& values)
{
	return  median_value(abs(values-median_value(values)));
}


///compute the standard deviation (sigma) of all values, 
///normalization is done by n-1 (unbiased estimator)
template<typename T>
T std_value(const vec<T>& values)

{
	T v = var_value(values);
	return sqrt(v);
}



///compute the mean and the variance (sigma^2) of all values
template<typename T>
void var_and_mean_value(const vec<T>& values, T& mu, T&var)
{

	unsigned N = values.size();	
	mu=0;

	for(unsigned i = 0; i < N;i++)
	{
		mu+=values(i);	
	}

	mu/=(T)N;
	var = 0;
	for(unsigned i = 0; i < N;i++)
	{
		var+=sqr(values(i)-mu);	
	}

	var/=(T)(N-1);	
}


///sort vector elements in ascending or descending order
template <typename T>
void sort_values(vec<T>& values, bool ascending=true)
{
	
	if(ascending)
		std::sort(&values(0),&values(values.size()-1)+1,std::less<T>());
	else
		std::sort(&values(0),&values(values.size()-1)+1,std::greater<T>());
}



///returns the sum of all entries
template <typename T>
T sum_values(const vec<T>& values)
{
	T v =0;
	for(unsigned i = 0; i < values.size(); i++)
		v+=values(i);
	return v;
}

///computes cumulative sums in cumsumvalues 
///cumsumvalues[i] = values[0]+...+values[i-1]
///returns the sum of all entries
template <typename T>
T cumsum_values(const vec<T>& values, vec<T>& cumsumvalues)
{
	cumsumvalues.resize(values.size());
	T v =0;
	for(unsigned i = 0; i < values.size(); i++)
	{
		cumsumvalues(i)=v;
		v+=values(i);
	}
	return v;
}



///returns the product of all entries
template <typename T>
T prod_values(vec<T>& values)
{
	T v =1;
	for(unsigned i = 0; i < values.size(); i++)
		v*=values(i);
	return v;
}


///returns the kth-smallest value of values
///the input vector values will be rearranged to have this value in location arr[k], 
///with all smaller elements moved to values[0..k-1] (in arbitrary order) and all 
///larger elements in values[k+1..n] (also in arbitrary order). 
template <typename T>
T select_value(unsigned k, vec<T>& values)
{
	if (k >= values.size())
		k = values.size()-1;
	std::nth_element(&values(0),&values(k),&values(values.size()-1)+1);
	return values(k);
}



///returns the value of values
///the input vector arr will be rearanged to have this value in location arr[k], 
///with all smaller elements moved to values[0..k-1] (in arbitrary order) and all 
///larger elements in values[k+1..n] (also in arbitrary order).
///if the number of components are even then the ceil((n+1)/2) entry is returned
template <typename T>
T select_median_value( vec<T>& values)
{	
	return select_value((values.size())/2,values);
}



///returns median element without modifying the given vector
template <typename T>
T median_value(const vec<T>& values)
{
	vec<T> c = values;
	return select_value((c.size())/2,c);
}



///create a linearly spaced vector with N values starting at first_val and ending ant last_val
template <typename T>
const vec<T>  lin_space(const T& first_val, const T& last_val, unsigned N=10)
{
	vec<T> lv(N);
	if(N == 1)
	{
		lv(0) = last_val;
		return lv;
	}
	T diff = last_val-first_val;

	for(unsigned i = 0; i < N; i++)
	{
		lv(i) = first_val + i*diff/((T)N-(T)1.0);
	}
	return lv;
}

//create N chebychev sample points for interval [first_val,last_val]
template <typename T>
const vec<T>  cheb_points(const T& first_val, const T& last_val,unsigned N=10)
{
	vec<T> lv(N) ;
	if(N == 1)
	{
		lv(0) = (T)(first_val+last_val)/2.0;
		return lv;
	}
	T diff = (last_val-first_val)/(T)2.0;

	for(unsigned i = 0; i < N; i++)
	{
		lv(i) =  diff*((first_val+1.0)-cos((T)((2*i+1)*3.14159)/((T)(2.0*(N-1)+2))));
	}
	return lv;
}



///create a log spaced vector with N values starting at 10^first_pow_of_10 and 
///ending at 10 last_val
template <typename T>
const vec<T>  log_space(const T& first_pow_of_10, const T& last_pow_of_10, unsigned N=10)
{

	vec<T> lv(N);
	if(N == 1)
	{
		lv(0) = pow((T)10.0,last_pow_of_10);
		return lv;
	}
	T diff = last_pow_of_10 - first_pow_of_10;

	for(unsigned i = 0; i < N; i++)
	{
		lv(i) = pow((T)10.0,(T)first_pow_of_10 + (T)i*diff/((T)N-(T)1.0));
	}
	return lv;
}



///linear interpolation returns (1-t)*v1 + t*v2
template <typename T>
const vec<T> lerp(const vec<T>& v1, const vec<T>& v2, T t)
{
	return (1-t)*v1+t*v2;
}



///linear interpolation returns (1-t)*v1 + t*v2
template <typename T>
const T lerp(const T& s1, const T& s2, T t)
{
	return (1-t)*s1+t*s2;
}



///spherical linear interpolation 
template <typename T>
const vec<T> slerp(const vec<T>& v0, const vec<T>& v1, T t)
{
	T dotv0v1 = dot(v0,v1);
	//clamp between [-1,1]
	if(dotv0v1 < -1)
		dotv0v1 = -1;

	if(dotv0v1 > 1)
		dotv0v1 = 1;

	T theta = acos(dotv0v1)*t;
	cgv::math::vec<T> v2 = normalize(v1 - (dotv0v1)*v0);
	return cos(theta)*v0 + sin(theta)*v2;
}



	}
}
