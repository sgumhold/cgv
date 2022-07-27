#pragma once

#include <cgv/render/render_types.h>
#include <cgv/base/node.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		using namespace render;
		/// Interface for objects that provides a modelview_projection_window_matrix that is supposed to transform all objects in the hierarchy after this one.
		///	The transform should only ever be set by the object itself. Transforming is NOT meant to provide a way to modify this transform from the outside.
		class CGV_API transforming
		{
			// Used to traverse the hierarchy to calculate the global transform
			base::node* _node;
			bool tried_node_cast{ false };
			base::node* get_node();
		public:
			/// init to identity matrix
			transforming();
			virtual ~transforming() {}

			// Public interface methods. The global transform is calculated using the getter functions for the local transforms.

			/// read access to model transform (local)
			virtual const mat4& get_model_transform() const = 0;
			/// read access to inverse model transform (local)
			virtual const mat4& get_inverse_model_transform() const;
			/// read access to the translation component (local)
			virtual vec3 get_local_position() const;
			/// read access to the rotation component (local)
			virtual quat get_local_rotation() const;
			/// read access to the scale component (local)
			virtual vec3 get_local_scale() const;
		protected:
			// Setter functions only to be used as helpers internally
			// (set_model_transform is required to be implemented to be used by set_global_model_transform).

			/// set model transform and compute inverse model transform (local)
			virtual void set_model_transform(const mat4& _M) {};
			/// set model transform and inverse model transform (local)
			virtual void set_model_transform(const mat4& _M, const mat4& _iM) {};
			/// set model transform and compute inverse model transform (global).
			///	Computes local transform such that the accumulated global transform matches the given transform.
			///	If implementing object is not a node this will just set the local transform.
			void set_global_model_transform(const mat4& _M);
			///	set model transform and inverse model transform (global).
			///	Computes local transform such that the accumulated global transform matches the given transform.
			///	If implementing object is not a node this will just set the local transform.
			void set_global_model_transform(const mat4& _M, const mat4& _iM);
		public:
			// Helper functions. Can be overwritten if necessary (e.g. for better performance), but have a default implementation.

			/// transform a point with the local model transform
			virtual vec3 transform_point(const vec3& p) const;
			/// inverse transform a point with the local model transform
			virtual vec3 inverse_transform_point(const vec3& p) const;
			/// transform a vector with the local model transform
			virtual vec3 transform_vector(const vec3& v) const;
			/// inverse transform a vector with the local model transform
			virtual vec3 inverse_transform_vector(const vec3& v) const;
			/// transform a normal with the local model transform
			virtual vec3 transform_normal(const vec3& n) const;
			/// inverse transform a normal with the local model transform
			virtual vec3 inverse_transform_normal(const vec3& n) const;

			// Static Helper functions

			/// Accumulate model transforms over hierarchy (global transform of object)
			static const mat4& get_global_model_transform(base::node_ptr obj);
			/// Accumulate inverse model transforms over hierarchy (global inverse transform of object)
			static const mat4& get_global_inverse_model_transform(base::node_ptr obj);
			/// Extract the translation, rotation and scale components of the given 4x4 tranformation matrix (assuming no shear, perspective or negative scale)
			static void extract_transform_components(const mat4& transform, vec3& translation, quat& rotation, vec3& scale);
			/// Extract the translation, rotation and scale components of the given 4x4 tranformation matrix (assuming no shear, perspective or negative scale)
			static const mat4& construct_transform_from_components(const vec3& translation, const quat& rotation, const vec3& scale);
		};

	}
}
#include <cgv/config/lib_end.h>