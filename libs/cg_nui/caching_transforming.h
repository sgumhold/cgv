#pragma once

#include <cgv/render/context.h>
#include <cg_nui/transforming.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		using namespace render;
		/// Transforming interface with internal storage variable for transform and inverse transform.
		///	Can be used as an implementation of transforming as is.
		class CGV_API caching_transforming : public transforming
		{
		protected:
			/// Internally stored model matrix
			mat4 M;
			/// Internally stored inverse model matrix
			mat4 iM;
			// TODO: Save the transform as components not matrix?
			//vec3 translation_component;
			//quat rotation_component;
			//vec3 scale_component;
			//vec3 perspective_component;
		public:
			/// Init to identity matrix
			caching_transforming();

			// Implementation of the transforming interface using the internally stored model matrix

			mat4 get_model_transform() const override;
			mat4 get_inverse_model_transform() const override;
			vec3 get_local_position() const override;
			quat get_local_rotation() const override;
			vec3 get_local_scale() const override;
			void set_model_transform(const mat4& _M) override;
			void set_model_transform(const mat4& _M, const mat4& _iM) override;
		};

	}
}
#include <cgv/config/lib_end.h>