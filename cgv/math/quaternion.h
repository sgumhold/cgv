#pragma once

#include "fvec.h"
#include "fmat.h"
#include "functions.h"

namespace cgv {
	namespace math {

#define EPSILON 1e-6

/** implements a quaternion.*/
template <typename T>
class quaternion : public fvec<T,4>
{
public:
	/**@name{\large a) preliminaries}*/
	//@{
	/// base class type
	typedef fvec<T,4> base_type;
	/// coordinate type
	typedef T coord_type;
	/// enumeration of the three coordinate axes
	enum AxisEnum { X_AXIS, Y_AXIS, Z_AXIS };
	/// type of 3d axis
	typedef fvec<T,3> vec_type;
	/// type of 3x3 matrix
	typedef fmat<T, 3, 3> mat_type;
	/// type of 4x4 matrix
	typedef fmat<T, 4, 4> hmat_type;
	//@}

	/**@name{\large b) construction}*/
	//@{
	/// standard constructor initializes to unit quaternion
	quaternion() : fvec<T,4>(T(0),T(0),T(0),T(1)) {}
	/// copy constructor
	quaternion(const quaternion<T>& quat) : fvec<T,4>(quat) {}
	/// copy constructor with type conversion
	template <typename S>
	quaternion(const quaternion<S>& q) : fvec<T,4>(q) {}
	/// assignement operator
	quaternion<T>& operator=(const quaternion<T>& quat) {
		base_type::operator = (quat);
		return *this;
	}
	/// construct quaternion from coordinate axis and rotation angle
	quaternion(AxisEnum axis, coord_type angle)                      { set(axis, angle); }
	/// construct quaternion from axis and rotation angle
	quaternion(const vec_type& axis, coord_type angle)               { set(axis, angle); }
	/// construct quaternion from 3x3 rotation matrix
	quaternion(const mat_type& matrix)                             { set(matrix); }
	/// construct quaternion directly
	quaternion(coord_type w,coord_type x, coord_type y,coord_type z) { set(w,x,y,z); }
	/// construct quaternion from real part and vector
	quaternion(coord_type re, const vec_type& im)                    { set(re, im); }
	base_type& vec() { return *this; }
	const base_type& vec() const { return *this; }
	//@}
	/**@name{\large c) initialization}*/
	//@{
	/// initialize quaternion from coordinate axis and rotation angle
	void set(AxisEnum axis, coord_type angle)
	{
		vec_type v(0,0,0);
		v((int)axis) = 1;
		set(v, angle);
	}
	/// initialize quaternion from axis and rotation angle
	void set(const vec_type& axis, coord_type angle)
	{
		angle *= (coord_type)0.5;
		set(cos(angle), coord_type(sin(angle))*axis);
	}
	/// setter from quaternion
	void set(const quaternion<T>& quat) { *this = quat; }
	/// initialize quaternion from 3x3 rotation matrix
	void set(const mat_type& M)
	{
		this->w() = sqrt(plus(T(0.25)*( M(0, 0) + M(1, 1) + M(2, 2) + T(1))));
		this->x() = sqrt(plus(T(0.25)*( M(0, 0) - M(1, 1) - M(2, 2) + T(1))));
		this->y() = sqrt(plus(T(0.25)*(-M(0, 0) + M(1, 1) - M(2, 2) + T(1))));
		this->z() = sqrt(plus(T(0.25)*(-M(0, 0) - M(1, 1) + M(2, 2) + T(1))));
		if (this->w() >= this->x() && this->w() >= this->y() && this->w() >= this->z()) {
			this->x() *= cgv::math::sign(M(2, 1) - M(1, 2));
			this->y() *= cgv::math::sign(M(0, 2) - M(2, 0));
			this->z() *= cgv::math::sign(M(1, 0) - M(0, 1));
		}
		else if (this->x() >= this->y() && this->x() >= this->z()) {
			this->w() *= cgv::math::sign(M(2, 1) - M(1, 2));
			this->y() *= cgv::math::sign(M(0, 1) + M(1, 0));
			this->z() *= cgv::math::sign(M(2, 0) + M(0, 2));
		}
		else if (this->y() >= this->z()) {
			this->w() *= cgv::math::sign(M(0, 2) - M(2, 0));
			this->x() *= cgv::math::sign(M(0, 1) + M(1, 0));
			this->z() *= cgv::math::sign(M(1, 2) + M(2, 1));
		}
		else {
			this->w() *= cgv::math::sign(M(1, 0) - M(0, 1));
			this->x() *= cgv::math::sign(M(0, 2) + M(2, 0));
			this->y() *= cgv::math::sign(M(1, 2) + M(2, 1));
		}
	}
	/// initialize quaternion directly
	void set(coord_type re, coord_type ix, coord_type iy, coord_type iz)
	{
		this->x() = ix;
		this->y() = iy;
		this->z() = iz;
		this->w() = re;
	}
	/// initialize quaternion from real part and vector
	void set(coord_type re, const vec_type& im)
	{
		set(re, im.x(), im.y(), im.z());
	}
	//@}
	/**@name{\large d) conversions and application}*/
	//@{
	/// compute equivalent 3x3 rotation matrix
	void put_matrix(mat_type& M) const
	{
		M(0, 0) = 1 - 2 * this->y()*this->y() - 2 * this->z()*this->z();
		M(0, 1) = 2 * this->x()*this->y() - 2 * this->w()*this->z();
		M(0, 2) = 2 * this->x()*this->z() + 2 * this->w()*this->y();
		M(1, 0) = 2 * this->x()*this->y() + 2 * this->w()*this->z();
		M(1, 1) = 1 - 2 * this->x()*this->x() - 2 * this->z()*this->z();
		M(1, 2) = 2 * this->y()*this->z() - 2 * this->w()*this->x();
		M(2, 0) = 2 * this->x()*this->z() - 2 * this->w()*this->y();
		M(2, 1) = 2 * this->y()*this->z() + 2 * this->w()*this->x();
		M(2, 2) = 1-2*this->x()*this->x()-2*this->y()*this->y();
	}
	/// return equivalent 3x3 rotation matrix
	mat_type get_matrix() const { mat_type M; put_matrix(M); return M; }
	/// compute equivalent homogeneous 4x4 rotation matrix
	void put_homogeneous_matrix(hmat_type& M) const
	{
		M(0, 0) = 1 - 2 * this->y()*this->y() - 2 * this->z()*this->z();
		M(0, 1) = 2 * this->x()*this->y() - 2 * this->w()*this->z();
		M(0, 2) = 2 * this->x()*this->z() + 2 * this->w()*this->y();
		M(1, 0) = 2 * this->x()*this->y() + 2 * this->w()*this->z();
		M(1, 1) = 1 - 2 * this->x()*this->x() - 2 * this->z()*this->z();
		M(1, 2) = 2 * this->y()*this->z() - 2 * this->w()*this->x();
		M(2, 0) = 2 * this->x()*this->z() - 2 * this->w()*this->y();
		M(2, 1) = 2 * this->y()*this->z() + 2 * this->w()*this->x();
		M(2, 2) = 1 - 2 * this->x()*this->x() - 2 * this->y()*this->y();
		M(3,0) = M(3,1) = M(3,2) = M(0,3) = M(1, 3) = M(2, 3) = M(3, 3) = 0;
		M(3,3) = 1;
	}
	/// return equivalent 4x4 rotation matrix
	hmat_type get_homogeneous_matrix() const { hmat_type M; put_homogeneous_matrix(M); return M; }
	/// initialize quaternion from normal vector
	void set_normal(const vec_type& n)
	{
		coord_type cosfac = 1-n.x();
		if (fabs(cosfac)<EPSILON)
			set(1,0,0,0);
		else {
			cosfac = sqrt(cosfac);
			coord_type sinfac = sqrt(n.y()*n.y() + n.z()*n.z());
			static coord_type fac = (coord_type)(1/sqrt(2.0));
			coord_type tmp = fac*cosfac/sinfac;
			set(fac*sinfac/cosfac, 0, -n.z()*tmp, n.y()*tmp);
		}
	}
	/// extract normal vector
	void put_normal(coord_type* n)
	{
		n[0] = 1-2*(this->y()*this->y()+ this->z()*this->z());
		n[1] = 2*(this->w()*this->z() + this->x()*this->y());
		n[2] = 2*(this->x()*this->z() - this->w()*this->y());
	}
	/// rotate preimage according to quaternion into image
	void put_image(const vec_type& preimage, vec_type& image) const
	{
		image = cross(im(), preimage);
		image = dot(preimage,im())*im() + re()*(re()*preimage + (coord_type)2*image) + cross(im(),image);
	}
	/// rotate vector according to quaternion
	void rotate(vec_type& v) const { vec_type tmp; put_image(v, tmp); v = tmp; }
	/// return rotated vector
	vec_type get_rotated(const vec_type& v) const { vec_type tmp; put_image(v, tmp); return tmp; }
	/// Rotate a frame according to quaternion.
	void rotate(mat_type& m) const
	{
		m.set_col(0, get_rotated(m.col(0)));
		m.set_col(1, get_rotated(m.col(1)));
		m.set_col(2, get_rotated(m.col(2)));
	}
	/// Rotate source frame s into destination frame d.
	mat_type get_rotated(const mat_type& M) const { mat_type tmp = M; rotate(tmp); return tmp; }
	/// rotate image according to quaternion into preimage
	void put_preimage(const vec_type& image, vec_type& preimage) const
	{
		preimage = cross(-im(), image);
		preimage = dot(image,-im())*(-im()) + re()*(re()*image + (coord_type)2*preimage) + cross(-im(),preimage);
	}
	/// rotate vector according to the inverse quaternion
	void inverse_rotate(vec_type& image) const { vec_type tmp; put_preimage(image, tmp); image = tmp; }
	/// rotate vector according to quaternion
	vec_type apply(const vec_type& v) const { vec_type tmp; put_image(v, tmp); return tmp; }
	//@}
	/**@name{\large e) operations}*/
	//@{
	/// return the conjugate
	quaternion<T> conj() const { return quaternion<T>(re(), -im()); }
	/// return the negated quaternion
	quaternion<T> negated() const { return quaternion<T>(-re(), -im()); }
	/// negate the quaternion
	void negate() { conjugate(); re() = -re(); }
	/// compute conjugate
	void conjugate() { this->x() = -this->x(); this->y() = -this->y(); this->z() = -this->z(); }
	/// return the inverse
	quaternion<T> inverse() const { quaternion<T> tmp(*this); tmp.invert(); return tmp; }
	/// compute inverse
	void invert()
	{
		conjugate();
		coord_type sn = this->sqr_length();
		if (sn < EPSILON*EPSILON)
			base_type::operator *= ((coord_type) 1e14);
		else
			base_type::operator /= (sn);
	}
	/// compute affin combination with angular interpolation
	void affin(const quaternion<T>& p, coord_type t, const quaternion<T>& q)
	{
		coord_type omega, cosom, sinom, sclp, sclq;
		*this = q*p;
		cosom = this->sqr_length();
		if ( ( 1 + cosom) > EPSILON ) {
			if ( ( 1 - cosom) > EPSILON ) {
				omega = acos( cosom );
				sinom = sin ( omega );
				sclp = sin ( (1-t) * omega ) / sinom;
				sclq = sin ( t*omega ) / sinom;
			}
			else
			{
				sclp = 1 - t;
				sclq = t;
			}
			set(sclp*p.w() + sclq*q.w(), sclp*p.x() + sclq*q.x(), sclp*p.y() + sclq*q.y(), sclp*p.z() + sclq*q.z());
		}
		else {
			sclp = (T)sin ( (1-t)*M_PI/2 );
			sclq = (T)sin ( t * M_PI/2 );
			set(p.z(), sclp*p.x() - sclq*p.y(), sclp*p.y() + sclq*p.x(), sclp*p.z() - sclq*p.w());
		}
	}
	/// compute affin combination with angular interpolation
	void affin(coord_type s, const quaternion<T>& q) { quaternion<T> tmp; tmp.affin(*this, s, q); *this = tmp; }
	/// negation operator
	quaternion<T> operator - () const { return negated(); }
	/// field multiplication
	quaternion<T>& operator*=(const quaternion<T>& q)
	{
		 // Fastmul-Alg. siehe Seidel-Paper p.4
		coord_type s[9], t;
		s[0] = (this->z()-this->y())*(q.y()-q.z());
		s[1] = (this->w()+this->x())*(q.w()+q.x());
		s[2] = (this->w()-this->x())*(q.y()+q.z());
		s[3] = (this->z()+this->y())*(q.w()-q.x());
		s[4] = (this->z()-this->x())*(q.x()-q.y());
		s[5] = (this->z()+this->x())*(q.x()+q.y());
		s[6] = (this->w()+this->y())*(q.w()-q.z());
		s[7] = (this->w()-this->y())*(q.w()+q.z());
		s[8] = s[5]+s[6]+s[7];
		t    = (s[4] +s[8])/2;
		set(s[0]+t-s[5], s[1]+t-s[8], s[2]+t-s[7], s[3]+t-s[6]);
		return *this;
	}
	/// field multiplication
	quaternion<T> operator * (const quaternion<T>& q) const { quaternion<T> tmp(*this); tmp*=q; return tmp; }
	///in place multiplication with s
	quaternion<T>& operator *= (const T& s) { for (unsigned i=0;i<4;++i) this->v[i] *= s; return *this; }
	///multiplication with scalar s
	quaternion<T>  operator * (const T& s) const { quaternion<T> r = *this; r *= s; return r; }
	//@}
	/**@name{\large f) access to members}*/
	//@{
	/// return real part
	coord_type re() const { return this->w(); }
	/// return reference to real part
	coord_type& re() { return this->w(); }
	/// put imaginary part
	void put_im(vec_type& vector) const { vector.x() = this->x(); vector.y() = this->y(); vector.z() = this->z(); }
	/// return this as vector
	vec_type im() const { return vec_type(this->x(),this->y(),this->z()); }
	/// put rotation axis and return rotation angle
	coord_type put_axis(vec_type& v) const
	{
		if (re() > 1) {
			quaternion<T> q(*this);
			q.normalize();
			return q.put_axis(v);
		}
		coord_type angle = 2 * acos(re());
		coord_type s = sqrt(1 - re()*re());
		if (s < (coord_type)EPSILON) {
			v = vec_type(0,0,1);
			return 0;
		}
		v  = im()/s;
/*		if (2*angle > M_PI) {
			angle -= (coord_type)(2*M_PI);
			v = -v;
		}*/
		return angle;
	}
	/// exponential map
	quaternion<T> exp() const
	{
		T m = im().length();
		quaternion<T> quat(cos(m), im()*(sin(m) / m));
		(typename quaternion<T>::base_type&) quat *= exp(re());
		return quat;
	}
	/// logarithmic map
	quaternion<T> log() const
	{
		T R = this->length();
		typename quaternion<T>::vec_type v(im());
		T sinTimesR = v.length();
		if (sinTimesR < EPSILON)
			v /= sinTimesR;
		else {
			v = typename quaternion<T>::vec_type(0, 0, 0);
			sinTimesR = 0;
		}
		return quaternion<T>(::log(R), atan2(sinTimesR, re()*v));
	}
	//@}
};

///returns the product of a scalar s and vector v
template <typename T>
quaternion<T> operator * (const T& s, const quaternion<T>& v)
{
	quaternion<T> r = v; r *= s; return r;
}


	}
}
