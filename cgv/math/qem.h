#pragma once

#include "vec.h"
#include "mat.h"
#include "inv.h"

namespace cgv {
	namespace math { 


/// dimension independent implementation of quadric error metrics
template <typename T>
class qem : public vec<T>
{
public:
	/// standard constructor initializes qem based on dimension
	qem(int d = -1) : vec<T>((d+1)*(d+2)/2)
	{
		if (d>=0)
			this->zeros();
	}
	/// construct from point and normal
	qem(const vec<T>& p, const vec<T>& n) : vec<T>((p.size()+1)*(p.size()+2)/2)
	{
		set(n, -dot(p,n));
	}
	/// construct from normal and distance to origin
	qem(const vec<T>& n, T d) : vec<T>((n.size()+1)*(n.size()+2)/2)
	{
		set(n,d);
	}
	/// set from normal and distance to origin
	void set(const vec<T>& n, T d)
	{
		this->first() = d*d;
		unsigned int i,j,k=n.size()+1;
		for (i=0;i<n.size(); ++i) {
			(*this)(i+1) = d*n(i);
			for (j=i;j<n.size(); ++j, ++k)
				(*this)(k) = n(i)*n(j);
		}
	}
	///number of elements
	unsigned dim() const
	{
		return (unsigned int)sqrt(2.0*this->size())-1;
	}
	///assignment of  a vector v
	qem<T>& operator = (const qem<T>& v) 
	{ 
		*static_cast<vec<T>*>(this) = v;
		return *this; 
	}
	/// return the scalar part of the qem
	const T& scalar_part() const
	{
		return this->first();
	}
	/// return the vector part of the qem
	vec<T> vector_part() const
	{
		vec<T> v(dim());
		for (unsigned int i=0; i<v.size(); ++i)
			v(i) = (*this)(i+1);
		return v;
	}
	/// return matrix part
	mat<T> matrix_part() const
	{
		unsigned int d = dim();
		mat<T> m(d,d);
		unsigned int i,j,k=d+1;
		for (i=0; i<d; ++i)
			for (j=i; j<d; ++j,++k)
				m(i,j) = m(j,i) = (*this)(k);
		return m;
	}
	/// evaluate the quadric error metric at given location
	T evaluate(const vec<T>& p) const
	{
		return dot(matrix_part()*p+2.0*vector_part(),p)+scalar_part();
	}
	static bool inside(const vec<T>& p, const vec<T>& minp, const vec<T>& maxp)
	{
		for (unsigned int i=0; i<p.size(); ++i) {
			if (p(i) < minp(i))
				return false;
			if (p(i) > maxp(i))
				return false;
		}
		return true;
	}
	/** compute point that minimizes distance to qem and is inside the sphere of radius max_distance 
	    around p_ref. If max_distance is -1, no sphere inclusion test is performed.
	    relative_epsilon gives the absolute value of the fraction betweenan eigenvalue and the
		 largest eigenvalue before it is set to zero. epsilon is a global limit on the absolute
		 value of a singular value before accepted as non zero. */
	vec<T> minarg(const vec<T>& p_ref, T relative_epsilon, T max_distance = -1, T epsilon = 1e-10) const
	{
		unsigned int d = p_ref.size();
		assert(d == dim());
		T max_distance2 = max_distance*max_distance;
		mat<T> U,V,A = matrix_part();
		diag_mat<T> W,iW;
		svd(A,U,W,V);
		U.transpose();
		iW = inv(W);
		vec<T> y_solve = -(inv(W)*(U*vector_part()));
		vec<T> y_ref   = transpose(V)*p_ref;
		vec<T> y(3);
		for (unsigned int i = d; i > 0; ) {
			--i;
			if (fabs(W(i)) > epsilon && fabs(W(i)*iW(0)) > relative_epsilon) {
				unsigned int j;
				for (j = 0; j <= i; ++j)
					y(j) = y_solve(j);
				for (; j < d; ++j)
					y(j) = y_ref(j);
				vec<T> p = V*y;
				if (max_distance != -1 && (p-p_ref).sqr_length() <= max_distance2)
					return p;
			}
		}
		return p_ref;
	}
	/// in place qem addition
	template <typename S> 
	qem<T>& operator += (const qem<S>& v) 
	{ 
		assert(this->size() == v.size());
		for (unsigned i=0;i<this->size();++i) (*this)(i) += v(i); return *this;
	}

	///in place qem subtraction
	template <typename S> 
	qem<T>& operator -= (const qem<S>& v) 
	{
		assert(this->size() == v.size());
		for (unsigned i=0;i<this->size();++i) (*this)(i) -= v(i); return *this;
	}
	
	///qem addition
	template <typename S> 
	const qem<T>  operator +  (const qem<S>& v) const 
	{ 
		vec<T> r = *this; r += v; return r; 
	}

	///qem subtraction
	template <typename S> 
	qem<T>  operator -  (const qem<S>& v) const 
	{
		 qem<T> r = *this; r -= v; return r; 
	}

	///negates the qem
	qem<T>  operator-(void) const 
	{
		qem<T> r=(*this);
		r=(T)(-1)*r;
		return r; 
	}

	///multiplication with scalar s
	qem<T>  operator * (const T& s) const 
	{
		 qem<T> r = *this; r *= s; return r; 
	}

	
	///divides vector by scalar s
	qem<T>  operator / (const T& s)const 
	{ 
		qem<T> r = *this;
		r /= s;
		return r; 
	}
	
	///resize the vector
	void resize(unsigned d)
	{
		vec<T>::resize((d+1)*(d+2)/2);
	}

	///test for equality
	template <typename S> 
	bool operator == (const qem<S>& v) const
	{ 
		for (unsigned i=0;i<this->size();++i)
			if((*this)(i) != (T)v(i)) return false;
		return true; 
	}

	///test for inequality
	template <typename S> 
	bool operator != (const qem<S>& v) const
	{ 
		for (unsigned i=0;i<this->size();++i)
			if((*this)(i) != (T)v(i)) return true;
		return false; 
	}

};

///returns the product of a scalar s and qem v
template <typename T>
const qem<T> operator * (const T& s, const qem<T>& v) 
{
	qem<T> r = v; r *= s; return r; 
}

	}
}


