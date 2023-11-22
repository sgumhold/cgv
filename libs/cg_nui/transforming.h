#pragma once

#include <cgv/render/render_types.h>
#include <cgv/base/node.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		using namespace render;
		/// Interface for objects that provides a model matrix that is supposed to transform all objects in the hierarchy after this one.
		///	The transform should only ever be set by the object itself. This interface is NOT meant to provide a way to modify this transform from the outside.
		/// To allow for the calculation of a global transform implementing classes need to inherit base::node to allow for traversal of the hierarchy.
		class CGV_API transforming
		{
			// Used to traverse the hierarchy to calculate the global transform

			base::node* _node;
			bool tried_node_cast{ false };
			base::node* get_node();
		public:
			transforming();
			virtual ~transforming() {}

			// Public interface methods. The global transform is calculated using the getter functions for the local transforms.

			/// Read access to model transform (local)
			virtual mat4 get_model_transform() const = 0;
			/// Read access to inverse model transform (local)
			virtual mat4 get_inverse_model_transform() const;
			/// Read access to the translation component (local)
			virtual vec3 get_local_position() const;
			/// Read access to the rotation component (local)
			virtual quat get_local_rotation() const;
			/// Read access to the scale component (local)
			virtual vec3 get_local_scale() const;
		protected:
			// Setter functions only to be used as helpers internally
			// (set_model_transform is required to be implemented to be used by set_global_model_transform).

			/// Set model transform and compute inverse model transform (local)
			virtual void set_model_transform(const mat4& _M) {};
			/// Set model transform and inverse model transform (local)
			virtual void set_model_transform(const mat4& _M, const mat4& _iM) {};
			/// Set model transform and compute inverse model transform (global).
			///	Computes local transform such that the accumulated global transform matches the given transform.
			///	If implementing object is not a node this will just set the local transform.
			void set_global_model_transform(const mat4& _M);
			///	Set model transform and inverse model transform (global).
			///	Computes local transform such that the accumulated global transform matches the given transform.
			///	If implementing object is not a node this will just set the local transform.
			void set_global_model_transform(const mat4& _M, const mat4& _iM);
		public:
			// Helper functions. Can be overwritten if necessary (e.g. for better performance), but have a default implementation.

			/// Transform a point with the local model transform
			virtual vec3 transform_point(const vec3& p) const;
			/// Inverse transform a point with the local model transform
			virtual vec3 inverse_transform_point(const vec3& p) const;
			/// Transform a vector with the local model transform
			virtual vec3 transform_vector(const vec3& v) const;
			/// Inverse transform a vector with the local model transform
			virtual vec3 inverse_transform_vector(const vec3& v) const;
			/// Transform a normal with the local model transform
			virtual vec3 transform_normal(const vec3& n) const;
			/// Inverse transform a normal with the local model transform
			virtual vec3 inverse_transform_normal(const vec3& n) const;

			// Static Helper functions

			/// Accumulate model transforms over entire hierarchy (global transform of object)
			static mat4 get_global_model_transform(base::node_ptr obj);
			/// Accumulate inverse model transforms over entire hierarchy (global inverse transform of object)
			static mat4 get_global_inverse_model_transform(base::node_ptr obj);
			/// Accumulate model transforms from obj to root
			static mat4 get_partial_model_transform(base::node_ptr obj, base::node_ptr root);
			/// Accumulate inverse model transforms from obj to root
			static mat4 get_partial_inverse_model_transform(base::node_ptr obj, base::node_ptr root);
			/// Extract the translation, rotation and scale components of the given 4x4 tranformation matrix (assuming no shear, perspective or negative scale)
			static void extract_transform_components(const mat4& transform, vec3& translation, quat& rotation, vec3& scale);
			/// Construct a new transform from the given translation, rotation and scale components
			static mat4 construct_transform_from_components(const vec3& translation, const quat& rotation, const vec3& scale);
			/// Construct a new inverse transform from the given inverse translation, rotation and scale components
			static mat4 construct_inverse_transform_from_components(const vec3& inverse_translation, const quat& inverse_rotation, const vec3& inverse_scale);
		};

	}
}
#include <cgv/config/lib_end.h>