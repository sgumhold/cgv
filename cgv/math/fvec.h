#pragma once

#define _USE_MATH_DEFINES
#include <limits>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <cgv/type/standard_types.h>

namespace cgv {
	namespace math {

template <typename T> class vec;

/** A vector with zero based index.*/
template <typename T, cgv::type::uint32_type N>
class fvec 
{
protected:
	//elements of vector
	T v[N];
public:
	//@name type definitions
	//@{
	///
	typedef T              value_type;
	///
	typedef T&             reference;
	///
    typedef const T&       const_reference;
	///
    typedef std::size_t    size_type;
	///
	typedef std::ptrdiff_t difference_type;
	///
	typedef T*             pointer;
	///
	typedef const T*       const_pointer;
	///
    typedef T*             iterator;
	///
    typedef const T*       const_iterator;   
	///
	typedef std::reverse_iterator<iterator> reverse_iterator;
	///
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	//@}

	//@name iterator generation
	//@{
	///
	iterator begin() { return v; }
	///
	iterator end() { return v+N; }
	///
	const_iterator begin() const { return v; }
	///
	const_iterator end() const { return v+N; }
	///
	reverse_iterator rbegin() { return reverse_iterator(end()); }
	///
	reverse_iterator rend() { return reverse_iterator(begin()); }
	///
	const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
	/// reverse iterator pointing to the end of reverse iteration
	const_reverse_iterator rend() const	{ return const_reverse_iterator(begin()); }
	//@}

	//@name construction and assignment
	//@{
	///creates a vector not initialized
	fvec() {}
	///creates a vector, where all N components are initialized to the constant value a
	fvec(const T &a) { std::fill(v, v+N, a); }
	/// construct and init first two coordinates to the given values	
	fvec(const T &x, const T &y) { set(x,y); }
	/// construct and init first three coordinates to the given values	
	fvec(const T &x, const T &y, const T &z) { set(x,y,z); }
	/// construct and init first four coordinates to the given values	
	fvec(const T &x, const T &y, const T &z,const T &w) { set(x,y,z,w); }	
	///creates a vector from a n-element array a, if n < N remaining N-n elements are set to zero
	fvec(cgv::type::uint32_type n, const T *a) {
		cgv::type::uint32_type i, min_n = n < N ? n : N;
		std::copy(a, a+min_n, v);
		for (i = min_n; i < N; ++i) v[i] = T(0);
	}
	///creates a column vector initialized to array of a different type with zeros filled to not copied components
	template <typename S>
	fvec(cgv::type::uint32_type n, const S *a) { 
		cgv::type::uint32_type i, min_n = n < N ? n : N;
		for (i=0; i<min_n; ++i) v[i] = (T)a[i];
		for (; i < N; ++i) v[i] = T(0);
	}
	///copy constructor
	fvec(const fvec<T,N> &rhs) { if (this != &rhs) std::copy(rhs.v, rhs.v+N, v); }
	///copies a column vector of a different type
	template <typename S>
	fvec(const fvec<S,N>& fv) { for (unsigned i=0; i<N; ++i) v[i] = (T)fv(i); }
	/// construct from vector of one dimension less plus a scalar
	template <typename S1, typename S2>
	fvec(const fvec<S1, N - 1>& fv, S2 w) { for (unsigned i = 0; i < N - 1; ++i) v[i] = (T)fv(i); v[N - 1] = (T)w; }
	///assign vector rhs, if vector and rhs have different sizes, vector has been resized to match the size of
	fvec & operator = (const fvec<T,N> &rhs) { if (this != &rhs) std::copy(rhs.v, rhs.v+N, v); return *this; }
	/// set all components of vector to constant value a
	fvec & operator = (const T &a) { std::fill(v, v+N, a); return *this; }	
	///set the first two components
	void set(const T &x, const T &y) { v[0] = x; v[1] = y; }
	///set the first three components
	void set(const T &x, const T &y, const T &z) { v[0] = x; v[1] = y; v[2] = z; }
	///set the first four components
	void set(const T &x, const T &y, const T &z, const T &w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
	///fill elements of vector with scalar v
	void fill(const T& a) { std::fill(v, v+N, a); }
	///fill the vector with zeros
	void zeros() { fill((T)0); }
	///fill the vector with ones
	void ones() { fill((T)1); }
	/// convert to homogeneous version by adding a 1
	fvec<T,N+1> lift() const { fvec<T,N+1> h_v; (fvec<T,N>&)h_v=*this; h_v(N) = 1; return h_v; }
	/// conversion to vector type
	vec<T> to_vec() const;
	//@}

	//@name access to components
	//@{
	///first element
	T& x() { return v[0]; }
	///first element of const vector
	const T& x() const { return v[0]; }
	///second element
	T& y() { return v[1]; }
	///second element of const vector
	const T& y() const { return v[1]; }
	///third element
	T& z() { return v[2]; }
	///third element of const vector
	const T& z() const { return v[2]; }
	///fourth element
	T& w() { return v[3]; }
	///fourth element of const vector
	const T& w() const { return v[3]; }
	///access i'th element
	T& operator()(const int i) { return v[i]; }
	///access i'th element of const vector
	const T & operator()(const int i) const { return v[i]; }
	///return number of elements
	static cgv::type::uint32_type size() { return N; }
	///cast into array. This allows calls like glVertex<N><T>v(p) instead of glVertex<N><T,N>(p.x(),p.y(),....)
	operator T*() { return v; }
	///cast into const array
	operator const T*() const { return v; }
	//@}

	//@name operators
	//@{
	///in place addition of a scalar s
	fvec<T,N>& operator += (const T& s) { for (unsigned i=0;i<N;++i) v[i] += s; return *this; }
	///in place subtraction by scalar s
	fvec<T,N>& operator -= (const T& s) { for (unsigned i=0;i<N;++i) v[i] -= s; return *this; }
	///in place multiplication with s
	fvec<T,N>& operator *= (const T& s) { for (unsigned i=0;i<N;++i) v[i] *= s; return *this; }
	///in place division by scalar s
	fvec<T,N>& operator /= (const T& s) { for (unsigned i=0;i<N;++i) v[i] /= s; return *this; }
	///in place vector addition
	template <typename S> 
	fvec<T,N>& operator += (const fvec<S,N>& _v)  { for (unsigned i=0; i<N;++i) v[i] += _v(i); return *this; }
	///in place vector subtraction
	template <typename S> 
	fvec<T,N>& operator -= (const fvec<S,N>& _v) { for (unsigned i=0; i<N;++i) v[i] -= _v(i); return *this; }
	///in place componentwise vector multiplication
	template <typename S> 
	fvec<T,N>& operator *= (const fvec<S,N>& _v) { for (unsigned i=0; i<N;++i) v[i] *= _v(i); return *this; }
	///in place componentwise vector division
	template <typename S> 
	fvec<T,N>& operator /= (const fvec<S,N>& _v) { for (unsigned i=0; i<N;++i) v[i] /= _v(i); return *this; }
	///vector addition
	template <typename S> 
	fvec<T,N> operator + (const fvec<S,N>& v) const { fvec<T,N> r = *this; r += v; return r; }
	///componentwise addition of scalar
	fvec<T,N> operator + (const T& s) const { fvec<T,N> r = *this; r += s; return r; }
	///componentwise subtraction of scalar
	fvec<T,N>  operator - (const T& s) const { fvec<T,N> r = *this; r -= s; return r; }
	///vector subtraction
	template <typename S> 
	fvec<T,N>  operator -  (const fvec<S,N>& v) const { fvec<T,N> r = *this; r -= v; return r; }
	///componentwise vector multiplication
	template <typename S>
	fvec<T,N> operator *  (const fvec<S,N>& v) const { fvec<T,N> r = *this; r *= v; return r; }
	///componentwise vector division
	template <typename S> 
	fvec<T,N>  operator / (const fvec<S,N>& v) const { fvec<T,N> r = *this; r /= v; return r; }
	///negates the vector
	fvec<T,N>  operator-(void) const { fvec<T,N> r; for (unsigned i=0; i<N; ++i) r(i) = -v[i]; return r; }
	///multiplication with scalar s
	fvec<T,N>  operator * (const T& s) const { fvec<T,N> r = *this; r *= s; return r; }
	///divides vector by scalar s
	fvec<T,N>  operator / (const T& s) const { fvec<T,N> r = *this; r /= s; return r; }
	///test for equality
	template <typename S> 
	bool operator == (const fvec<S,N>& v) const { 
		for (unsigned i=0;i<N;++i) 
			if(operator()(i) != (T)v(i)) return false; 
		return true; 
	}
	///test for inequality
	template <typename S> 
	bool operator != (const fvec<S,N>& v) const { 
		for (unsigned i=0;i<N;++i) 
			if(operator()(i) != (T)v(i)) return true; 
		return false; 
	}
	//@}

	//@name functions
	//@{
	///length of the vector L2-Norm
	T length() const
	{
		return (T)sqrt((double)sqr_length());
	}

	///componentwise absolute values
	void abs() {	
		if(std::numeric_limits<T>::is_signed) {
			for(unsigned i = 0; i < N;i++)
				v[i]=(T)std::abs((double)v[i]);
		}
	}

	///ceil componentwise
	void ceil() {
		for(unsigned i = 0; i < N;i++)
			v[i]=(T)::ceil((double)v[i]);
	}

	///floor componentwise
	void floor() {
		for(unsigned i = 0; i < N;i++)
			v[i]=(T)::floor((double)v[i]);
	}

	///round componentwise
	void round() {
		for(unsigned i = 0; i < N;i++)
			v[i]=(T)::floor((double)v[i]+0.5);	
	}


	///square length of vector
	T sqr_length() const {
		T l=0;		
		for(unsigned i = 0; i!=N;i++)
			l+= operator()(i)*operator()(i);
		return l;
	}

	/// normalize the vector using the L2-Norm and return the length
	double normalize()  {
		T l     = length();
		T inv_l = (T)1.0/l;
		for(unsigned i = 0; i<N; i++)
			 operator()(i)=inv_l*operator()(i);
		return l;
	}
	//@}
};

/// return normalized vector
template<typename T, cgv::type::uint32_type N>
fvec<T,N> normalize(const fvec<T,N>& v) { fvec<T,N> w(v); w.normalize(); return w; }

///output of a vector
template<typename T, cgv::type::uint32_type N>
std::ostream& operator<<(std::ostream& out, const fvec<T,N>& v)
{
	for (unsigned i=0;i<N-1;++i)
		out << v(i)<<" ";
	out << v(N-1);
	return out;

}

///input of a vector
template<typename T, cgv::type::uint32_type N>
std::istream& operator>>(std::istream& in, fvec<T,N>& v)
{
	for (unsigned i=0;i<N;++i)
		in >> v(i);	
	return in;
}

///returns the product of a scalar s and vector v
template <typename T, cgv::type::uint32_type N>
fvec<T,N> operator * (const T& s, const fvec<T,N>& v) {	fvec<T,N> r = v; r *= s; return r; }

///returns the dot product of vector v and w
template <typename T, cgv::type::uint32_type N>
inline T dot(const fvec<T,N>& v, const fvec<T,N>& w)
{ 
	T r = 0; 
	for (unsigned i=0;i<N;++i)
		r += v(i)*w(i); 
	return r; 
}

///returns the length of vector v 
template <typename T, cgv::type::uint32_type N>
inline T length(const fvec<T, N>& v) { return sqrt(dot(v, v)); }

/// apply abs function component wise to vector
template <typename T, cgv::type::uint32_type N>
inline fvec<T, N> abs(const fvec<T, N>& v) { fvec<T, N> r(v); r.abs(); return r; }

/// apply round function component wise to vector
template <typename T, cgv::type::uint32_type N>
inline fvec<T, N> round(const fvec<T, N>& v) { fvec<T, N> r(v); r.round(); return r; }

/// apply floor function component wise to vector
template <typename T, cgv::type::uint32_type N>
inline fvec<T, N> floor(const fvec<T, N>& v) { fvec<T, N> r(v); r.floor(); return r; }

/// apply ceil function component wise to vector
template <typename T, cgv::type::uint32_type N>
inline fvec<T, N> ceil(const fvec<T, N>& v) { fvec<T, N> r(v); r.ceil(); return r; }

///returns the squared length of vector v 
template <typename T, cgv::type::uint32_type N>
inline T sqr_length(const fvec<T,N>& v) 
{
	return dot(v,v);
}


///returns the cross product of vector v and w
template < typename T, cgv::type::uint32_type N>
inline fvec<T,N> cross(const fvec<T,N>& v, const fvec<T,N>& w) 
{ 
	fvec<T,N> r(3);
	r(0)= v(1)*w(2) - v(2)*w(1);
	r(1)= v(2)*w(0) - v(0)*w(2);
	r(2)= v(0)*w(1) - v(1)*w(0);
	return r;
}

///returns the cross product of vector v and w
template < typename T, cgv::type::uint32_type N>
inline fvec<T,N+1> hom(const fvec<T,N>& v) 
{ 
	fvec<T,N+1> h;
	for (unsigned i = 0; i<N; ++i)
		h(i) = v(i);
	h(N) = 1;
	return h;
}

///returns the minimal entry
template < typename T, cgv::type::uint32_type N>
T min_value(const fvec<T,N> &v)
{
	return *(std::min_element(&v(0),&v(N-1)+1));
}

///returns the index of the smallest value
template < typename T, cgv::type::uint32_type N>
unsigned min_index(const fvec<T,N> &v)
{
	return (unsigned) (std::min_element(&v(0),&v(N-1)+1)-&v(0));
}

///return the index of the largest entry
template < typename T, cgv::type::uint32_type N>
unsigned max_index(const fvec<T,N> &v)
{
	return (unsigned) (std::max_element(&v(0),&v(N-1)+1)-&v(0));
}

///return the value of the largest entry
template < typename T, cgv::type::uint32_type N>
T max_value(const fvec<T,N> &v)
{
	return *(std::max_element(&v(0),&v(N-1)+1));
}

///linear interpolation returns (1-t)*v1 + t*v2
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> lerp(const fvec<T, N>& v1, const fvec<T, N>& v2, T t)
{
	return ((T)1 - t)*v1 + t * v2;
}

///linear interpolation returns (1-t)*v1 + t*v2
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> lerp(const fvec<T, N>& v1, const fvec<T, N>& v2, fvec<T, N> t)
{
	return (fvec<T, N>(1) - t)*v1 + t * v2;
}

	}
}


#include "vec.h"

namespace cgv {
	namespace math {

/// conversion to vector type
template <typename T, cgv::type::uint32_type N>
vec<T> fvec<T,N>::to_vec() const {
	vec<T> r;
	r.set_extern_data(N,const_cast<T*>(v));
	return r;
}

	}
}

/*
#include <cgv/utils/convert_string.h>
#include <cgv/type/info/type_name.h>

namespace cgv {
	namespace type {
		namespace info {

template <typename T, cgv::type::uint32_type N>
struct type_name<cgv::math::fvec<T,N> >
{
	static const char* get_name() { 
		static std::string name;
		if (name.empty()) {
			name = "fvec<";
			name += type_name<T>::get_name();
			name += ',';
			name += cgv::utils::to_string(N);
			name += '>';
		}
		return name.c_str(); 
	}
};
		}
	}
}
*/