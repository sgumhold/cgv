#pragma once

#include <cgv/math/fvec.h>
#include <cgv/math/quaternion.h>
#include <cgv/math/fmat.h>

namespace cgv {
	namespace math {

/** implementation of a rigid body transformation with a quaternion and a 
    translation vector. */
template <typename T>
class rigid_transform
{
public:
	/// type of 3d vector
	typedef cgv::math::fvec<T,3> vec_type;
	/// type of homogenous vector
	typedef cgv::math::fvec<T,4> hvec_type;
	/// type of quaternions
	typedef cgv::math::quaternion<T> quat_type;
	/// type of 3x3 matrix
	typedef cgv::math::fmat<T,3,3> mat_type;
	/// type of 4x4 matrix
	typedef cgv::math::fmat<T,4,4> hmat_type;
protected:
	/// store transformation as quaternion
	quat_type q;
	/// and translation vector
	vec_type  t;
public:
	/// construct identity
	rigid_transform() : q(1,0,0,0), t(0,0,0) {}
	/// construct from quaternion and translation vector to the transformation that first rotates and then translates
	rigid_transform(const quat_type& _q, const vec_type& _t) : q(_q), t(_t) {}
	/// apply transformation to vector
	void transform_vector(vec_type& v) const { q.rotate(v); }
	/// apply transformation to point
	void transform_point(vec_type& p) const { q.rotate(p); p += t; }
	/// concatenate two rigid transformations
	rigid_transform<T> operator * (const rigid_transform<T>& M) const { return rigid_transform<T>(q*M.q, get_transformed_vector(M.t)+t); }
	/// multiply quaternion and translation with scalar
	rigid_transform<T>& operator += (const rigid_transform<T>& T) { q += T.q; t += T.t; return *this; }
	/// multiply quaternion and translation with scalar
	rigid_transform<T>& operator *= (T s) { q.vec() *= s; t *= s; return *this; }
	/// multiply quaternion and translation with scalar
	rigid_transform<T> operator * (T s) const { rigid_transform<T> r(*this); r *= s; return r; }
	///
	void normalize() { q.normalize(); }
	/// return the inverse transformation
	rigid_transform<T> inverse() const { return rigid_transform<T>(q.inverse(), q.inverse().apply(-t)); }
	/// apply transformation to vector
	vec_type get_transformed_vector(const vec_type& v) const { return q.apply(v); }
	/// apply transformation to point
	vec_type get_transformed_point(const vec_type& p) const { return q.apply(p)+t; }
	/// return the translational part
	const vec_type& get_translation() const { return t; }
	/// return the translational part
	vec_type& ref_translation() { return t; }
	/// return the rotation as quaternion
	const quat_type& get_quaternion() const { return q; }
	/// return the rotation as quaternion
	quat_type& ref_quaternion() { return q; }
	/// convert transformation to homogeneous transformation
	hmat_type get_hmat() const {
		T M[9];
		q.put_matrix(M);
		hmat_type H;
		H.set_col(0, hvec_type(M[0], M[3], M[6], 0));
		H.set_col(1, hvec_type(M[1], M[4], M[7], 0));
		H.set_col(2, hvec_type(M[2], M[5], M[8], 0));
		H.set_col(3, hvec_type(t(0), t(1), t(2), 1));
		return H;
	}
	/// convert transformation to transpose of homogeneous transformation
	hmat_type get_transposed_hmat() const {
		T M[9];
		q.put_matrix(M);
		hmat_type H;
		H.set_row(0, hvec_type(M[0], M[3], M[6], 0));
		H.set_row(1, hvec_type(M[1], M[4], M[7], 0));
		H.set_row(2, hvec_type(M[2], M[5], M[8], 0));
		H.set_row(3, hvec_type(t(0), t(1), t(2), 1));
		return H;
	}
};

	}
}