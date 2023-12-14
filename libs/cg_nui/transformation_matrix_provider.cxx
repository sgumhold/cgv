
#include "transformation_matrix_provider.h"
#include <cgv/math/inv.h>
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace nui {
bool transformation_matrix_provider::has_pre_transformation() const
{
	return false;
}
mat4 transformation_matrix_provider::get_pre_transformation_matrix() const
{
	return cgv::math::identity4<float>();
}
mat4 transformation_matrix_provider::get_inverse_pre_transformation_matrix() const
{
	return cgv::math::identity4<float>();
}
bool transformation_matrix_provider::has_post_transformation() const
{
	return false;
}
mat4 transformation_matrix_provider::get_post_transformation_matrix() const
{
	return cgv::math::identity4<float>();
}
mat4 transformation_matrix_provider::get_inverse_post_transformation_matrix() const
{
	return cgv::math::identity4<float>();
}
mat4 transformation_matrix_provider::get_partial_transformation_matrix() const
{
	return get_transformation_matrix();
}
mat4 transformation_matrix_provider::get_inverse_partial_transformation_matrix() const
{
	return get_inverse_transformation_matrix();
}
mat4 transformation_matrix_provider::get_inverse_transformation_matrix() const {
	return inv(get_transformation_matrix()); 
}

	}
}

#include <cgv/config/lib_end.h>