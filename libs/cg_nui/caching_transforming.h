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
			/// store model and inverse model matrix
			mat4 M, iM;
			// TODO: Save the transform as components not matrix?
			//vec3 translation_component;
			//quat rotation_component;
			//vec3 scale_component;
			//vec3 perspective_component;
		public:
			/// init to identity matrix
			caching_transforming();
			/// read access to model transform (local)
			const mat4& get_model_transform() const override;
			/// read access to inverse model transform (local)
			const mat4& get_inverse_model_transform() const override;
			/// read access to the translation component (local)
			vec3 get_local_position() const override;
			/// read access to the rotation component (local)
			quat get_local_rotation() const override;
			/// read access to the scale component (local)
			vec3 get_local_scale() const override;

			/// set model transform and compute inverse model transform (local)
			void set_model_transform(const mat4& _M) override;
			/// set model transform and inverse model transform (local)
			void set_model_transform(const mat4& _M, const mat4& _iM) override;
		};

	}
}
#include <cgv/config/lib_end.h>