#pragma once

#include "transformation_matrix_provider.h"
#include <cgv/math/ftransform.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// interface for objects that can be scaled
class CGV_API scalable : public transformation_matrix_provider
{
public:
	/// different representation types for scalings
	enum class scaling_type { uniform, non_uniform };
	/// default construction is empty
	scalable() {}
	/// return internal representation
	virtual scaling_type get_scaling_type() const = 0;
	/// return a vector with one scaling factor per spatial dimension
	virtual vec3 get_scale() const = 0;
	/// set new scale vector and return whether successful - otherwise setter can retrieve corrected scale vector with get_scale() method
	virtual bool set_scale(const vec3& scale) = 0;
};

/// implementation of scalable with uniform scale stored as float
class CGV_API uniformly_scalable : public scalable
{
protected:
	/// store uniform scale
	float uniform_scale = 1.0f;
public:
	/// default initialization of members
	uniformly_scalable() {}
	/// return transformation matrix
	mat4 get_transformation_matrix() const { return cgv::math::scale4<float>(uniform_scale); }
	/// return inverse transformation matrix
	mat4 get_inverse_transformation_matrix() const { return cgv::math::scale4<float>(1.0f / uniform_scale); }
	/// return internal representation
	scaling_type get_scaling_type() const { return scaling_type::uniform; }
	/// return a vector with one scaling factor per spatial dimension
	vec3 get_scale() const { return vec3(uniform_scale); }
	/// set new scale vector and return whether successful - otherwise setter can retrieve corrected scale vector with get_scale() method
	bool set_scale(const vec3& scale) {
		if (scale[0] == scale[1] && scale[1] == scale[2]) {
			uniform_scale = scale[0];
			return true;
		}
		uniform_scale = scale[max_index(abs(scale - get_scale()))];
		return false;
	}
};

/// implementation of scalable with uniform scale stored as float
class CGV_API non_uniformly_scalable : public scalable
{
protected:
	/// store scale vector
	vec3 scale = vec3(1.0f);
public:
	/// default initialization of members
	non_uniformly_scalable() {}
	/// return transformation matrix
	mat4 get_transformation_matrix() const { return cgv::math::scale4<float>(scale); }
	/// return inverse transformation matrix
	mat4 get_inverse_transformation_matrix() const { return cgv::math::scale4<float>(1.0f/scale); }
	/// return internal representation
	scaling_type get_scaling_type() const { return scaling_type::non_uniform; }
	/// return a vector with one scaling factor per spatial dimension
	vec3 get_scale() const { return scale; }
	/// set new scale vector and return whether successful - otherwise setter can retrieve corrected scale vector with get_scale() method
	bool set_scale(const vec3& _scale) {
		scale = _scale;
		return true;
	}
};

	}
}

#include <cgv/config/lib_end.h>