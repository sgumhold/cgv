#include "scalable.h"
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace nui {

scalable::scalable() {}
uniformly_scalable::uniformly_scalable(float _uniform_scale) : uniform_scale(_uniform_scale) 
{
}
mat4 uniformly_scalable::get_transformation_matrix() const { return cgv::math::scale4<float>(uniform_scale); }
mat4 uniformly_scalable::get_inverse_transformation_matrix() const { return cgv::math::scale4<float>(1.0f / uniform_scale); }
uniformly_scalable::scaling_type uniformly_scalable::get_scaling_type() const { return scaling_type::uniform; }
vec3 uniformly_scalable::get_scale() const { return vec3(uniform_scale); }
bool uniformly_scalable::set_scale(const vec3& scale) {
	if (scale[0] == scale[1] && scale[1] == scale[2]) {
		uniform_scale = scale[0];
		return true;
	}
	uniform_scale = scale[max_index(abs(scale - get_scale()))];
	return false;
}
non_uniformly_scalable::non_uniformly_scalable(const vec3& _scale) : scale(_scale)
{
}
mat4 non_uniformly_scalable::get_transformation_matrix() const
{
	return cgv::math::scale4<float>(scale); 
}
mat4 non_uniformly_scalable::get_inverse_transformation_matrix() const
{ 
	return cgv::math::scale4<float>(1.0f/scale);
}
non_uniformly_scalable::scaling_type non_uniformly_scalable::get_scaling_type() const
{
	return scaling_type::non_uniform;
}
vec3 non_uniformly_scalable::get_scale() const
{ 
	return scale;
}
bool non_uniformly_scalable::set_scale(const vec3& _scale) {
	scale = _scale;
	return true;
}

	}
}

#include <cgv/config/lib_end.h>