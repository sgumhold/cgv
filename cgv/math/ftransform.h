#pragma once

#include "fmat.h"
#include "quaternion.h"

namespace cgv {
	namespace math {
		/// construct 4x4 zero matrix
		template <typename T> fmat<T, 4, 4>
			zero4() { fmat<T, 4, 4> M; M.zeros(); return M; }
		/// construct 2x2 identity matrix
		template <typename T> fmat<T, 2, 2>
			identity2() { fmat<T, 2, 2> M; M.identity(); return M; }
		/// construct 3x3 identity matrix
		template <typename T> fmat<T, 3, 3>
			identity3() { fmat<T, 3, 3> M; M.identity(); return M; }
		/// construct 4x4 identity matrix
		template <typename T> fmat<T, 4, 4>
			identity4() { fmat<T, 4, 4> M; M.identity(); return M; }
		/// construct homogeneous 2x2 translation matrix from vec2
		template <typename T> fmat<T, 3, 3>
			translate2h(const fvec<T, 2>& t) { fmat<T, 3, 3> M; M.identity(); M(0, 2) = t(0); M(1, 2) = t(1); return M; }
		/// construct homogeneous 2x2 translation matrix from xy components
		template <typename T> fmat<T, 3, 3>
			translate2h(const T& tx, const T& ty) { return translate2h(fvec<T, 2>(tx, ty)); }
		/// construct 4x4 translation matrix from vec3
		template <typename T> fmat<T, 4, 4>
			translate4(const fvec<T, 3>& t) { fmat<T, 4, 4> M; M.identity(); M(0, 3) = t(0); M(1, 3) = t(1); M(2, 3) = t(2); return M; }
		/// construct 4x4 translation matrix from xyz components
		template <typename T> fmat<T, 4, 4>
			translate4(const T& tx, const T& ty, const T& tz) { return translate4(fvec<T, 3>(tx, ty, tz)); }
		/// construct homogeneous 2x2 scale matrix from vec2
		template <typename T> fmat<T, 3, 3>
			scale2h(const fvec<T, 2>& s) { fmat<T, 3, 3> M; M.identity(); M(0, 0) = s(0); M(1, 1) = s(1); return M; }
		/// construct homogeneous 2x2 scale matrix from xy components
		template <typename T> fmat<T, 3, 3>
			scale2h(const T& sx, const T& sy) { return scale2h(fvec<T, 2>(sx, sy)); }
		/// construct 2x2 scale matrix from xy scales
		template <typename T> fmat<T, 2, 2>
			scale2(const fvec<T, 2>& s) { fmat<T, 2, 2> M; M.identity(); M(0, 0) = s(0); M(1, 1) = s(1); return M; }
		/// construct 2x2 scale matrix from xy scales
		template <typename T> fmat<T, 2, 2>
			scale2(const T& sx, const T& sy) { return scale2(fvec<T, 2>(sx, sy)); }
		/// construct 3x3 scale matrix from xyz scales
		template <typename T> fmat<T, 3, 3>
			scale3(const fvec<T, 3>& s) { fmat<T, 3, 3> M; M.identity(); M(0, 0) = s(0); M(1, 1) = s(1); M(2, 2) = s(2); return M; }
		/// construct 3x3 scale matrix from xyz scales
		template <typename T> fmat<T, 3, 3>
			scale3(const T& sx, const T& sy, const T& sz) { return scale3(fvec<T, 3>(sx, sy, sz)); }
		/// construct 4x4 scale matrix from xyz scales
		template <typename T> fmat<T, 4, 4>
			scale4(const fvec<T, 3>& s) { fmat<T, 4, 4> M; M.identity(); M(0, 0) = s(0); M(1, 1) = s(1); M(2, 2) = s(2); return M; }
		/// construct 4x4 scale matrix from xyz scales
		template <typename T> fmat<T, 4, 4>
			scale4(const T& sx, const T& sy, const T& sz) { return scale4(fvec<T, 3>(sx, sy, sz)); }
		/// construct 4x4 uniform scale matrix
		template <typename T> fmat<T, 4, 4>
			scale4(const T& s) { return scale4(fvec<T, 3>(s, s, s)); }
		/// construct 2x2 rotation matrix from angle in degrees
		template <typename T> fmat<T, 2, 2>
			rotate2(const T& A) {
				fmat<T, 2, 2> M;
				T angle = T(0.01745329252)*A;
				T c = cos(angle);
				T s = sin(angle);
				M(0, 0) = c;
				M(0, 1) = -s;
				M(1, 0) = s;
				M(1, 1) = c;
				return M;
			}
		/// construct homogeneous 2x2 rotation matrix from angle in degrees
		template <typename T> fmat<T, 3, 3>
			rotate2h(const T& A) {
				fmat<T, 3, 3> M;
				M.identity();
				T angle = T(0.01745329252)*A;
				T c = cos(angle);
				T s = sin(angle);
				M(0, 0) = c;
				M(0, 1) = -s;
				M(1, 0) = s;
				M(1, 1) = c;
				return M;
			}
		/// construct 3x3 rotation matrix from angle in degrees and axis 
		template <typename T> fmat<T, 3, 3>
			rotate3(const T& A, const fvec<T, 3>& a) {
				fmat<T, 3, 3> M;
				T angle = T(0.01745329252)*A;
				T c = cos(angle);
				T s = sin(angle);
				M(0, 0) = c + a(0) * a(0)*(T(1) - c);
				M(0, 1) = -a(2) * s + a(1) * a(0)*(T(1) - c);
				M(0, 2) = a(1) * s + a(2) * a(0)*(T(1) - c);
				M(1, 0) = a(2) * s + a(0) * a(1)*(T(1) - c);
				M(1, 1) = c + a(1) * a(1)*(T(1) - c);
				M(1, 2) = -a(0) * s + a(2) * a(1)*(T(1) - c);
				M(2, 0) = -a(1) * s + a(0) * a(2)*(T(1) - c);
				M(2, 1) = a(0) * s + a(1) * a(2)*(T(1) - c);
				M(2, 2) = c + a(2) * a(2)*(T(1) - c);
				return M;
			}
		/// construct 3x3 rotation matrix from euler angles yaw, pitch and roll around axes y, x and z in degrees
		template <typename T> fmat<T, 3, 3>
			rotate3(const fvec<T, 3>& A) {
				fmat<T, 3, 3>  M;
				fvec<T, 3> angles = cgv::math::fvec<T, 3>(T(0.01745329252))*A;
				// cos and sin for yaw, pitch and roll in radians
				T cy = cos(angles[0]);
				T sy = sin(angles[0]);
				T cp = cos(angles[1]);
				T sp = sin(angles[1]);
				T cr = cos(angles[2]);
				T sr = sin(angles[2]);
				// construct matrix of form rotY(yaw) * rotX(pitch) * rotZ(roll)
				M(0, 0) = sy * sp * sr + cy * cr;
				M(0, 1) = sy * sp * cr - cy * sr;
				M(0, 2) = sy * cp;
				M(1, 0) = cp * sr;
				M(1, 1) = cp * cr;
				M(1, 2) = -sp;
				M(2, 0) = cy * sp * sr - sy * cr;
				M(2, 1) = cy * sp * cr + sy * sr;
				M(2, 2) = cy * cp;
				return M;
			}
		/// construct 3x3 rotation matrix from kardan angles (roll, pitch, yaw) in degrees
		template <typename T> fmat<T, 3, 3> 
			rotate3_rpy(const fvec<T, 3>& A)
			{
				fmat<T, 3, 3> M;
				fvec<T, 3> angles = cgv::math::fvec<T, 3>(T(0.01745329252)) * A;
				T cx = cos(angles[0]);
				T sx = sin(angles[0]);
				T cy = cos(angles[1]);
				T sy = sin(angles[1]);
				T cz = cos(angles[2]);
				T sz = sin(angles[2]);
				M(0, 0) = cy * cz;
				M(0, 1) = sx * sy * cz - cx * sz;
				M(0, 2) = cx * sy * cz + sx * sz;
				M(1, 0) = cy * sz;
				M(1, 1) = sx * sy * sz + cx * cz;
				M(1, 2) = cx * sy * sz - sx * cz;
				M(2, 0) = -sy;
				M(2, 1) = sx * cy;
				M(2, 2) = cx * cy;
				return M;
			}
		/// construct 3x3 rotation matrix from spin vector via Rodrigues formula
		template <typename T> fmat<T, 3, 3>
			rotate3s(fvec<T, 3> k) {
				T o = k.length();
				k /= std::max(o, T(1e-8f));
				fmat<T,3,3> K = { 0.0f, k.z(), -k.y(), -k.z(), 0.0f ,k.x(), k.y(),-k.x(), 0.0f };
				return identity3<float>() + sin(o) * K + (T(1) - cos(o)) * K * K;
			}
		/// construct 4x4 rotation matrix from angle in degrees and axis 
		template <typename T> fmat<T, 4, 4>
			rotate4(const T& A, const fvec<T, 3>& a) { 
				fmat<T, 4, 4> M;
				M.identity();
				T angle = T(0.01745329252)*A;
				T c = cos(angle);
				T s = sin(angle);
				M(0, 0) = c + a(0) * a(0)*(T(1) - c);
				M(0, 1) = -a(2) * s + a(1) * a(0)*(T(1) - c);
				M(0, 2) = a(1) * s + a(2) * a(0)*(T(1) - c);
				M(1, 0) = a(2) * s + a(0) * a(1)*(T(1) - c);
				M(1, 1) = c + a(1) * a(1)*(T(1) - c);
				M(1, 2) = -a(0) * s + a(2) * a(1)*(T(1) - c);
				M(2, 0) = -a(1) * s + a(0) * a(2)*(T(1) - c);
				M(2, 1) = a(0) * s + a(1) * a(2)*(T(1) - c);
				M(2, 2) = c + a(2) * a(2)*(T(1) - c);
				return M;
			}
		/// construct 4x4 rotation matrix from angle in degrees and axis 
		template <typename T> fmat<T, 4, 4>
			rotate4(const T& A, const T& ax, const T& ay, const T& az) { return rotate4(A, fvec<T, 3>(ax, ay, az)); }

		/// construct 4x4 rotation matrix from euler angles yaw, pitch and roll around axes y, x and z in degrees
		template <typename T> fmat<T, 4, 4>
			rotate4(const fvec<T, 3>& A) {
				fmat<T, 4, 4>  M;
				M.identity();
				fvec<T, 3> angles = cgv::math::fvec<T, 3>(T(0.01745329252))*A;
				// cos and sin for yaw, pitch and roll in radians
				T cy = cos(angles[0]);
				T sy = sin(angles[0]);
				T cp = cos(angles[1]);
				T sp = sin(angles[1]);
				T cr = cos(angles[2]);
				T sr = sin(angles[2]);
				// construct matrix of form rotY(yaw) * rotX(pitch) * rotZ(roll)
				M(0, 0) = sy * sp * sr + cy * cr;
				M(0, 1) = sy * sp * cr - cy * sr;
				M(0, 2) = sy * cp;
				M(1, 0) = cp * sr;
				M(1, 1) = cp * cr;
				M(1, 2) = -sp;
				M(2, 0) = cy * sp * sr - sy * cr;
				M(2, 1) = cy * sp * cr + sy * sr;
				M(2, 2) = cy * cp;
				return M;
			}

		/// construct 4x4 rotation matrix from kardan angles (roll, pitch, yaw) in degrees
		template <typename T> fmat<T, 4, 4> 
			rotate4_rpy(const fvec<T, 3>& A)
			{
				fmat<T, 4, 4> M;
				M.identity();
				fvec<T, 3> angles = cgv::math::fvec<T, 3>(T(0.01745329252)) * A;
				T cx = cos(angles[0]);
				T sx = sin(angles[0]);
				T cy = cos(angles[1]);
				T sy = sin(angles[1]);
				T cz = cos(angles[2]);
				T sz = sin(angles[2]);
				M(0, 0) = cy * cz;
				M(0, 1) = sx * sy * cz - cx * sz;
				M(0, 2) = cx * sy * cz + sx * sz;
				M(1, 0) = cy * sz;
				M(1, 1) = sx * sy * sz + cx * cz;
				M(1, 2) = cx * sy * sz - sx * cz;
				M(2, 0) = -sy;
				M(2, 1) = sx * cy;
				M(2, 2) = cx * cy;
				return M;
		}

		/// construct 4x4 pose matrix from a 3x4 matrix
		template <typename T> fmat<T, 4, 4>
			pose4(const fmat<T,3,4>& M) { 
				fmat<T, 4, 4> M4;
				M4.set_col(0, fvec<T, 4>(M.col(0), 0));
				M4.set_col(1, fvec<T, 4>(M.col(1), 0));
				M4.set_col(2, fvec<T, 4>(M.col(2), 0));
				M4.set_col(3, fvec<T, 4>(M.col(3), 1));
				return M4;
			}
		/// construct 4x4 pose matrix from a 3x3 rotation matrix and translation vector
		template <typename T> fmat<T, 4, 4>
			pose4(const fmat<T, 3, 3>& R, const fvec<T, 3>& t) {
				fmat<T, 4, 4> M4;
				M4.set_col(0, fvec<T, 4>(R.col(0), 0));
				M4.set_col(1, fvec<T, 4>(R.col(1), 0));
				M4.set_col(2, fvec<T, 4>(R.col(2), 0));
				M4.set_col(3, fvec<T, 4>(t, 1));
				return M4;
			}

		/// construct 4x4 pose matrix from quaternion and translation vector
		template <typename T> fmat<T, 4, 4>
			pose4(const quaternion<T>& q, const fvec<T, 3>& t) {
				fmat<T, 3, 3> R;
				q.put_matrix(R);
				return pose4<T>(R, t);
			}

		/// return rigid body transformation that performs look at transformation
		template <typename T> fmat<T, 4, 4>
			look_at4(const fvec<T, 3>& eye, const fvec<T, 3>& focus, const fvec<T, 3>& view_up_dir) {
				fvec<T, 3> view_dir = focus-eye; 
				view_dir.normalize();
				fvec<T, 3> up_dir(view_up_dir); 
				up_dir.normalize();
				fvec<T, 3> x = cross(view_dir,up_dir); 
				x.normalize();
				fvec<T, 3> y = cross(x, view_dir);
				fmat<T, 4, 4> R = identity4<T>();
				R(0, 0) = x(0); R(0, 1) = x(1); R(0, 2) = x(2);
				R(1, 0) = y(0); R(1, 1) = y(1); R(1, 2) = y(2);
				R(2, 0) = -view_dir(0); R(2, 1) = -view_dir(1); R(2, 2) = -view_dir(2);
				return R*translate4<T>(-eye);
			}

		/// construct 4x4 frustum matrix for orthographic projection
		template <typename T> fmat<T, 4, 4>
			ortho4(const T& l, const T& r, const T& b, const T& t, const T& n, const T& f) {
				fmat<T, 4, 4> M;
				M.zeros();
				M(0, 0) = T(2) / (r - l);
				M(0, 3) = (r + l) / (l - r);
				M(1, 1) = T(2) / (t - b);
				M(1, 3) = (t + b) / (b - t);
				M(2, 2) = T(2) / (n - f);
				M(2, 3) = (n + f) / (n - f);
				M(3, 3) = T(1);
				return M;
			}
		/// construct 4x4 frustum matrix for perspective projection
		template <typename T> fmat<T, 4, 4>
			frustum4(const T& l, const T& r, const T& b, const T& t, const T& n, const T& f) {
				fmat<T, 4, 4> M;
				M.zeros();
				M(0, 0) = T(2) * n / (r - l);
				M(0, 2) = (r + l) / (r - l);
				M(1, 1) = T(2) * n / (t - b);
				M(1, 2) = (t + b) / (t - b);
				M(2, 2) = (n + f) / (n - f);
				M(2, 3) = T(2) * n * f / (n - f);
				M(3, 2) = -T(1);
				return M;
			}
		/// construct 4x4 perspective frustum matrix
		template <typename T> fmat<T, 4, 4>
			perspective4(const T& fovy, const T& aspect, const T& zNear, const T& zFar) {
				fmat<T, 4, 4> M;
				M.zeros();
				T angle = T(0.008726646262)*fovy;
				T f = cos(angle)/sin(angle);
				M(0, 0) = f / aspect;
				M(1, 1) = f;
				M(2, 2) = (zNear + zFar) / (zNear - zFar);
				M(2, 3) = T(2) * zNear * zFar / (zNear - zFar);
				M(3, 2) = -T(1);
				return M;
			}

		/// extract distance of near clipping plane from a 4x4 projection matrix (ortho or perspective)
		template <typename T>
			inline T znear_from_proj4(const fmat<T, 4, 4>& proj) {
				return (proj[14] + proj[15]) / (proj[10] + proj[11]);
		}
		/// extract distance of far clipping plane from a 4x4 projection matrix (ortho or perspective)
		template <typename T>
			inline T zfar_from_proj4(const fmat<T, 4, 4>& proj) {
				return (proj[14] - proj[15]) / (proj[10] - proj[11]);
		}
		/// extract distance of near clipping plane from given inverse of a 4x4 projection matrix (ortho or perspective)
		template <typename T>
			inline T znear_from_invproj4(const fmat<T, 4, 4>& invproj) {
				return -(invproj[14] - invproj[10]) / (invproj[15] - invproj[11]);
		}
		/// extract distance of far clipping plane from given inverse of a 4x4 projection matrix (ortho or perspective)
		template <typename T>
			inline T zfar_from_invproj4(const fmat<T, 4, 4>& invproj) {
				return -(invproj[10] + invproj[14]) / (invproj[11] + invproj[15]);
		}

		/// return perspective projection for given eye (from -1 ... left most to 1 ... right most) without translation
		template <typename T> fmat<T, 4, 4>
			stereo_frustum_screen4(T eye, const T& eyeSeparation, const T& screenWidth, const T& screenHeight,
									const T& zZeroParallax, const T& zNear, const T& zFar) {
				T aspect = screenWidth / screenHeight;
				T top = T(0.5)*screenHeight*zNear / zZeroParallax;
				T bottom = -top;
				T delta = T(0.5)*eyeSeparation*eye*screenWidth*zNear / zZeroParallax;
				T left = bottom * aspect - delta;
				T right = top * aspect - delta;
				return frustum4<T>(left, right, bottom, top, zNear, zFar);
			}

		/// return translation from center to eye (from -1 ... left most to 1 ... right most)
		template <typename T> fmat<T, 4, 4>
			stereo_translate_screen4(T eye, const T& eyeSeparation, const T& screenWidth) {
				return translate4<T>(-T(0.5)*eyeSeparation*eye*screenWidth, 0, 0);
			}

		/// return translation from center to eye (from -1 ... left most to 1 ... right most)
		template <typename T> fmat<T, 4, 4>
			stereo_translate4(T eye, const T& eyeSeparation, const T& fovy, const T& aspect, const T& zZeroParallax) {
				return stereo_translate_screen4(eye, eyeSeparation, T(2)*tan(T(.8726646262e-2)*fovy)*zZeroParallax * aspect);
			}

		/// return perspective projection for given eye (from -1 ... left most to 1 ... right most) including translation
		template <typename T> fmat<T, 4, 4>
			stereo_perspective_screen4(T eye, const T& eyeSeparation,	const T& screenWidth, const T& screenHeight,
										const T& zZeroParallax, const T& zNear, const T& zFar) {
				return stereo_frustum_screen4(eye, eyeSeparation, screenWidth, screenHeight, zZeroParallax, zNear, zFar) *
				       stereo_translate_screen4(eye, eyeSeparation, screenWidth);
			}

		/// return perspective projection for given eye (from -1 ... left most to 1 ... right most) including translation
		template <typename T> fmat<T, 4, 4>
			stereo_perspective4(T eye, const T& eyeSeparation, const T& fovy, const T& aspect,
							    const T& zZeroParallax, const T& zNear, const T& zFar) {
				T screenHeight = T(2)*tan(T(.8726646262e-2)*fovy)*zZeroParallax;;
				return stereo_perspective_screen4<double>(eye, eyeSeparation, screenHeight*aspect, screenHeight, zZeroParallax, zNear, zFar);
			}
	}
}
