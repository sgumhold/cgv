#include "transforming.h"
#include <cgv/math/inv.h>

namespace cgv {
	namespace nui {

		transforming::transforming()
		{
			M.identity();
			iM.identity();
		}
		/// read access to model transform
		const mat4& transforming::get_model_transform() const
		{
			return M;
		}
		/// read access to inverse model transform
		const mat4& transforming::get_inverse_model_transform() const
		{
			return iM;
		}
		/// set model transform and compute inverse model transform
		void transforming::set_model_transform(const mat4& _M)
		{
			M = _M;
			iM = inv(M);
		}
		/// set model transform and inverse model transform
		void transforming::set_model_transform(const mat4& _M, const mat4& _iM)
		{
			M = _M;
			iM = _iM;
		}
		/// transform a point
		vec3 transforming::transform_point(const vec3& p)
		{
			return M * vec4(p, 1.0f);
		}
		/// inverse transform a point
		vec3 transforming::inverse_transform_point(const vec3& p)
		{
			return iM * vec4(p, 1.0f);
		}
		/// transform a vector
		vec3 transforming::transform_vector(const vec3& v)
		{
			return M * vec4(v, 0.0f);
		}
		/// inverse transform a vector
		vec3 transforming::inverse_transform_vector(const vec3& v)
		{
			return iM * vec4(v, 0.0f);
		}

		/// transform a normal
		vec3 transforming::transform_normal(const vec3& n)
		{
			return vec4(n, 0.0f) * iM;
		}
		/// inverse transform a normal
		vec3 transforming::inverse_transform_normal(const vec3& n)
		{
			return vec4(n, 0.0f) * M;
		}
	}
}
#include <cgv/config/lib_end.h>