#pragma once

#include <iostream>
#include <cmath>
#include <cassert>
#include <cgv/math/vec.h>
#include <cgv/math/mat.h>

namespace cgv {
namespace math {

/**
* Quaterions are stored in vectors in the following order:
* [w, x, y, z]  -> w + x*i + y*j + z*k
*/


// returns the product of quaternion v and q
template <typename T>
vec<T> quat_multiply(const vec<T>& v,const vec<T>& q)
{ 
	assert(v.dim() == 4);
	assert(q.dim() == 4);
	vec<T> r(4);
	r(0) = v[0]*q(0) - v[1]*q(1) - v[2]*q(2) - v[3]*q(3);
	r(1) = v[0]*q(1) + v[1]*q(0) + v[2]*q(3) - v[3]*q(2);	 
	r(2) = v[0]*q(2) + v[2]*q(0) + v[3]*q(1) - v[1]*q(3);
	r(3) = v[0]*q(3) + v[3]*q(0) + v[1]*q(2) - v[2]*q(1);
	return r;
}

///returns a quaterion q defined by the given axis and angle
template <typename T>
vec<T> axis_angle_2_quat(vec<T> axis, T angle)
{
	angle = (T)angle*3.14159/360.0;
	T s = (T)sinf((float)angle );
	axis.normalize();
	
	vec<T> q(4);
	
	q(0) = cos(angle );
	q(1) = axis(0) * s;
	q(2) = axis(1) * s;
	q(3) = axis(2) * s;
	return q;
}

///extract the axis and angle in degree from quaternion q
template <typename T>
void quat_2_axis_angle(const vec<T>& q,vec<T>& axis, T& angle)
{
	axis.resize(3);
	T s = sqrt((q(1) * q(1)) + (q(2) * q(2)) + (q(3) * q(3)));

	if (s == 0) 
	{
	 	axis(0) = 1;
		axis(1) = 0;
		axis(2) = 0;
		angle = 0;

	} 
	else 
	{
		axis(0) = q(1) / s;
		axis(1) = q(2) / s;
		axis(2) = q(3) / s;
		angle = 2.0 * acos(q(0));
	}
}


///returns the rotation of  vector v by quaternion q
template <typename T>
vec<T> quat_rotate(const vec<T>&q, const vec<T>& v)
{
	
	assert(v.dim() >= 3);
	assert(q.dim() == 4);
	
	vec<T> pos(v.dim());

	T w =  - q(1)*v(0) - q(2)*v(1) - q(3)*v(2);
	T x =    q(0)*v(0) + q(2)*v(2) - q(3)*v(1);
	T y =    q(0)*v(1) + q(3)*v(0) - q(1)*v(2);
	T z =    q(0)*v(2) + q(1)*v(1) - q(2)*v(0);
	
	pos(0) = -w*q(1) + x*q(0) - y*q(3) + z*q(2);
	pos(1) = -w*q(2) + y*q(0) - z*q(1) + x*q(3);
	pos(2) = -w*q(3) + z*q(0) - x*q(2) + y*q(1);
	int i;
	for(i = 3; i < v.dim(); i++)
		pos(i) = v(i);
	return pos;

}

///returns the conjugate quaternion q
template <typename T>
vec<T> quat_conj(const vec<T>& q)
{
	vec<T> r(4);
	r(0) =  q(0);
	r(1) = -q(1);
	r(2) = -q(2);
	r(3) = -q(3);
}

///returns  a normalize version of quaternion q
template <typename T>
vec<T> quat_normalize(const vec<T>& q)
{
	T magnitude = sqrt(q(0)*q(0) + q(1)*q(1) + q(2)*q(2) + q(3)*q(3));

	vec<T> r(4);
	r(0)= q(0) / magnitude;
	r(1)= q(1) / magnitude;
	r(2)= q(2) / magnitude;
	r(3)= q(3) / magnitude;	
	return r;
}

///return  the inverse of quaterion q
template <typename T>
vec<T> quat_inv(const vec<T>& q)
{
	T magnitude = sqrt(q(0)*q(0) + q(1)*q(1) + q(2)*q(2) + q(3)*q(3));

	vec<T> r(4);
	r(0)= q(0) / magnitude;
	r(1)= -q(1) / magnitude;
	r(2)= -q(2) / magnitude;
	r(3)= -q(3) / magnitude;
	return r;
}

///convert unit quaternion to 3x3 rotation matrix
template <typename T>
mat<T> quat_2_mat_33(const vec<T>& q)
{
	assert(q.dim() == 4);
	mat<T> m(3,3);
	T xx      = q[1] * q[1];
	T xy      = q[1] * q[2];
	T xz      = q[1] * q[3];
	T xw      = q[1] * q[0];
	T yy      = q[2] * q[2];
	T yz      = q[2] * q[3];
	T yw      = q[2] * q[0];
	T zz      = q[3] * q[3];
	T zw      = q[3] * q[0];
	m(0,0)  = 1 - 2 * ( yy + zz );
	m(0,1)  =     2 * ( xy - zw );
	m(0,2)  =     2 * ( xz + yw );
	m(1,0)  =     2 * ( xy + zw );
	m(1,1)  = 1 - 2 * ( xx + zz );
	m(1,2)  =     2 * ( yz - xw );
	m(2,0)  =     2 * ( xz - yw );
	m(2,1)  =     2 * ( yz + xw );
	m(2,2)  = 1 - 2 * ( xx + yy );
	return m;
}

///convert unit quaternion to 4x4 rotation matrix
template <typename T>
mat<T> quat_2_mat_44(const vec<T>& q) 
{
	assert(q.dim() == 4);
	
	mat<T> m(4,4);
	T xx      = q[1] * q[1];
	T xy      = q[1] * q[2];
	T xz      = q[1] * q[3];
	T xw      = q[1] * q[0];
	T yy      = q[2] * q[2];
	T yz      = q[2] * q[3];
	T yw      = q[2] * q[0];
	T zz      = q[3] * q[3];
	T zw      = q[3] * q[0];
	m(0,0)  = 1 - 2 * ( yy + zz );
	m(0,1)  =     2 * ( xy - zw );
	m(0,2)  =     2 * ( xz + yw );
	m(1,0)  =     2 * ( xy + zw );
	m(1,1)  = 1 - 2 * ( xx + zz );
	m(1,2)  =     2 * ( yz - xw );
	m(2,0)  =     2 * ( xz - yw );
	m(2,1)  =     2 * ( yz + xw );
	m(2,2) = 1 - 2 * ( xx + yy );
	m(3,0) = m(3,1) = m(3,2) = m(0,3) = m(1,3) = m(2,3) = 0;
	m(3,3) = 1;
	return m;
	
}

	
	/*
/// A column vector class.
template <typename T>
class quat
{	

private:
	///pointer to _data storage
	T v[4];
	
public:

	quat()
	{
		v[0]    = 0;
		v[1]    = 0;
		v[2]    = 0;
		v[3]    = 1;
	}

	quat(const T& x,const T& y,const T& z,const T w)
	{
		v[0]    = x;
		v[1]    = y;
		v[2]    = z;
		v[3]    = w;
		
	}

	quat(const vec<T>& axis,const T angle)
	{
		set(axis,angle);
	}


	quat(const quat &q)
	{
		for (int i=0; i<4; i++)
			v[i]=q(i);
	}

	quat& operator=(const quat &rhs) 
	{
		if (this != &rhs)
		{
			for (int i=0; i<4; i++)
				v[i]=rhs(i);
		}
		return *this;
	}

	void conjugate()
    {
		v[0]    = -v[0];
		v[1]    = -v[1];
		v[2]    = -v[2];
    }

	quat<T>& operator*=(const quat<T>& q)
    {
		T w = v[3]*q(3) - v[0]*q(0) - v[1]*q(1) - v[2]*q(2);
		T x = v[3]*q(0) + v[0]*q(3) + v[1]*q(2) - v[2]*q(1);
		T y = v[3]*q(1) + v[1]*q(3) + v[2]*q(0) - v[0]*q(2);
		T z = v[3]*q(2) + v[2]*q(3) + v[0]*q(1) - v[1]*q(0);
		v[0]=x;
		v[1]=y;
		v[2]=z;
		v[3]=w;
		
		return *this;

    }

	const quat<T> operator*(const quat<T>& q)const
    {
		
		quat<T> r =*this;
		r*=q;
		return r;

    }


	T length()
	{
		T l =0;
		for(int i = 0; i < 4;i++)
			l += v[i]*v[i];
		return sqrt(l);
	}

	T sqr_length()
	{
		T l =0;
		for(int i = 0; i < 4;i++)
			l += v[i]*v[i];
		return l;
	}


	mat<T> to_mat_33() const
	{
		mat<T> mat(3,3);
		T xx      = v[0] * v[0];
		T xy      = v[0] * v[1];
		T xz      = v[0] * v[2];
		T xw      = v[0] * v[3];
		T yy      = v[1] * v[1];
		T yz      = v[1] * v[2];
		T yw      = v[1] * v[3];
		T zz      = v[2] * v[2];
		T zw      = v[2] * v[3];
		mat(0,0)  = 1 - 2 * ( yy + zz );
		mat(0,1)  =     2 * ( xy - zw );
		mat(0,2)  =     2 * ( xz + yw );
		mat(1,0)  =     2 * ( xy + zw );
		mat(1,1)  = 1 - 2 * ( xx + zz );
		mat(1,2)  =     2 * ( yz - xw );
		mat(2,0)  =     2 * ( xz - yw );
		mat(2,1)  =     2 * ( yz + xw );
		mat(2,2) = 1 - 2 * ( xx + yy );
		return mat;
	}

	mat<T> to_mat_44() const
	{
		mat<T> mat(4,4);
		T xx      = v[0] * v[0];
		T xy      = v[0] * v[1];
		T xz      = v[0] * v[2];
		T xw      = v[0] * v[3];
		T yy      = v[1] * v[1];
		T yz      = v[1] * v[2];
		T yw      = v[1] * v[3];
		T zz      = v[2] * v[2];
		T zw      = v[2] * v[3];
		mat(0,0)  = 1 - 2 * ( yy + zz );
		mat(0,1)  =     2 * ( xy - zw );
		mat(0,2)  =     2 * ( xz + yw );
		mat(1,0)  =     2 * ( xy + zw );
		mat(1,1)  = 1 - 2 * ( xx + zz );
		mat(1,2)  =     2 * ( yz - xw );
		mat(2,0)  =     2 * ( xz - yw );
		mat(2,1)  =     2 * ( yz + xw );
		mat(2,2) = 1 - 2 * ( xx + yy );
		mat(3,0) = mat(3,1) =mat(3,2)=mat(0,3)=mat(1,3)=mat(2,3) = 0;
		mat(3,3) = 1;
		return mat;
	}

	T& operator()(const int i)	//subscripting
	{
		return v[i];
	}
	
	
	const T&  operator()(const int i) const	//subscripting
	{
		return v[i];
	}

	///cast into non const array 
	operator T*()
	{
		return v;
	}

	///cast into const array
	operator const T*() const
	{
		return v;
	}

	///normalize the vector
	void normalize() 
	{
		T l =0;
		for(int i = 0; i < 4;i++)
			l += v[i]*v[i];
		l=(T)1.0/sqrt(l);
		for(unsigned i = 0; i<4; i++)
			 operator()(i)=l*operator()(i);
	}

	void set(const T& x, const T&y, const T&z,const T&w)
	{
		v[0]=x;
		v[1]=y;
		v[2]=z;
		v[3]=w;
		
	}

	
	void set(vec<T> axis, const T angle) 
	{
		axis.normalize();
		T sin_a = (T)sin( angle / (T)2.0 );
		T cos_a = (T)cos( angle / (T)2.0 );
		v[0]    = axis.x() * sin_a;
		v[1]    = axis.y() * sin_a;
		v[2]    = axis.z() * sin_a;
		v[3]    = cos_a;

	}

	void invert()
	{
		
		conjugate();
		normalize();
		
	}

	void rotate(vec<T>& pos) const
	{
		
		T w =  - v[0]*pos(0) - v[1]*pos(1) - v[2]*pos(2);
		T x = v[3]*pos(0)  + v[1]*pos(2) - v[2]*pos(1);
		T y = v[3]*pos(1)  + v[2]*pos(0) - v[0]*pos(2);
		T z = v[3]*pos(2)  + v[0]*pos(1) - v[1]*pos(0);
	
		pos(0) = -w*v[0] + x*v[3] - y*v[2] + z*v[1];
		pos(1) = -w*v[1] + y*v[3] - z*v[0] + x*v[2];
		pos(2) = -w*v[2] + z*v[3] - x*v[1] + y*v[0];
		
	}

};

template<typename T>
vec<T> rotate(const quat<T>& q,const vec<T>& pos)
{
	vec<T> v = pos;
	q.rotate(v);
	return v;
}

template<typename T>
quat<T> normalize(const quat<T>& q)
{
	quat<T> r = q;
	r.normalize();
	return r;
}

//output of a vector
template<typename T>
std::ostream& operator<<(std::ostream& out, const quat<T>& q)
{

	for (unsigned i=0;i<3;++i)
	{
		out << q(i)<<"\t";	
	}
	out << q(3);
	return out;

}

//input of a vector
template<typename T>
std::istream& operator>>(std::istream& in, quat<T>& q)
{

	for (unsigned i=0;i<4;++i)
	{
		in >> q(i);	
	}
	
	return in;

}

*/


}

}
