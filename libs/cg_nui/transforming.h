#pragma once

#include "focusable.h"
#include <cgv/render/context.h>
#include <cgv/math/ftransform.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
		using namespace cgv::render;
		/// interface for objects that provide a model transformation with default implementation under the assumption that get_model_transform() is implemented.
		class CGV_API transforming
		{
		public:
			/// init to identity matrix
			transforming();
			/// read access to model transform
			virtual mat4 get_model_transform() const = 0;
			/// read access to inverse model transform
			virtual mat4 get_inverse_model_transform() const;
			/// set model transform and compute inverse model transform
			virtual void set_model_transform(const mat4& _M) = 0;
			/// set model transform and inverse model transform
			virtual void set_model_transform(const mat4& _M, const mat4& _iM) = 0;
			/// transform a point
			virtual vec3 transform_point(const vec3& p);
			/// inverse transform a point
			virtual vec3 inverse_transform_point(const vec3& p);
			/// transform a vector
			virtual vec3 transform_vector(const vec3& v);
			/// inverse transform a vector
			virtual vec3 inverse_transform_vector(const vec3& v);
			/// transform a normal
			virtual vec3 transform_normal(const vec3& n);
			/// inverse transform a normal
			virtual vec3 inverse_transform_normal(const vec3& n);
		};
		/// implement transforming interface by storing model matrix and its inverse
		class CGV_API matrix_cached_transforming : public transforming
		{
		protected:
			/// store model and inverse model matrix
			mat4 M, iM;
		public:
			/// construct from given matrix or as identity
			matrix_cached_transforming(const mat4& _M = cgv::math::identity4<float>());
			/// read access to model transform
			mat4 get_model_transform() const;
			/// read access to inverse model transform
			mat4 get_inverse_model_transform() const;
			/// set model transform and compute inverse model transform
			void set_model_transform(const mat4& _M);
			/// set model transform and inverse model transform
			void set_model_transform(const mat4& _M, const mat4& _iM);
		};
	}
}
#include <cgv/config/lib_end.h>