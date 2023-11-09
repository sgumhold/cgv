#include "caching_transforming.h"
#include <cgv/math/inv.h>
#include <cgv/base/node.h>
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace nui {

		caching_transforming::caching_transforming()
		{
			M.identity();
			iM.identity();
		}

		mat4 caching_transforming::get_model_transform() const
		{
			return M;
		}
		
		mat4 caching_transforming::get_inverse_model_transform() const
		{
			return iM;
		}

		vec3 caching_transforming::get_local_position() const
		{
			return get_model_transform().col(3);
		}

		quat caching_transforming::get_local_rotation() const
		{
			mat4 transform = get_model_transform();
			quat rotation = quat(mat3({ normalize(vec3(transform.col(0))), normalize(vec3(transform.col(1))), normalize(vec3(transform.col(2))) }));
			rotation.normalize();
			return rotation;
		}

		vec3 caching_transforming::get_local_scale() const
		{
			mat4 transform = get_model_transform();
			return vec3(vec3(transform.col(0)).length(), vec3(transform.col(1)).length(), vec3(transform.col(2)).length());
		}

		void caching_transforming::set_model_transform(const mat4& _M)
		{
			M = _M;
			iM = inv(M);
		}
		
		void caching_transforming::set_model_transform(const mat4& _M, const mat4& _iM)
		{
			M = _M;
			iM = _iM;
		}
	}
}
#include <cgv/config/lib_end.h>