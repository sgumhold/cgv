#pragma once

#include "fmat.h"

namespace cgv {
	namespace math {
		/// construct 4x4 zero matrix
		template <typename T> fmat<T, 4, 4>
			zero4() { fmat<T, 4, 4> M; M.zeros(); return M; }
		/// construct 4x4 identity matrix
		template <typename T> fmat<T, 4, 4>
			identity4() { fmat<T, 4, 4> M; M.identity(); return M; }
		/// construct 4x4 translation matrix from vec3
		template <typename T> fmat<T, 4, 4>
			translate4(const fvec<T, 3>& t) { fmat<T, 4, 4> M; M.identity(); M(0, 3) = t(0); M(1, 3) = t(1); M(2, 3) = t(2); return M; }
		/// construct 4x4 translation matrix from xyz components
		template <typename T> fmat<T, 4, 4>
			translate4(const T& tx, const T& ty, const T& tz) { return translate4(fvec<T, 3>(tx, ty, tz)); }
		/// construct 4x4 scale matrix from xyz scales
		template <typename T> fmat<T, 4, 4>
			scale4(const fvec<T, 3>& s) { fmat<T, 4, 4> M; M.identity(); M(0, 0) = s(0); M(1, 1) = s(1); M(2, 2) = s(2); return M; }
		/// construct 4x4 scale matrix from xyz scales
		template <typename T> fmat<T, 4, 4>
			scale4(const T& sx, const T& sy, const T& sz) { return scale4(fvec<T, 3>(sx, sy, sz)); }
		/// construct 4x4 rotation matrix from angle in degrees and axis 
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

		/// return perspective projection for given eye (-1 ... left / 1 ... right) without translation
		template <typename T> fmat<T, 4, 4>
			stereo_frustum_screen4(int eye, const T& eyeSeparation, const T& screenWidth, const T& screenHeight,
									const T& zZeroParallax, const T& zNear, const T& zFar) {
				T aspect = screenWidth / screenHeight;
				T top = 0.5*screenHeight*zNear / zZeroParallax;
				T bottom = -top;
				T delta = 0.5*eyeSeparation*eye*screenWidth*zNear / zZeroParallax;
				T left = bottom * aspect - delta;
				T right = top * aspect - delta;
				return frustum4<T>(left, right, bottom, top, zNear, zFar);
			}

		/// return translation from center to eye (-1 ... left / 1 ... right)
		template <typename T> fmat<T, 4, 4>
			stereo_translate_screen4(int eye, const T& eyeSeparation, const T& screenWidth) {
				return translate4<T>(-T(0.5)*eyeSeparation*eye*screenWidth, 0, 0);
			}

		/// return translation from center to eye (-1 ... left / 1 ... right)
		template <typename T> fmat<T, 4, 4>
			stereo_translate4(int eye, const T& eyeSeparation, const T& fovy, const T& aspect, const T& zZeroParallax) {
				return stereo_translate_screen4(eye, eyeSeparation, T(2)*tan(T(.8726646262e-2)*fovy)*zZeroParallax * aspect);
			}

		/// return perspective projection for given eye (-1 ... left / 1 ... right) including translation
		template <typename T> fmat<T, 4, 4>
			stereo_perspective_screen4(int eye, const T& eyeSeparation,	const T& screenWidth, const T& screenHeight,
										const T& zZeroParallax, const T& zNear, const T& zFar) {
				return stereo_frustum_screen4(eye, eyeSeparation, screenWidth, screenHeight, zZeroParallax, zNear, zFar) *
				       stereo_translate_screen4(eye, eyeSeparation, screenWidth);
			}

		/// return perspective projection for given eye (-1 ... left / 1 ... right) including translation
		template <typename T> fmat<T, 4, 4>
			stereo_perspective4(int eye, const T& eyeSeparation, const T& fovy, const T& aspect,
							    const T& zZeroParallax, const T& zNear, const T& zFar) {
				T screenHeight = T(2)*tan(T(.8726646262e-2)*fovy)*zZeroParallax;;
				return stereo_perspective_screen4<double>(eye, eyeSeparation, screenHeight*aspect, screenHeight, zZeroParallax, zNear, zFar);
			}
	}
}