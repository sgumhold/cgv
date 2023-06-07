#pragma once

// Make sure this is the first thing the compiler sees, while preventing warnings if
// it happened to already be defined by something else including this header
#ifndef _USE_MATH_DEFINES
	#define _USE_MATH_DEFINES 1
#endif
#include <array>
#include <limits>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <cgv/type/standard_types.h>
#include <cgv/math/functions.h>

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

	/// compile-time constant indicating the dimensionality of the vector
	enum { dims = N };

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
	/// construct from vector of one dimension higher by cutting of the highest dimension
	template <typename S>
	fvec(const fvec<S, N + 1>& fv) { for (unsigned i = 0; i < N; ++i) v[i] = (T)fv(i); }
	/// construct from std::array of same size
	fvec(const std::array<T, N>& arr) : fvec(N, arr.data()) {}
	///assign vector rhs, if vector and rhs have different sizes, vector has been resized to match the size of
	fvec & operator = (const fvec<T,N> &rhs) { if (this != &rhs) std::copy(rhs.v, rhs.v+N, v); return *this; }
	/// set all components of vector to constant value a
	fvec & operator = (const T &a) { std::fill(v, v+N, a); return *this; }	
	/// set to the contents of the given std::array with same size
	fvec & operator = (const std::array<T, N>& arr) { std::copy(arr.cbegin(), arr.cend(), v); return *this; }
	/// set the first two components
	void set(const T &x, const T &y) { v[0] = x; v[1] = y; }
	/// set the first three components
	void set(const T &x, const T &y, const T &z) { v[0] = x; v[1] = y; v[2] = z; }
	/// set the first four components
	void set(const T &x, const T &y, const T &z, const T &w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
	/// fill elements of vector with scalar v
	void fill(const T& a) { std::fill(v, v+N, a); }
	/// fill the vector with zeros
	void zeros() { fill((T)0); }
	/// fill the vector with zeros except for the last component, which will be set to one
	void zerosh() { std::fill(v, v+N-1, (T)0); v[N-1] = (T)1; }
	/// fill the vector with ones
	void ones() { fill((T)1); }
	/// convert to homogeneous version by adding a 1
	fvec<T,N+1> lift() const { fvec<T,N+1> h_v; (fvec<T,N>&)h_v=*this; h_v(N) = 1; return h_v; }
	/// creates a homogeneous zero-vector (yields same result as calling fvec<T,N-1>(0).lift() but is faster)
	static fvec<T,N> zeroh() {
		fvec<T,N> r;
		std::fill(r.v, r.v+N-1, (T)0); r.v[N-1] = (T)1;
		return r;
	}
	/// conversion to vector type
	vec<T> to_vec() const;
	/// conversion from vector
	static fvec<T, N> from_vec(const vec<T>&);
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
	fvec<T,N>& operator += (const fvec<S,N>& _v)  { for (unsigned i=0; i<N;++i) v[i] += T(_v(i)); return *this; }
	///in place vector subtraction
	template <typename S> 
	fvec<T,N>& operator -= (const fvec<S,N>& _v) { for (unsigned i=0; i<N;++i) v[i] -= T(_v(i)); return *this; }
	///in place componentwise vector multiplication
	template <typename S> 
	fvec<T,N>& operator *= (const fvec<S,N>& _v) { for (unsigned i=0; i<N;++i) v[i] *= T(_v(i)); return *this; }
	///in place componentwise vector division
	template <typename S> 
	fvec<T,N>& operator /= (const fvec<S,N>& _v) { for (unsigned i=0; i<N;++i) v[i] /= T(_v(i)); return *this; }
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

	///componentwise sign values
	void sign() {
		for (unsigned i = 0; i < N; i++)
			v[i] = cgv::math::sign(v[i]);
	}

	///componentwise sign values
	void step(const fvec<T, N>& r) {
		for (unsigned i = 0; i < N; i++)
			v[i] = cgv::math::step(v[i], r[i]);
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
	T normalize()  {
		T l     = length();
		T inv_l = (T)1.0/l;
		for(unsigned i = 0; i<N; i++)
			 operator()(i)=inv_l*operator()(i);
		return l;
	}

	/// normalize the vector if length is not zero using the L2-Norm and return the length
	T safe_normalize() {
		T l		= length();
		if(std::abs(l) < std::numeric_limits<T>::epsilon())
			return (T)0;
		T inv_l = (T)1.0 / l;
		for(unsigned i = 0; i < N; i++)
			operator()(i) = inv_l * operator()(i);
		return l;
	}
	//@}
};

/// return normalized vector
template<typename T, cgv::type::uint32_type N>
fvec<T,N> normalize(const fvec<T,N>& v) { fvec<T,N> w(v); w.normalize(); return w; }

/// return safely normalized vector
template<typename T, cgv::type::uint32_type N>
fvec<T, N> safe_normalize(const fvec<T, N>& v) { fvec<T, N> w(v); w.safe_normalize(); return w; }

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
	for (unsigned i = 0; i < N; ++i) {
		in >> v(i);
		if (in.fail() && i == 1) {
			for (unsigned i = 1; i < N; ++i)
				v(i) = v(0);
			break;
		}
	}
	return in;
}

///returns the product of a scalar s and vector v
template <typename T, cgv::type::uint32_type N>
fvec<T,N> operator * (const T& s, const fvec<T,N>& v) {	fvec<T,N> r = v; r *= s; return r; }

///returns a vector containing the quotients of a scalar s with each component of v
template <typename T, cgv::type::uint32_type N>
fvec<T,N> operator / (const T& s, const fvec<T,N>& v)
{ 
	T r;
	for (unsigned i=0;i<N;++i)
		r = s/v(i);
	return r;
}

///returns the dot product of vector v and w
template <typename T, typename S, cgv::type::uint32_type N>
inline T dot(const fvec<T,N>& v, const fvec<S,N>& w)
{ 
	T r = 0;
	for (unsigned i=0;i<N;++i)
		r += v(i)*w(i);
	return r;
}

///returns the dot product of N-dimensional vector v and (N+1)-dimensional position vector w, implicitly
///homogenizing the first operand
template <typename T, typename S, cgv::type::uint32_type N>
inline S dot_pos(const fvec<T,N>& v, const fvec<S,N+1>& w)
{ 
	T r = 0;
	for (unsigned i=0;i<N;++i)
		r += v(i)*w(i);
	return r+w(N);
}
///returns the dot product of (N+1)-dimensional vector v and N-dimensional position vector w, implicitly
///homogenizing the second operand
template <typename T, typename S, cgv::type::uint32_type N>
inline S dot_pos(const fvec<T,N+1>& v, const fvec<S,N>& w)
{ 
	T r = 0;
	for (unsigned i=0;i<N;++i)
		r += v(i)*w(i);
	return r+v(N);
}

///returns the dot product of N-dimensional vector v and (N+1)-dimensional direction vector w, implicitly
///homogenizing the first operand
template <typename T, typename S, cgv::type::uint32_type N>
inline S dot_dir(const fvec<T,N>& v, const fvec<S,N+1>& w)
{ 
	T r = 0;
	for (unsigned i=0;i<N;++i)
		r += v(i)*w(i);
	return r;
}
///returns the dot product of (N+1)-dimensional vector v and N-dimensional direction vector w, implicitly
///homogenizing the second operand
template <typename T, typename S, cgv::type::uint32_type N>
inline S dot_dir(const fvec<T,N+1>& v, const fvec<S,N>& w)
{ 
	T r = 0;
	for (unsigned i=0;i<N;++i)
		r += v(i)*w(i);
	return r;
}

///returns the length of vector v 
template <typename T, cgv::type::uint32_type N>
inline T length(const fvec<T, N>& v) { return sqrt(dot(v, v)); }

/// apply sign function component wise to vector
template <typename T, cgv::type::uint32_type N>
inline fvec<T, N> sign(const fvec<T, N>& v) { fvec<T, N> r(v); r.sign(); return r; }

/// apply step function component wise to vector
template <typename T, cgv::type::uint32_type N>
inline fvec<T, N> step(const fvec<T, N>& a, const fvec<T, N>& b) { fvec<T, N> r(a); r.step(b); return r; }

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
const fvec<T, N> lerp(const fvec<T, N>& v1, const fvec<T, N>& v2, const fvec<T, N>& t)
{
	return (fvec<T, N>(1) - t)*v1 + t * v2;
}

/// spherical linear interpolation
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> slerp(const fvec<T, N> &v0, const fvec<T, N> &v1, T t) {
	T dotv0v1 = dot(v0, v1);
	// clamp between [-1,1]
	if (dotv0v1 < -1)
		dotv0v1 = -1;

	if (dotv0v1 > 1)
		dotv0v1 = 1;

	T theta = acos(dotv0v1) * t;
	auto v2 = normalize(v1 - (dotv0v1)*v0);
	return T(cos(theta)) * v0 + T(sin(theta)) * v2;
}

///return a vector containing the minimum value of each component of v and the scalar t
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> min(const fvec<T, N>& v, T t) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c(i) = std::min(v(i), t);
	return c;
}

///return a vector containing the component-wise minimum value
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> min(const fvec<T, N>& v, const fvec<T, N>& t) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c(i) = std::min(v(i), t(i));
	return c;
}

///return a vector containing the maximum value of each component of v and the scalar t
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> max(const fvec<T, N>& v, T t) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c(i) = std::max(v(i), t);
	return c;
}

///return a vector containing the component-wise maximum value
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> max(const fvec<T, N>& v, const fvec<T, N>& t) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c(i) = std::max(v(i), t(i));
	return c;
}

///clamp the components to the given range
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> clamp(const fvec<T, N>& v, T l, T r) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c(i) = cgv::math::clamp(v(i), l, r);
	return c;
}

///clamp the components to the given range
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> clamp(const fvec<T, N>& v, const fvec<T, N>& vl, const fvec<T, N>& vr)
{
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c(i) = cgv::math::clamp(v(i), vl(i), vr(i));
	return c;
}

///shortcut to clamp the components to [0,1]
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> saturate(const fvec<T, N>& v) {
	return clamp(v, T(0), T(1));
}

///pow function for fvec type
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> pow(const fvec<T, N>& v, T e) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c(i) = std::pow(v(i), e);
	return c;
}

///pow function for fvec type
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> pow(const fvec<T, N>& v, const fvec<T, N>& e) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c(i) = std::pow(v(i), e(i));
	return c;
}

/// returns an orthogonal vector to v (only defined for 2d and 3d case)
template <typename T, cgv::type::uint32_type N>
fvec<T, N> ortho(const fvec<T, N>& v) = delete;

/// returns an orthogonal vector to v
template <typename T>
fvec<T, 2> ortho(const fvec<T, 2>& v) {
	return fvec<T, 2>(v.y(), -v.x());
}

/// returns an orthogonal vector to v
template <typename T>
fvec<T, 3> ortho(const fvec<T, 3>& v) {
	return std::abs(v.x()) > std::abs(v.z()) ? fvec<T, 3>(-v.y(), v.x(), T(0)) : fvec<T, 3>(T(0), -v.z(), v.y());
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

/// conversion from vector
template <typename T, cgv::type::uint32_type N>
fvec<T, N> fvec<T, N>::from_vec(const vec<T>& v)
{
	return fvec<T, N>(std::min(N, v.dim()), &v[0]);
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
