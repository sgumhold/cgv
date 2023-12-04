#include "transforming.h"
#include <cgv/math/inv.h>

namespace cgv {
	namespace nui {
		transforming::transforming()
		{
		}
		mat4 transforming::get_inverse_model_transform() const
		{
			return inv(get_model_transform());
		}
		/// transform a point
		vec3 transforming::transform_point(const vec3& p)
		{
			return get_model_transform() * vec4(p, 1.0f);
		}
		/// inverse transform a point
		vec3 transforming::inverse_transform_point(const vec3& p)
		{
			return get_inverse_model_transform() * vec4(p, 1.0f);
		}
		vec3 transforming::transform_vector(const vec3& v)
		{
			return get_model_transform() * vec4(v, 0.0f);
		}
		vec3 transforming::inverse_transform_vector(const vec3& v)
		{
			return get_inverse_model_transform()* vec4(v, 0.0f);
		}
		vec3 transforming::transform_normal(const vec3& n)
		{
			return vec4(n, 0.0f) * get_inverse_model_transform();
		}
		vec3 transforming::inverse_transform_normal(const vec3& n)
		{
			return vec4(n, 0.0f) * get_model_transform();
		}
		matrix_cached_transforming::matrix_cached_transforming(const mat4& _M)
		{
			set_model_transform(_M);
		}
		mat4 matrix_cached_transforming::get_model_transform() const
		{
			return M;
		}
		mat4 matrix_cached_transforming::get_inverse_model_transform() const
		{
			return iM;
		}
		void matrix_cached_transforming::set_model_transform(const mat4& _M)
		{
			M = _M;
			iM = inv(M);
		}
		void matrix_cached_transforming::set_model_transform(const mat4& _M, const mat4& _iM)
		{
			M = _M;
			iM = _iM;
		}
	}
}
#include <cgv/config/lib_end.h>