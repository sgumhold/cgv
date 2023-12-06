#include "translatable.h"
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace nui {
translatable::translatable() 
{
}
default_translatable::default_translatable(const vec3& _position) : position(_position) 
{
}
mat4 default_translatable::get_transformation_matrix() const 
{
	return cgv::math::translate4<float>(position); 
}
mat4 default_translatable::get_inverse_transformation_matrix() const
{
	return cgv::math::translate4<float>(-position); 
}
vec3 default_translatable::get_position() const
{
	return position; 
}
bool default_translatable::set_position(const vec3& _position) 
{
	position = _position;
	return true;
}
	}
}

#include <cgv/config/lib_end.h>