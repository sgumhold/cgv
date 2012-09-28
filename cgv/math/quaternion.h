#pragma once

#include "fvec.h"
#include "mat.h"

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
	/// A frame
	typedef mat<T> mat_type;
	//@}

	/**@name{\large b) construction}*/
	//@{
	/// empty standard constructor
	quaternion() {}
	/// copy constructor
	quaternion(const quaternion<T>& quat) : fvec<T,4>(quat) {}
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
	quaternion(const coord_type* matrix)                             { set(matrix); }
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
		set(cos(angle), sin(angle)*axis);
	}
	/// initialize quaternion from 3x3 rotation matrix
	void set(const coord_type* M)
	{
		coord_type t = M[0]+M[4]+M[8]+1;
		if (t > 0) {
			coord_type s = (coord_type) (0.5/sqrt(t));
			set((coord_type) (0.25/s), s*(M[7]-M[5]), s*(M[2]-M[6]), s*(M[3] - M[1]));
		}
		else {
			if ( (M[0] > M[4]) && (M[0] > M[8]) ) {
				coord_type s  = (coord_type) (0.5/sqrt( 1.0 + M[0] - M[4] - M[8] ));
				set((M[5]+M[7])*s, (coord_type) (0.5*s), (M[1]+M[3])*s, (M[2]+M[6])*s);
			}
			else if ( M[4] > M[8] ) {
				coord_type s  = (coord_type) (0.5/sqrt( 1.0 + M[4] - M[0] - M[8] ));
				set((M[2]+M[6])*s, (M[1]+M[3])*s, (coord_type) (0.5*s), (M[5] + M[7])*s);
			}
			else {
				coord_type s = (coord_type) (0.5/sqrt( 1.0 + M[8] - M[0] - M[4] ) );
				set((M[1]+M[3])*s, (M[2]+M[6])*s, (M[5]+M[7])*s, (coord_type) (0.5*s));
			}
		}
	}
	/// initialize quaternion directly
	void set(coord_type re, coord_type ix, coord_type iy, coord_type iz) 
	{
		x() = ix;
		y() = iy;
		z() = iz;
		w() = re;
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
	void put_matrix(coord_type* M) const
	{
	  M[0] = 1-2*y()*y()-2*z()*z();
	  M[1] = 2*x()*y()-2*w()*z();
	  M[2] = 2*x()*z()+2*w()*y();
	  M[3] = 2*x()*y()+2*w()*z();
	  M[4] = 1-2*x()*x()-2*z()*z();
	  M[5] = 2*y()*z()-2*w()*x();
	  M[6] = 2*x()*z()-2*w()*y();
	  M[7] = 2*y()*z()+2*w()*x();
	  M[8] = 1-2*x()*x()-2*y()*y();
	}
	/// initialize quaternion from normal vector
	void set_normal(const vec_type& n)
	{
		Coord cosfac = 1-n.x();
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
		n[0] = 1-2*(y()*y()+ z()*z());
		n[1] = 2*(w()*z() + x()*y());
		n[2] = 2*(x()*z() - w()*y());
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
		preimage = cross(image, im());
		preimage = dot(image,im())*im()+ (re()*image + re()*(coord_type)2*preimage) + cross(preimage,im());
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
	void conjugate() { x() = -x(); y() = -y(); z() = -z(); }
	/// return the inverse
	quaternion<T> inverse() const { quaternion<T> tmp(*this); tmp.invert(); return tmp; }
	/// compute inverse
	void invert()
	{
		conjugate();
		coord_type sn = sqr_length();
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
		cosom = square_length();
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
			sclp = sin ( (1-t)*PI/2 );
			sclq = sin ( t * PI/2 );
			setPoint(p.z(), sclp*p.x() - sclq*p.y(), sclp*p.y() + sclq*p.x(), sclp*p.z() - sclq*p.w());
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
		s[0] = (z()-y())*(q.y()-q.z());
		s[1] = (w()+x())*(q.w()+q.x());
		s[2] = (w()-x())*(q.y()+q.z());
		s[3] = (z()+y())*(q.w()-q.x());
		s[4] = (z()-x())*(q.x()-q.y());
		s[5] = (z()+x())*(q.x()+q.y());
		s[6] = (w()+y())*(q.w()-q.z());
		s[7] = (w()-y())*(q.w()+q.z());
		s[8] = s[5]+s[6]+s[7];
		t    = (s[4] +s[8])/2;
		set(s[0]+t-s[5], s[1]+t-s[8], s[2]+t-s[7], s[3]+t-s[6]);
		return *this;
	}
	/// field multiplication
	quaternion<T> operator * (const quaternion<T>& q) const { quaternion<T> tmp(*this); tmp*=q; return tmp; }
	///in place multiplication with s
	quaternion<T>& operator *= (const T& s) { for (unsigned i=0;i<4;++i) v[i] *= s; return *this; }
	///multiplication with scalar s
	quaternion<T>  operator * (const T& s) const { quaternion<T> r = *this; r *= s; return r; }
	//@}
	/**@name{\large f) access to members}*/
	//@{
	/// return real part
	coord_type re() const { return w(); }
	/// return reference to real part
	coord_type& re() { return w(); }
	/// put imaginary part
	void put_im(vec_type& vector) const { vector.x() = x(); vector.y() = y(); vector.z() = z(); }
	/// return this as vector
	vec_type im() const { return vec_type(x(),y(),z()); }
	/// put rotation axis and return rotation angle
	coord_type put_axis(vec_type& v) const
	{
		coord_type l = length();
		if (l < (coord_type)EPSILON) {
			v = vec_type(0,0,1);
			return 0;
		}
		else {
			put_im(v);
			coord_type a = atan2(l, re());
			if (a > 0.5*PI) {
				a = a - PI;
				axis = -axis;
			}
			v /= l;
			return 2*a;
		}
	}
	//@}
};

///returns the product of a scalar s and vector v
template <typename T>
quaternion<T> operator * (const T& s, const quaternion<T>& v) 
{
	quaternion<T> r = v; r *= s; return r; 
}


/// exponential map
template <typename T>
quaternion<T> exp(const quaternion<T>& q)
{
	coord_type m = q.im().length();
	quaternion<T> quat(cos(m),q.im()*(sin(m)/m));
	(base_type&) quat *= exp(q.re());
	return quat;
}
/// logarithmic map
template <typename T>
quaternion<T> log(const quaternion<T>& q)
{
	coord_type R = q.length();
	vec_type v(q.im());
	coord_type sinTimesR = v.length();
	if (sinTimesR < EPSILON) 
		v /= sinTimesR;
	else {
		v = vec_type(0,0,0);
		sinTimesR = 0;
	}
	return quaternion<T>(::log(R),atan2(sinTimesR, q.re()*v));
}
	}
}