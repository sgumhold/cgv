#pragma once

#include "focusable.h"
#include <cgv/render/context.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
		/// interface for objects that provides a modelview_projection_window_matrix
		class CGV_API transforming
		{
		protected:
			/// store model and inverse model matrix
			mat4 M, iM;
		public:
			/// init to identity matrix
			transforming();
			/// read access to model transform
			const mat4& get_model_transform() const;
			/// read access to inverse model transform
			const mat4& get_inverse_model_transform() const;
			/// set model transform and compute inverse model transform
			void set_model_transform(const mat4& _M);
			/// set model transform and inverse model transform
			void set_model_transform(const mat4& _M, const mat4& _iM);
			/// transform a point
			vec3 transform_point(const vec3& p);
			/// inverse transform a point
			vec3 inverse_transform_point(const vec3& p);
			/// transform a vector
			vec3 transform_vector(const vec3& v);
			/// inverse transform a vector
			vec3 inverse_transform_vector(const vec3& v);
			/// transform a normal
			vec3 transform_normal(const vec3& n);
			/// inverse transform a normal
			vec3 inverse_transform_normal(const vec3& n);
		};

	}
}
#include <cgv/config/lib_end.h>