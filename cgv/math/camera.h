#pragma once

#include <vector>
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/math/mat.h>
#include <cgv/math/pose.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/inv.h>
#include "lib_begin.h"

namespace cgv {
	namespace math {
/*
point constraint: (u,v,1) <-> (x,y,1)
                              -x h_4   - y h_5   - h_6 + v*x h_7 + v*y h_8 + v h_9, 
   x h_1   + y h_2   + h_3                             - u*x h_7 - u*y h_8 - u h_9, 
-v*x h_1 - v*y h_2 - v h_3 + u*x h_4 + u*y h_5 + u h_6

Input to Algebra System:
{ {h_1, h_2,h_3}, {h_4, h_5,h_6}, {h_7, h_8,h_9} } . {x, y, z}
{u,v,w} cross (x h_1 + y h_2 + z h_3, x h_4 + y h_5 + z h_6, x h_7 + y h_8 + z h_9)
point constraint: (u,v,w) <-> (x,y,z)
                           -w*x h_4 -w*y h_5 -w*z h_6 +v*x h_7 +v*y h_8 +v*z h_9
 w*x h_1  w*y h_2 +w*z h_3                            -u*x h_7 -u*y h_8 -u*z h_9
-v*x h_1 -v*y h_2 -v*z h_3 +u*x h_4 +u*y h_5 +u*z h_6

line constraint: (d,e,f) <-> (a,b,c)
         - c*d h_2 + b*d h_3           - c*e h_5 + b*e h_6           - c*f h_8 + b*f h_9
 c*d h_1           - a*d h_3 + c*e h_4           - a*e h_6 + c*f h_7           - a*f h_9
-b*d h_1 + a*d h_2           - b*e h_4 + a*e h_5           - b*f h_7 + a*f h_8

point constraint: (u,v,1) <-> (x,y,1)
   0,   0, 0, -x, -y,-1, v*x, v*y, v 
   x,   y, 1,  0,  0, 0,-u*x,-u*y,-u
-v*x,-v*y,-v,u*x,u*y, u,   0,   0, 0

point constraint: (u,v,w) <-> (x,y,z)
   0,   0,   0,-w*x,-w*y,-w*z, v*x, v*y, v*z
 w*x, w*y, w*z,   0,   0,   0,-u*x,-u*y,-u*z
-v*x,-v*y,-v*z, u*x, u*y, u*z,   0,   0,   0

line constraint: (d,e,f) <-> (a,b,c)
   0,-c*d, b*d,   0,-c*e, b*e,   0,-c*f, b*f
 c*d,   0,-a*d, c*e,   0,-a*e, c*f,   0,-a*f
-b*d, a*d,   0,-b*e, a*e,   0,-b*f, a*f,   0

check of formula in Zhang's paper
KC = {{S,H,X},{0,T,Y},{0,0,1}}
KC = {{s,h,x},{0,t,y},{0,0,a}}={{a S,a H,a X},{0,a T,a Y},{0,0,a}}
KC^(-1)={{1/s,-h/(s t)),(h y-t x)/(s t a)},{0,1/t,-y/(t a)}, {0,0,1/a}}
B = l*KC^(-T)*KC^(-1) with l=a^(-2)

B11 = s^(-2)
B12 = -(h/(s^2 t))
B13 = (-(t x) + h y)/(a s^2 t)
B22 = t^(-2) + (h^2)/(s^2 t^2)
B23 = -(y/(a t^2)) - (h (-(t x) + h y))/(a s^2 t^2)
B33 = a^(-2) + (y^2)/(a^2 t^2) + ((-(t x) + h y)^2)/(a^2 s^2 t^2)

a1 = x/(a s^2 t^2) = B12 B23 - B13 B22 = (-(h/(s^2 t)))(-(y/(a t^2)) - (h (-(t x) + h y))/(a s^2 t^2))-((-(t x) + h y)/(a s^2 t))(t^(-2) + (h^2)/(s^2 t^2))
a2 =   1/(s^2 t^2) = B11 B22 - B12^2 = (s^(-2))(t^(-2) + (h^2)/(s^2 t^2))-(-(h/(s^2 t)))^2
a3 = y/(a s^2 t^2) = B12 B13 - B11 B23 = (-(h/(s^2 t)))((-(t x) + h y)/(a s^2 t))-(s^(-2))(-(y/(a t^2)) - (h (-(t x) + h y))/(a s^2 t^2))

X = x/a   = a1/a2
Y = y/a   = a3/a2
l = 1/a^2 = B33 - (B13^2+Y*a3)/B11 = a^(-2) + (y^2)/(a^2 t^2) + ((-(t x) + h y)^2)/(a^2 s^2 t^2) - (((-(t x) + h y)/(a s^2 t))^2 + (y/a)(y/(a s^2 t^2)))/(s^(-2))
S = s/a   =  sqrt(l/B11) = sqrt(a^(-2) s^2)
T = t/a   =  sqrt(l*B11/a2) = sqrt(a^(-2) s^(-2) s^2 t^2)
H = h/a   = -sqrt(l B12^2/(B11 a2)) = sqrt(a^(-2) h^2 s^(-4) t^(-2) s^2 s^2 t^2) = -+h/a

special case S=T:
KC = {{s,h,x},{0,t,y},{0,0,a}}
KC^(-1)={{1/s,-h/(s^2),(h y-s x)/(s^2 a)},{0,1/s,-y/(s a)},{0,0,1/a}}
B11=s^(-2)
B12=-(h/s^3)
B13=(-(s x) + h y)/(a s^3)
B22=h^2/s^4 + s^(-2)
B23=-(y/(a s^2)) - (h (-(s x) + h y))/(a s^4)
B33=a^(-2) + y^2/(a^2 s^2) + (-(s x) + h y)^2/(a^2 s^4)

a1 = x/(a s^4) = B12 B23 - B13 B22
a2 =   1/(s^4) = B11 B22 - B12^2
a3 = y/(a s^4) = B12 B13 - B11 B23

X = x/a   = a1/a2
Y = y/a   = a3/a2
l = 1/a^2 = B33 - (B13^2+Y*a3)/B11 = a^(-2) + (y^2)/(a^2 t^2) + ((-(t x) + h y)^2)/(a^2 s^2 t^2) - (((-(t x) + h y)/(a s^2 t))^2 + (y/a)(y/(a s^2 t^2)))/(s^(-2))
S = s/a   =  sqrt(l/B11) = sqrt(a^(-2) s^2)
T = t/a   =  sqrt(l*B11/a2) = sqrt(a^(-2) s^(-2) s^2 t^2)
H = h/a   = -sqrt(l B12^2/(B11 a2)) = sqrt(a^(-2) h^2 s^(-4) t^(-2) s^2 s^2 t^2) = -+h/a

special case h=0:
KC^(-1)={{1/s,0,(-x)/(s a)},{0,1/t,-y/(t a)}, {0,0,1/a}}
B11=s^(-2)
B12=0
B13=-(x/(a s^2))
B22=t^(-2)
B23=-(y/(a t^2))
B33=a^(-2) + x^2/(a^2 s^2) + y^2/(a^2 t^2)}}

a1 = x/(a s^2 t^2) = - B13 B22 = x/(a s^2 t^2)
a2 = 1/(s^2 t^2) = B11 B22 = (s^(-2))(t^(-2))
a3 = y/(a s^2 t^2) = - B11 B23 = x/(a s^2 t^2)

X = x/a   = a1/a2
Y = y/a   = a3/a2
l = 1/a^2 = B33 - (B13^2+Y*a3)/B11
S = s/a   =  sqrt(l/B11) = sqrt(a^(-2) s^2)
T = t/a   =  sqrt(l*B11/a2) = sqrt(a^(-2) s^(-2) s^2 t^2)
H = h/a   = 0

special case h=0 and S=T:
KC^(-1)={{1/s,0,(-x)/(s a)},{0,1/s,-y/(s a)}, {0,0,1/a}}
B11=s^(-2)
B12=0
B13=-(x/(a s^2))
B22=s^(-2)
B23=-(y/(a s^2))
B33=a^(-2) + x^2/(a^2 s^2) + y^2/(a^2 s^2)}}

a1 = x/(a s^4) = - B13 B22 = x/(a s^2 t^2)
a2 = 1/(s^4) = B11 B22 = (s^(-2))(t^(-2))
a3 = y/(a s^4) = - B11 B23 = x/(a s^2 t^2)

X = x/a   = a1/a2
Y = y/a   = a3/a2
l = 1/a^2 = B33 - (B13^2+Y*a3)/B11
S = s/a   =  sqrt(l/B11) = sqrt(a^(-2) s^2)
T = t/a   =  sqrt(l*B11/a2) = sqrt(a^(-2) s^(-2) s^2 t^2)
H = h/a   = 0
*/

// compute homography from point to point correspondences and return whether this was possible
template <typename T>
bool compute_homography_from_constraint_matrix(const mat<T>& A, fmat<T,3,3>& H)
{
	assert(A.ncols() == 9);
	mat<T> U, V;
	diag_mat<T> W;
	if (!svd(A, U, W, V, true))
		return false;
	vec<T> v = V.col(8);
	H(0,0) = v(0);
	H(0,1) = v(1);
	H(0,2) = v(2);
	H(1,0) = v(3);
	H(1,1) = v(4);
	H(1,2) = v(5);
	H(2,0) = v(6);
	H(2,1) = v(7);
	H(2,2) = v(8);
	return true;
}
// compute homography from pixel \c ui to 2D point \c xi correspondences and return whether this was possible
template <typename T>
bool compute_homography_from_2D_point_correspondences(const std::vector<fvec<T, 2>>& ui, const std::vector<fvec<T, 2>>& xi, fmat<T, 3, 3>& H)
{
	assert(ui.size() == xi.size());
	mat<T> A(3*ui.size(), 9);
	A.zeros();
	for (size_t i = 0; i < ui.size(); ++i) {
		const T& u = ui[i].x();
		const T& v = ui[i].y();
		const T& x = xi[i].x();
		const T& y = xi[i].y();
		size_t j=3*i;
		A(j,3) = -x;
		A(j,4) = -y;
		A(j,5) = T(-1);
		A(j,6) = v * x;
		A(j,7) = v * y;
		A(j,8) = v;
		++j;
		A(j,0) = x;
		A(j,1) = y;
		A(j,2) = T(1);
		A(j,6) = -u * x;
		A(j,7) = -u * y;
		A(j,8) = -u;
		++j;
		A(j,0) = -v*x;
		A(j,1) = -v*y;
		A(j,2) = -v;
		A(j,3) = u * x;
		A(j,4) = u * y;
		A(j,5) = u;
	}
	return compute_homography_from_constraint_matrix(A, H);
}
// compute homography from 3D homogenous pixels \c ui to 3D point \c xi correspondences and return whether this was possible
template <typename T>
bool compute_homography_from_3D_point_correspondences(const std::vector<fvec<T,3>>& ui, const std::vector<fvec<T,3>>& xi, fmat<T,3,3>& H)
{
	assert(ui.size() == xi.size());
	mat<T> A(3*ui.size(), 9);
	A.zeros();
	for (size_t i = 0; i < ui.size(); ++i) {
		const T& u = ui[i].x();
		const T& v = ui[i].y();
		const T& w = ui[i].y();
		const T& x = xi[i].x();
		const T& y = xi[i].y();
		const T& z = xi[i].z();
		size_t j=3*i;
		A(j,3) = -w*x;
		A(j,4) = -w*y;
		A(j,5) = -w*z;
		A(j,6) =  v*x;
		A(j,7) =  v*y;
		A(j,8) =  v*z;
		++j;
		A(j,0) =  w*x;
		A(j,1) =  w*y;
		A(j,2) =  w*z;
		A(j,6) = -u*x;
		A(j,7) = -u*y;
		A(j,8) = -u*z;
		++j;
		A(j,0) = -v*x;
		A(j,1) = -v*y;
		A(j,2) = -v*z;
		A(j,3) =  u*x;
		A(j,4) =  u*y;
		A(j,5) =  u*z;
	}
	return compute_homography_from_constraint_matrix(A, H);
}
// compute homography from 2D lines in pixel coords \c lpi to 2D lines \c li correspondences and return whether this was possible
template <typename T>
bool compute_homography_from_2D_line_correspondences(const std::vector<fvec<T, 3>>& lpi, const std::vector<fvec<T, 3>>& li, fmat<T,3,3>& H)
{
	assert(lpi.size() == li.size());
	mat<T> A(3*lpi.size(), 9);
	A.zeros();
	for (size_t i = 0; i < lpi.size(); ++i) {
		const T& d = lpi[i].x();
		const T& e = lpi[i].y();
		const T& f = lpi[i].y();
		const T& a = li[i].x();
		const T& b = li[i].y();
		const T& c = li[i].z();
		size_t j = 3*i;
		A(j,1) = -c*d;
		A(j,2) =  b*d;
		A(j,4) = -c*e;
		A(j,5) =  b*e;
		A(j,7) = -c*f;
		A(j,8) =  b*f;
		++j;
		A(j, 0) = c*d;
		A(j, 2) = -a*d;
		A(j, 3) = c*e;
		A(j, 5) = -a*e;
		A(j, 6) = c*f;
		A(j, 8) = -a*f;
		++j;
		A(j, 0) = -b*d;
		A(j, 1) = a*d;
		A(j, 3) = -b*e;
		A(j, 4) = a*e;
		A(j, 6) = -b*f;
		A(j, 7) = a*f;
	}
	return compute_homography_from_constraint_matrix(A, H);
}

/// class to store and use a pinhole camera model without distortion but with support for skew and different focal lengths in x and y direction
template <typename T>
class pinhole
{
public:
	// extent in pixels
	unsigned w, h;
	// focal length in pixel units for x and y
	fvec<T,2> s;
	// optical center in pixel units
	fvec<T,2> c;
	// skew strength;
	T skew = 0.0f;
	/// standard constructor
	pinhole() : w(640), h(480), s(T(500)), c(T(0)) {}
	/// copy constructor
	template <typename S>
	pinhole(const pinhole<S>& ph) {
		w = ph.w;
		h = ph.h;
		s = ph.s;
		c = ph.c;
		skew = T(ph.skew);
	}
	fmat<T,2,3> get_camera_matrix() const {
		return { s[0], 0.0f, skew, s[1], c[0], c[1] };
	}
	fmat<T,3,3> get_squared_camera_matrix() const {
		return { s[0], 0.0f, 0.0f, skew, s[1], 0.0f, c[0], c[1], 1.0f };
	}
	fmat<T,4,4> get_homogeneous_camera_matrix() const {
		return { s[0], 0.0f, 0.0f, 0.0f, skew, s[1], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, c[0], c[1], 0.0f, 1.0f };
	}
	fvec<T,2> image_to_pixel_coordinates(const fvec<T,2>& x) const {
		return fvec<T,2>(s[0] * x[0] + skew * x[1] + c[0], s[1] * x[1] + c[1]);
	}
	fvec<T,2> pixel_to_image_coordinates(const fvec<T,2>& p) const {
		T y = (p[1] - c[1]) / s[1]; return fvec<T, 2>((p[0] - c[0] - skew*y)/s[0], y);
	}
	// estimate pinhole parameters from at least 3 homographies
	bool estimate_parameters(const std::vector<fmat<T, 3, 3>>& Hs, bool quadratic_pixels = true, bool no_skew = true)
	{
		assert(Hs.size() >= 3);
		mat<T> A(2 * Hs.size() + (no_skew ? (1 + (quadratic_pixels ? 1 : 0)) : 0) , 6);
		A.zeros();
		size_t j = 2 * Hs.size();
		if (no_skew) {
			A(j, 1) = 1;
			++j;
			if (quadratic_pixels) {
				A(j, 0) = 1;
				A(j, 3) = -1;
			}
		}
		for (size_t i = 0; i < Hs.size(); ++i) {
			vec<T> h1 = Hs[i].col(0);
			vec<T> h2 = Hs[i].col(1);
			size_t j = 2*i;
			A(j,0) = x*x-X*X;
			A(j,1) = T(2)*(x*y-X*Y);
			A(j,2) = T(2)*(x*z-X*Z);
			A(j,3) = y*y-Y*Y;
			A(j,4) = T(2)*(y*z-Y*Z);
			A(j,5) = z*z-Z*Z;
			++j;
			A(j,0) = x*X;
			A(j,1) = x*Y+X*y;
			A(j,2) = x*Z+X*z;
			A(j,3) = y*Y;
			A(j,4) = y*Z+Y*z;
			A(j,5) = z*Z;
		}
		mat<T> U, V;
		diag_mat<T> W;
		if (!svd(A, U, W, V, true))
			return false;
		vec<T> v = V.col(5);
		T B11 = v(0);
		T B12 = v(1);
		T B13 = v(2);
		T B22 = v(3);
		T B23 = v(4);
		T B33 = v(5);

		T a1, a2, a3, l;
		if (no_skew) {
			a1 = - B13 * B22;
			a2 =   B11 * B22;
			a3 = - B11 * B23;
		}
		else {
			a1 = B12*B23 - B13*B22;
			a2 = B11*B22 - B12*B12;
			a3 = B12*B13 - B11*B23;
		}
		c(0) = a1/a2;
		c(1) = a3/a2;
		l    = B33 - (B13*B13 + c(1)*a3)/B11;
		s(0) = sqrt(l / B11);
		if (quadratic_pixels)
			s(1) = s(0);
		else
			s(1) = sqrt(l*B11/a2);
		if (no_skew)
			skew = 0;
		else
			skew = = -sqrt(l*B12*B12/(B11*a2));
		return true;
	}
	// todo: minimize reprojection error 
};

/// common type declarations used by distorted_pinhole class that are independent of the template parameter
class distorted_pinhole_types
{
public:
	/// possible results of applying distortion model
	enum class distortion_result { success, out_of_bounds, division_by_zero };
	/// possible results of inverting distortion model
	enum class distortion_inversion_result { convergence, max_iterations_reached, divergence, out_of_bounds, division_by_zero };
	/// default maximum number of iterations used for inversion of distortion models
	static unsigned get_standard_max_nr_iterations() { return 20; }
};

/// function to provide type specific epsilon for inversion of distortion model
template <typename T> inline T distortion_inversion_epsilon() { return 1e-12; }
/// specialization to float
template <> inline float distortion_inversion_epsilon() { return 1e-6f; }

/// pinhole camera including distortion according to Brown-Conrady model
template <typename T>
class distorted_pinhole : public pinhole<T>, public distorted_pinhole_types
{
public:
	/// slow down factor [0,1] to decrease step size during inverse Jacobian stepping
	static T get_standard_slow_down() { return T(1); }
	// distortion center
	fvec<T,2> dc;
	// internal calibration
	T k[6], p[2];
	// maximum radius allowed for projection
	T max_radius_for_projection = T(10);
	/// standard constructor initializes to no distortion
	distorted_pinhole() : dc(T(0)) {
		k[0] = k[1] = k[2] = k[3] = k[4] = k[5] = p[0] = p[1] = T(0);
	}
	/// copy constructor
	template <typename S>
	distorted_pinhole(const distorted_pinhole<S>& dp) : pinhole<T>(dp) {
		dc = dp.dc;
		unsigned i;
		for (i = 0; i < 6; ++i)
			k[i] = T(dp.k[i]);
		for (i = 0; i < 2; ++i)
			p[i] = T(dp.p[i]);
		max_radius_for_projection = T(dp.max_radius_for_projection);
	}
	//! apply distortion model from distorted to undistorted image coordinates used in projection direction and return whether successful
	/*! Failure cases are zero denominator in distortion formula or radius larger than max projection radius. */
	distortion_result apply_distortion_model(const fvec<T, 2>& xd, fvec<T, 2>& xu, fmat<T, 2, 2>* J_ptr = 0, T epsilon = distortion_inversion_epsilon<T>()) const {
		fvec<T,2> od = xd - dc;
		T xd2 = od[0]*od[0];
		T yd2 = od[1]*od[1];
		T xyd = od[0]*od[1];
		T rd2 = xd2 + yd2;
		// ensure to be within valid projection radius
		if (rd2 > max_radius_for_projection * max_radius_for_projection)
			return distortion_result::out_of_bounds;
		T v = T(1) + rd2 * (k[3] + rd2 * (k[4] + rd2 * k[5]));
		T u = T(1) + rd2 * (k[0] + rd2 * (k[1] + rd2 * k[2]));
		// check for division by very small number or zero
		if (fabs(v) < epsilon*fabs(u))
			return distortion_result::division_by_zero;
		T inv_v = T(1) / v;
		T f = u * inv_v;
		xu[0] = f*od[0] + T(2)*xyd*p[0] + (T(3)*xd2 + yd2)*p[1];
		xu[1] = f*od[1] + T(2)*xyd*p[1] + (xd2 + T(3)*yd2)*p[0];
		if (J_ptr) {
			fmat<T,2,2>& J = *J_ptr;
			T du = k[0] + rd2*(T(2)*k[1] + T(3)*rd2*k[2]);
			T dv = k[3] + rd2*(T(2)*k[4] + T(3)*rd2*k[5]);
			T df = (du*v - dv*u)*inv_v*inv_v;
			J(0, 0) =       f + T(2)*(df*xd2 +      od[1]*p[0] + T(3)*od[0]*p[1]);
			J(1, 1) =       f + T(2)*(df*yd2 + T(3)*od[1]*p[0] +      od[0]*p[1]);
			J(1, 0) = J(0, 1) = T(2)*(df*xyd +      od[0]*p[0] +      od[1]*p[1]);
		}
		return distortion_result::success;
	}
	/// <summary>
	/// invert model for image coordinate inversion
	/// </summary>
	/// <param name="xu">input ... undistorted image coordinates</param>
	/// <param name="xd">output ... distorted image coordinates</param>
	/// <param name="use_xd_as_initial_guess">if true the passed values in xd are used as initial guess, 
	/// otherwise a pseudo inversion of the distortion model is used as initial guess</param>
	/// <param name="iteration_ptr">if provided, the passed index is used as iteration counter and 
	/// indicates after the call how many iterations have been performed</param>
	/// <param name="epsilon">epsilon used to detect division by zero and convergence</param>
	/// <param name="max_nr_iterations">maximum number of to be taken iterations</param>
	/// <param name="slow_down">factor in [0,1] to decrease step estimated by Jacobian inverse</param>
	/// <returns>reason of termination where in all case a best guess for xd is provided</returns>
	distortion_inversion_result invert_distortion_model(const fvec<T,2>& xu, fvec<T,2>& xd, bool use_xd_as_initial_guess = false,
		unsigned* iteration_ptr = 0, T epsilon = distortion_inversion_epsilon<T>(), unsigned max_nr_iterations = get_standard_max_nr_iterations(), T slow_down = get_standard_slow_down()) const {
		// start with approximate inversion
		if (!use_xd_as_initial_guess) {
			fvec<T, 2> od = xu - dc;
			T xd2 = od[0] * od[0];
			T yd2 = od[1] * od[1];
			T xyd = od[0] * od[1];
			T rd2 = xd2 + yd2;
			T inverse_radial = 1.0f + rd2 * (k[3] + rd2 * (k[4] + rd2 * k[5]));
			T enumerator = 1.0f + rd2 * (k[0] + rd2 * (k[1] + rd2 * k[2]));
			if (fabs(enumerator) >= epsilon)
				inverse_radial /= enumerator;
			od *= inverse_radial;
			od -= fvec<T, 2>((yd2 + 3 * xd2) * p[1] + 2 * xyd * p[0], (xd2 + 3 * yd2) * p[0] + 2 * xyd * p[1]);
			xd = od + dc;
		}
		// iteratively improve approximation
		fvec<T,2> xd_best = xd;
		T err_best = std::numeric_limits<T>::max();
		unsigned i;
		if (iteration_ptr == 0)
			iteration_ptr = &i;
		for (*iteration_ptr = 0; *iteration_ptr < max_nr_iterations; ++(*iteration_ptr)) {
			fmat<T,2,2> J;
			fvec<T,2> xu_i;
			distortion_result dr = apply_distortion_model(xd, xu_i, &J, epsilon);
			if (dr == distortion_result::division_by_zero) {
				xd = xd_best;
				return distortion_inversion_result::division_by_zero;
			}
			if (dr == distortion_result::out_of_bounds) {
				xd = xd_best;
				return distortion_inversion_result::out_of_bounds;
			}
			// check for convergence
			fvec<T,2> dxu = xu - xu_i;
			T err = dxu.sqr_length();
			if (err < epsilon * epsilon)
				return distortion_inversion_result::convergence;
			// check for divergence
			if (err > err_best) {
				xd = xd_best;
				return distortion_inversion_result::divergence;
			}
			// improve approximation before the last iteration
			xd_best = xd;
			err_best = err;
			fvec<T, 2> dxd = inv(J) * dxu;
			xd += slow_down * dxd;
		}
		return distortion_inversion_result::max_iterations_reached;
	}
	//! compute for all pixels the distorted image coordinates with the invert_distortion_model() function and store it in a distortion map
	/*! The distortion map can be computed to speed up distortion model inversion if these are 
	    used multiple times per pixel. Given the pixel coordinates x and y and the image width w 
		the distorted image coordinate is looked up via distortion_map[w*y+x]. For pixels 
		where the inversion of the distortion model failed, the invalid_point is stored.
		Further parameters are passed on the the invert_distortion_model() function.*/
	template <typename S>
	void compute_distortion_map(std::vector<cgv::math::fvec<S, 2>>& map, unsigned sub_sample = 1,
		const cgv::math::fvec<S, 2>& invalid_point = cgv::math::fvec<S, 2>(S(-10000)),
		T epsilon = distortion_inversion_epsilon<T>(), unsigned max_nr_iterations = get_standard_max_nr_iterations(), T slow_down = get_standard_slow_down()) const
	{
		unsigned iterations = 1;
		map.resize(w*h);
		size_t i = 0;
		for (uint16_t y = 0; y < h; y += sub_sample) {
			for (uint16_t x = 0; x < w; x += sub_sample) {
				fvec<T, 2> xu = pixel_to_image_coordinates(fvec<T, 2>(x, y));
				fvec<T, 2> xd = xu;
				if (invert_distortion_model(xu, xd, true, &iterations, epsilon, max_nr_iterations, slow_down) ==
					cgv::math::distorted_pinhole_types::distortion_inversion_result::convergence)
					map[i] = cgv::math::fvec<S, 2>(xd);
				else
					map[i] = invalid_point;
				++i;
			}
		}
	}
};

/// extend distorted pinhole with external calibration stored as a pose matrix
template <typename T>
class camera : public distorted_pinhole<T>
{
public:
	/// external calibration
	fmat<T,3,4> pose;
	/// standard constructor
	camera() {
		pose = pose_construct(identity3<T>(), fvec<T, 3>(T(0)));
	}
	/// copy constructor
	template <typename S>
	camera(const camera<S>& cam) : distorted_pinhole<T>(cam) {
		pose = cam.pose;
	}
};

	}
}

#include <cgv/config/lib_end.h>