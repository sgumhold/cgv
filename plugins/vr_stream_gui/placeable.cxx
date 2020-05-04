#include <cgv/math/ftransform.h>
#include <cgv/math/pose.h>

#include "placeable.h"


namespace cgv {
namespace render {

	// could use delegate-constructors
	placeable::placeable()
	{
		position = vec3(0.0f);
		orientation = quat(1.0f, 0.0f, 0.0f, 0.0f);
		scale = vec3(1.0f, 1.0f, 1.0f);
	}
	placeable::placeable(const vec3& _position) : position(_position)
	{
		orientation = quat(1.0f, 0.0f, 0.0f, 0.0f);
		scale = vec3(1.0f, 1.0f, 1.0f);
	}
	placeable::placeable(const vec3& _position, const quat& _orientation)
	    : position(_position), orientation(_orientation)
	{
		scale = vec3(1.0f, 1.0f, 1.0f);
	}

	placeable::placeable(const vec3& _position, const quat& _orientation,
	                     const vec3& _scale)
	    : position(_position), orientation(_orientation), scale(_scale)
	{
	}

	placeable::mat4 placeable::get_model_matrix() const
	{
		mat4 O;
		orientation.put_homogeneous_matrix(O);
		return cgv::math::translate4<float>(position)*O*cgv::math::scale4<float>(scale);
	}

} // namespace render
} // namespace cgv