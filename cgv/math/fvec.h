#pragma once

// Make sure this is the first thing the compiler sees, while preventing warnings if
// it happened to already be defined by something else including this header
#ifndef _USE_MATH_DEFINES
	#define _USE_MATH_DEFINES 1
#endif
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <cgv/type/standard_types.h>
#include <cgv/math/functions.h>

namespace cgv {
namespace math {

template <typename T> class vec;

/** A vector with zero based index.*/
template <typename T, cgv::type::uint32_type N>
class fvec {
protected:
	// the components of the vector
	T v[N];

public:
	//@name type definitions
	//@{
	///
	typedef T value_type;
	///
	typedef T& reference;
	///
	typedef const T& const_reference;
	///
	typedef std::size_t size_type;
	///
	typedef std::ptrdiff_t difference_type;
	///
	typedef T* pointer;
	///
	typedef const T* const_pointer;
	///
	typedef T* iterator;
	///
	typedef const T* const_iterator;
	///
	typedef std::reverse_iterator<iterator> reverse_iterator;
	///
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	/// compile-time constant indicating the dimensionality of the vector
	enum { dims = N };
	//@}

	//@name construction and assignment
	//@{
	/// create an uninitialized vector
	fvec() {}
	/// create a vector where all N components are initialized to the constant value a
	fvec(const T& a) { std::fill(v, v + N, a); }
	/// construct and initialize the first two components to the given values
	fvec(const T& x, const T& y) { set(x, y); }
	/// construct and initialize the first three components to the given values
	fvec(const T& x, const T& y, const T& z) { set(x, y, z); }
	/// construct and initialize the first four components to the given values
	fvec(const T& x, const T& y, const T& z, const T& w) { set(x, y, z, w); }
	/// create a vector from an array of size n; if n is less than N the remaining N-n components are set to zero
	fvec(cgv::type::uint32_type n, const T* a) {
		cgv::type::uint32_type i, min_n = n < N ? n : N;
		memmove(v, a, min_n * sizeof(T));
		for(i = min_n; i < N; ++i) v[i] = T(0);
	}
	/// create a vector from an array of size n of a different type; if n is less than N the remaining N-n components are set to zero
	template <typename S>
	fvec(cgv::type::uint32_type n, const S* a) {
		cgv::type::uint32_type i, min_n = n < N ? n : N;
		for(i = 0; i < min_n; ++i) v[i] = static_cast<T>(a[i]);
		for(; i < N; ++i) v[i] = T(0);
	}
	/// construct by copy from a vector of a different type
	template <typename S>
	fvec(const fvec<S, N>& other) { for(unsigned i = 0; i < N; ++i) v[i] = static_cast<T>(other[i]); }
	/// construct from a vector of one dimension less plus a scalar
	template <typename S1, typename S2>
	fvec(const fvec<S1, N - 1>& other, S2 s) { for(unsigned i = 0; i < N - 1; ++i) v[i] = static_cast<T>(other[i]); v[N - 1] = static_cast<T>(s); }
	/// construct from vector of one dimension higher by dropping the highest dimension
	template <typename S>
	fvec(const fvec<S, N + 1>& other) { for(unsigned i = 0; i < N; ++i) v[i] = static_cast<T>(other[i]); }
	/// construct from std::array of same size
	fvec(const std::array<T, N>& arr) : fvec(N, arr.data()) {}
	/// set to the contents of the given std::array with same size
	void assign(const std::array<T, N>& arr) { std::copy(arr.cbegin(), arr.cend(), v); }
	/// set the first two components
	void set(const T& x, const T& y) { v[0] = x; v[1] = y; }
	/// set the first three components
	void set(const T& x, const T& y, const T& z) { v[0] = x; v[1] = y; v[2] = z; }
	/// set the first four components
	void set(const T& x, const T& y, const T& z, const T& w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
	/// fill the vector with constant value x
	void fill(const T& x) { std::fill(v, v + N, x); }
	/// fill the vector with zeros
	void zeros() { fill(T(0)); }
	/// fill the vector with zeros except for the last component, which will be set to one
	void zerosh() { std::fill(v, v + N - 1, T(0)); v[N - 1] = T(1); }
	/// fill the vector with ones
	void ones() { fill(T(1)); }
	/// convert to homogeneous version by adding a 1
	fvec<T, N + 1> lift() const { fvec<T, N + 1> h; (fvec<T, N>&)h = *this; h[N] = T(1); return h; }
	/// constuct a homogeneous zero-vector (yields same result as calling fvec<T, N-1>(0).lift() but is faster)
	static fvec<T, N> zeroh() { fvec<T, N> r; std::fill(r.v, r.v + N - 1, T(0)); r[N - 1] = T(1); return r; }
	/// conversion to vector type
	vec<T> to_vec() const;
	/// conversion from vector
	static fvec<T, N> from_vec(const vec<T>&);
	//@}

	//@name access to components
	//@{
	///return number of components
	static cgv::type::uint32_type size() { return N; }
	/// return first component
	T& x() { return v[0]; }
	/// return first component
	const T& x() const { return v[0]; }
	/// return second component
	T& y() { return v[1]; }
	/// return second component
	const T& y() const { return v[1]; }
	/// return third component
	T& z() { return v[2]; }
	/// return third component
	const T& z() const { return v[2]; }
	/// return fourth component
	T& w() { return v[3]; }
	/// return fourth component
	const T& w() const { return v[3]; }
	/// return a reference to the component at specified index i
	T& operator[](int i) { return v[i]; }
	/// return a reference to the component at specified index i
	const T& operator[](int i) const { return v[i]; }
	/// return a reference to the component at specified index i
	T& operator()(int i) { return v[i]; }
	/// return a reference to the component at specified index i
	const T& operator()(int i) const { return v[i]; }
	/// return a pointer to the underlying array serving as component storage
	T* data() { return v; }
	/// return a pointer to the underlying array serving as component storage
	const T* data() const { return v; }
	//@}

	//@name iterators
	//@{
	/// return an iterator to the first component of *this
	iterator begin() { return v; }
	/// return an iterator past the last component of *this
	iterator end() { return v + N; }
	/// return an iterator to the first component of *this
	const_iterator begin() const { return v; }
	/// return an iterator past the last component of *this
	const_iterator end() const { return v + N; }
	/// return a reverse iterator to the first component of the reversed *this that corresponds to the last component of the non-reversed *this
	reverse_iterator rbegin() { return reverse_iterator(end()); }
	/// return a reverse iterator past the last component of the reversed *this that corresponds to the component preceding the first component of the non-reversed *this
	reverse_iterator rend() { return reverse_iterator(begin()); }
	/// return a reverse iterator to the first component of the reversed *this that corresponds to the last component of the non-reversed *this
	const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
	/// return a reverse iterator past the last component of the reversed *this that corresponds to the component preceding the first component of the non-reversed *this
	const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
	//@}

	//@name operators
	//@{
	/// in place addition of a scalar s
	fvec<T, N>& operator += (const T& s) { for(unsigned i = 0; i < N; ++i) v[i] += s; return *this; }
	/// in place subtraction by scalar s
	fvec<T, N>& operator -= (const T& s) { for(unsigned i = 0; i < N; ++i) v[i] -= s; return *this; }
	/// in place multiplication with s
	fvec<T, N>& operator *= (const T& s) { for(unsigned i = 0; i < N; ++i) v[i] *= s; return *this; }
	///in place division by scalar s
	fvec<T, N>& operator /= (const T& s) { for(unsigned i = 0; i < N; ++i) v[i] /= s; return *this; }
	/// in place vector addition
	template <typename S>
	fvec<T, N>& operator += (const fvec<S, N>& v) { for(unsigned i = 0; i < N; ++i) this->v[i] += static_cast<T>(v[i]); return *this; }
	/// in place vector subtraction
	template <typename S>
	fvec<T, N>& operator -= (const fvec<S, N>& v) { for(unsigned i = 0; i < N; ++i) this->v[i] -= static_cast<T>(v[i]); return *this; }
	/// in place componentwise vector multiplication
	template <typename S>
	fvec<T, N>& operator *= (const fvec<S, N>& v) { for(unsigned i = 0; i < N; ++i) this->v[i] *= static_cast<T>(v[i]); return *this; }
	/// in place componentwise vector division
	template <typename S>
	fvec<T, N>& operator /= (const fvec<S, N>& v) { for(unsigned i = 0; i < N; ++i) this->v[i] /= static_cast<T>(v[i]); return *this; }
	/// vector addition
	template <typename S>
	fvec<T, N> operator + (const fvec<S, N>& v) const { fvec<T, N> r = *this; r += v; return r; }
	/// vector subtraction
	template <typename S>
	fvec<T, N> operator - (const fvec<S, N>& v) const { fvec<T, N> r = *this; r -= v; return r; }
	/// componentwise vector multiplication
	template <typename S>
	fvec<T, N> operator * (const fvec<S, N>& v) const { fvec<T, N> r = *this; r *= v; return r; }
	/// componentwise vector division
	template <typename S>
	fvec<T, N> operator / (const fvec<S, N>& v) const { fvec<T, N> r = *this; r /= v; return r; }
	/// componentwise addition of scalar
	fvec<T, N> operator + (const T& s) const { fvec<T, N> r = *this; r += s; return r; }
	/// componentwise subtraction of scalar
	fvec<T, N> operator - (const T& s) const { fvec<T, N> r = *this; r -= s; return r; }
	/// multiplication with scalar s
	fvec<T, N> operator * (const T& s) const { fvec<T, N> r = *this; r *= s; return r; }
	/// divide vector by scalar s
	fvec<T, N> operator / (const T& s) const { fvec<T, N> r = *this; r /= s; return r; }

	/// negate the vector
	fvec<T, N> operator - () const { fvec<T, N> r; for(unsigned i = 0; i < N; ++i) r[i] = -v[i]; return r; }

	/// test for equality
	template <typename S>
	bool operator == (const fvec<S, N>& v) const {
		for(unsigned i = 0; i < N; ++i)
			if(operator[](i) != static_cast<T>(v[i])) return false;
		return true;
	}
	/// test for inequality
	template <typename S>
	bool operator != (const fvec<S, N>& v) const {
		for(unsigned i = 0; i < N; ++i)
			if(operator[](i) != static_cast<T>(v[i])) return true;
		return false;
	}
	//@}

	//@name functions
	//@{
	/// square length of vector
	T sqr_length() const {
		T l = 0;
		for(unsigned i = 0; i < N; ++i)
			l += v[i] * v[i];
		return l;
	}
	/// length of the vector L2-Norm
	T length() const {
		return std::sqrt(sqr_length());
	}
	/// componentwise sign values
	void sign() {
		for(unsigned i = 0; i < N; ++i)
			v[i] = cgv::math::sign(v[i]);
	}
	/// componentwise sign values
	void step(const fvec<T, N>& r) {
		for(unsigned i = 0; i < N; ++i)
			v[i] = cgv::math::step(v[i], r[i]);
	}
	/// componentwise absolute values
	void abs() {
		if(std::numeric_limits<T>::is_signed) {
			for(unsigned i = 0; i < N; ++i)
				v[i] = std::abs(v[i]);
		}
	}
	/// ceil componentwise
	void ceil() {
		for(unsigned i = 0; i < N; ++i)
			v[i] = std::ceil(v[i]);
	}
	/// floor componentwise
	void floor() {
		for(unsigned i = 0; i < N; ++i)
			v[i] = std::floor(v[i]);
	}
	/// round componentwise
	void round() {
		for(unsigned i = 0; i < N; ++i)
			v[i] = std::floor(v[i] + 0.5);
	}
	/// normalize the vector using the L2-Norm and return the length
	T normalize() {
		T l = length();
		T inv_l = T(1) / l;
		for(unsigned i = 0; i < N; ++i)
			v[i] *= inv_l;
		return l;
	}
	/// normalize the vector using the L2-Norm and return the length; if length is zero the vector remains unchanged
	T safe_normalize() {
		T l = length();
		if(std::abs(l) < std::numeric_limits<T>::epsilon())
			return T(0);
		T inv_l = T(1) / l;
		for(unsigned i = 0; i < N; ++i)
			v[i] *= inv_l;
		return l;
	}
	//@}
};

/// This symbol is defined when @ref cgv::math::fvec exists in the current compilation unit
#define CGV_MATH_FVEC_DECLARED

/// return normalized vector
template<typename T, cgv::type::uint32_type N>
fvec<T, N> normalize(const fvec<T, N>& v) { fvec<T, N> w(v); w.normalize(); return w; }

/// return safely normalized vector
template<typename T, cgv::type::uint32_type N>
fvec<T, N> safe_normalize(const fvec<T, N>& v) { fvec<T, N> w(v); w.safe_normalize(); return w; }

/// output vector to a stream
template<typename T, cgv::type::uint32_type N>
std::ostream& operator<<(std::ostream& out, const fvec<T, N>& v) {
	for(unsigned i = 0; i < N - 1; ++i)
		out << v[i] << " ";
	out << v[N - 1];
	return out;

}

/// read vector from a stream
template<typename T, cgv::type::uint32_type N>
std::istream& operator>>(std::istream& in, fvec<T, N>& v) {
	for(unsigned i = 0; i < N; ++i) {
		in >> v[i];
		if(in.fail() && i == 1) {
			for(unsigned i = 1; i < N; ++i)
				v[i] = v[0];
			break;
		}
	}
	return in;
}

/// vector to string
template<typename T, cgv::type::uint32_type N>
std::string to_string(const fvec<T, N>& v) {
	std::ostringstream ss;
	ss << v;
	return ss.str();
}

/// vector from string
template<typename T, cgv::type::uint32_type N>
bool from_string(const std::string& s, fvec<T, N>& v) {
	std::istringstream iss(s);
	iss >> v;
	return !iss.fail();
}

/// return the product of a scalar s and vector v
template <typename T, cgv::type::uint32_type N>
fvec<T, N> operator * (const T& s, const fvec<T, N>& v) { fvec<T, N> r = v; r *= s; return r; }

/// return a vector containing the quotients of a scalar s with each component of v
template <typename T, cgv::type::uint32_type N>
fvec<T, N> operator / (const T& s, const fvec<T, N>& v) {
	fvec<T, N> r;
	for(unsigned i = 0; i < N; ++i)
		r[i] = s / v[i];
	return r;
}

/// return the dot product of vector v and w
template <typename T, typename S, cgv::type::uint32_type N>
inline T dot(const fvec<T, N>& v, const fvec<S, N>& w) {
	T r = 0;
	for(unsigned i = 0; i < N; ++i)
		r += static_cast<T>(v[i] * w[i]);
	return r;
}

/// return the dot product of N-dimensional vector v and (N+1)-dimensional position vector w, implicitly
/// homogenizing the first operand
template <typename T, typename S, cgv::type::uint32_type N>
inline S dot_pos(const fvec<T, N>& v, const fvec<S, N + 1>& w) {
	T r = 0;
	for(unsigned i = 0; i < N; ++i)
		r += v[i] * w[i];
	return r + w[N];
}
/// return the dot product of (N+1)-dimensional vector v and N-dimensional position vector w, implicitly
/// homogenizing the second operand
template <typename T, typename S, cgv::type::uint32_type N>
inline S dot_pos(const fvec<T, N + 1>& v, const fvec<S, N>& w) {
	T r = 0;
	for(unsigned i = 0; i < N; ++i)
		r += v[i] * w[i];
	return r + v[N];
}

/// return the dot product of N-dimensional vector v and (N+1)-dimensional direction vector w, implicitly
/// homogenizing the first operand
template <typename T, typename S, cgv::type::uint32_type N>
inline S dot_dir(const fvec<T, N>& v, const fvec<S, N + 1>& w) {
	T r = 0;
	for(unsigned i = 0; i < N; ++i)
		r += v[i] * w[i];
	return r;
}
/// return the dot product of (N+1)-dimensional vector v and N-dimensional direction vector w, implicitly
/// homogenizing the second operand
template <typename T, typename S, cgv::type::uint32_type N>
inline S dot_dir(const fvec<T, N + 1>& v, const fvec<S, N>& w) {
	T r = 0;
	for(unsigned i = 0; i < N; ++i)
		r += v[i] * w[i];
	return r;
}

/// return the cross product of vector v and w
template < typename T, cgv::type::uint32_type N>
inline fvec<T, N> cross(const fvec<T, N>& v, const fvec<T, N>& w) {
	fvec<T, N> r(3);
	r[0] = v[1] * w[2] - v[2] * w[1];
	r[1] = v[2] * w[0] - v[0] * w[2];
	r[2] = v[0] * w[1] - v[1] * w[0];
	return r;
}

/// return the squared length of vector v 
template <typename T, cgv::type::uint32_type N>
inline T sqr_length(const fvec<T, N>& v) { return dot(v, v); }

/// return the length of vector v 
template <typename T, cgv::type::uint32_type N>
inline T length(const fvec<T, N>& v) { return std::sqrt(dot(v, v)); }

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

///returns homogenized vector
template < typename T, cgv::type::uint32_type N>
inline fvec<T, N + 1> hom(const fvec<T, N>& v) {
	fvec<T, N + 1> h;
	for(unsigned i = 0; i < N; ++i)
		h[i] = v[i];
	h[N] = 1;
	return h;
}

/// return the smalles component
template < typename T, cgv::type::uint32_type N>
T min_value(const fvec<T, N>& v) {
	return *(std::min_element(v.begin(), v.end()));
}

/// return the index of the smallest component
template < typename T, cgv::type::uint32_type N>
unsigned min_index(const fvec<T, N>& v) {
	return static_cast<unsigned>(std::distance(v.begin(), std::min_element(v.begin(), v.end())));
}

/// return the value of the largest component
template < typename T, cgv::type::uint32_type N>
T max_value(const fvec<T, N>& v) {
	return *(std::max_element(v.begin(), v.end()));
}

/// return the index of the largest component
template < typename T, cgv::type::uint32_type N>
unsigned max_index(const fvec<T, N>& v) {
	return static_cast<unsigned>(std::distance(v.begin(), std::max_element(v.begin(), v.end())));
}

/// linear interpolation between v0 and v1; returns (1-t)*v0 + t*v1
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> lerp(const fvec<T, N>& v0, const fvec<T, N>& v1, T t) {
	return (T(1) - t) * v0 + t * v1;
}

/// linear interpolation between v0 and v1; returns (1-t)*v0 + t*v1
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> lerp(const fvec<T, N>& v0, const fvec<T, N>& v1, const fvec<T, N>& t) {
	return (fvec<T, N>(1) - t) * v0 + t * v1;
}

/// spherical linear interpolation between v0 and v1
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> slerp(const fvec<T, N>& v0, const fvec<T, N>& v1, T t) {
	T dotv0v1 = dot(v0, v1);
	// clamp between [-1,1]
	if(dotv0v1 < -1)
		dotv0v1 = -1;

	if(dotv0v1 > 1)
		dotv0v1 = 1;

	T theta = std::acos(dotv0v1) * t;
	auto v2 = normalize(v1 - (dotv0v1)*v0);
	return T(std::cos(theta)) * v0 + T(std::sin(theta)) * v2;
}

/// return a vector containing the componentwise minimum value of v and the scalar s
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> min(const fvec<T, N>& v, T s) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c(i) = std::min(v[i], s);
	return c;
}

/// return a vector containing the componentwise minimum value of v0 and v1
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> min(const fvec<T, N>& v0, const fvec<T, N>& v1) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c[i] = std::min(v0[i], v1[i]);
	return c;
}

/// return a vector containing the componentwise maximum value of v and the scalar s
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> max(const fvec<T, N>& v, T s) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c[i] = std::max(v[i], s);
	return c;
}

/// return a vector containing the componentwise maximum value of v0 and v1
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> max(const fvec<T, N>& v0, const fvec<T, N>& v1) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c[i] = std::max(v0[i], v1[i]);
	return c;
}

/// clamp the components to the given range
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> clamp(const fvec<T, N>& v, T l, T r) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c[i] = cgv::math::clamp(v[i], l, r);
	return c;
}

/// clamp the components to the given range
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> clamp(const fvec<T, N>& v, const fvec<T, N>& vl, const fvec<T, N>& vr) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c[i] = cgv::math::clamp(v[i], vl[i], vr[i]);
	return c;
}

/// shortcut to clamp the components to [0,1]
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> saturate(const fvec<T, N>& v) {
	return clamp(v, T(0), T(1));
}

/// pow function for fvec type
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> pow(const fvec<T, N>& v, T e) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c[i] = std::pow(v[i], e);
	return c;
}

/// pow function for fvec type
template <typename T, cgv::type::uint32_type N>
const fvec<T, N> pow(const fvec<T, N>& v, const fvec<T, N>& e) {
	fvec<T, N> c;
	for(unsigned i = 0; i < N; ++i)
		c[i] = std::pow(v[i], e[i]);
	return c;
}

/// return an orthogonal vector to v (only defined for 2d and 3d case)
template <typename T, cgv::type::uint32_type N>
fvec<T, N> ortho(const fvec<T, N>& v) = delete;

/// return an orthogonal vector to v (not normalized; rotated counter-clockwise)
template <typename T>
fvec<T, 2> ortho(const fvec<T, 2>& v) {
	return fvec<T, 2>(-v.y(), v.x());
}

/// return an orthogonal vector to v (not normalized)
template <typename T>
fvec<T, 3> ortho(const fvec<T, 3>& v) {
	return std::abs(v.x()) > std::abs(v.z()) ? fvec<T, 3>(-v.y(), v.x(), T(0)) : fvec<T, 3>(T(0), -v.z(), v.y());
}

} // namespace math

/// @name Predefined Types
/// @{

/// declare type of 2d boolean vectors
typedef cgv::math::fvec<bool, 2> bvec2;
/// declare type of 3d boolean vectors
typedef cgv::math::fvec<bool, 3> bvec3;
/// declare type of 4d boolean vectors
typedef cgv::math::fvec<bool, 4> bvec4;

/// declare type of 2d single precision floating point vectors
typedef cgv::math::fvec<float, 2> vec2;
/// declare type of 3d single precision floating point vectors
typedef cgv::math::fvec<float, 3> vec3;
/// declare type of 4d single precision floating point vectors (used for homogeneous coordinates)
typedef cgv::math::fvec<float, 4> vec4;

/// declare type of 2d double precision floating point vectors
typedef cgv::math::fvec<double, 2> dvec2;
/// declare type of 3d double precision floating point vectors
typedef cgv::math::fvec<double, 3> dvec3;
/// declare type of 4d double precision floating point vectors (used for homogeneous coordinates)
typedef cgv::math::fvec<double, 4> dvec4;

/// declare type of 2d 16 bit integer vectors
typedef cgv::math::fvec<int16_t, 2> svec2;
/// declare type of 3d 16 bit integer vectors
typedef cgv::math::fvec<int16_t, 3> svec3;
/// declare type of 4d 16 bit integer vectors
typedef cgv::math::fvec<int16_t, 4> svec4;
/// declare type of 2d 16 bit unsigned integer vectors
typedef cgv::math::fvec<uint16_t, 2> usvec2;
/// declare type of 3d 16 bit unsigned integer vectors
typedef cgv::math::fvec<uint16_t, 3> usvec3;
/// declare type of 4d 16 bit unsigned integer vectors
typedef cgv::math::fvec<uint16_t, 4> usvec4;

/// declare type of 2d 32 bit integer vectors
typedef cgv::math::fvec<int32_t, 2> ivec2;
/// declare type of 3d 32 bit integer vectors
typedef cgv::math::fvec<int32_t, 3> ivec3;
/// declare type of 4d 32 bit integer vectors
typedef cgv::math::fvec<int32_t, 4> ivec4;
/// declare type of 2d 32 bit unsigned integer vectors
typedef cgv::math::fvec<uint32_t, 2> uvec2;
/// declare type of 3d 32 bit unsigned integer vectors
typedef cgv::math::fvec<uint32_t, 3> uvec3;
/// declare type of 4d 32 bit unsigned integer vectors
typedef cgv::math::fvec<uint32_t, 4> uvec4;

/// declare type of 2d 64 bit integer vectors
typedef cgv::math::fvec<int64_t, 2> lvec2;
/// declare type of 3d 64 bit integer vectors
typedef cgv::math::fvec<int64_t, 3> lvec3;
/// declare type of 4d 64 bit integer vectors
typedef cgv::math::fvec<int64_t, 4> lvec4;
/// declare type of 2d 64 bit unsigned integer vectors
typedef cgv::math::fvec<uint64_t, 2> ulvec2;
/// declare type of 3d 64 bit unsigned integer vectors
typedef cgv::math::fvec<uint64_t, 3> ulvec3;
/// declare type of 4d 64 bit unsigned integer vectors
typedef cgv::math::fvec<uint64_t, 4> ulvec4;

/// @}

} // namespace cgv

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

} // namespace math
} // namespace cgv


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
