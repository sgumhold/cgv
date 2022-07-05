#pragma once

#include <cgv/render/context.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
		/// interface for objects that provides a modelview_projection_window_matrix
		class CGV_API transforming : public cgv::render::render_types
		{
			base::node* _node;
			bool tried_node_cast{ false };
			base::node* get_node();
		protected:
			/// store model and inverse model matrix
			mat4 M, iM;
		public:
			/// init to identity matrix
			transforming();
			virtual ~transforming() {}
			/// read access to model transform (local)
			virtual const mat4& get_model_transform() const;
			/// read access to inverse model transform (local)
			virtual const mat4& get_inverse_model_transform() const;
			/// read access to accumulated model transform (global)
			virtual const mat4& get_global_model_transform();
			/// read access to accumulated inverse model transform (global)
			virtual const mat4& get_global_inverse_model_transform();
			/// set model transform and compute inverse model transform (local)
			virtual void set_model_transform(const mat4& _M);
			/// set model transform and inverse model transform (local)
			virtual void set_model_transform(const mat4& _M, const mat4& _iM);
			/// set model transform and compute inverse model transform (global)
			///	Computes local transform such that the accumulated global transform matches the given transform
			virtual void set_global_model_transform(const mat4& _M);
			///	set model transform and inverse model transform (global)
			///	Computes local transform such that the accumulated global transform matches the given transform
			virtual void set_global_model_transform(const mat4& _M, const mat4& _iM);
			/// transform a point
			virtual vec3 transform_point(const vec3& p) const;
			/// inverse transform a point
			virtual vec3 inverse_transform_point(const vec3& p) const;
			/// transform a vector
			virtual vec3 transform_vector(const vec3& v) const;
			/// inverse transform a vector
			virtual vec3 inverse_transform_vector(const vec3& v) const;
			/// transform a normal
			virtual vec3 transform_normal(const vec3& n) const;
			/// inverse transform a normal
			virtual vec3 inverse_transform_normal(const vec3& n) const;
			/// Extract the translation, rotation and scale components of the given 4x4 tranformation matrix (assuming no shear, perspective or negative scale)
			void extract_transform_components(const mat4& transform, vec3& translation, quat& rotation, vec3& scale) const;
			/// Extract the translation, rotation and scale components of the given 4x4 tranformation matrix (assuming no shear, perspective or negative scale)
			mat4 construct_transform_from_components(const vec3& translation, const quat& rotation, const vec3& scale) const;
		};

	}
}
#include <cgv/config/lib_end.h>