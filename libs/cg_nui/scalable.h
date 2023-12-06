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
	scalable();
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
	uniformly_scalable(float _uniform_scale);
	/// return transformation matrix
	mat4 get_transformation_matrix() const;
	/// return inverse transformation matrix
	mat4 get_inverse_transformation_matrix() const;
	/// return internal representation
	scaling_type get_scaling_type() const;
	/// return a vector with one scaling factor per spatial dimension
	vec3 get_scale() const;
	/// set new scale vector and return whether successful - otherwise setter can retrieve corrected scale vector with get_scale() method
	bool set_scale(const vec3& scale);
};

/// implementation of scalable with uniform scale stored as float
class CGV_API non_uniformly_scalable : public scalable
{
protected:
	/// store scale vector
	vec3 scale = vec3(1.0f);
public:
	/// default initialization of members
	non_uniformly_scalable(const vec3& _scale);
	/// return transformation matrix
	mat4 get_transformation_matrix() const;
	/// return inverse transformation matrix
	mat4 get_inverse_transformation_matrix() const;
	/// return internal representation
	scaling_type get_scaling_type() const;
	/// return a vector with one scaling factor per spatial dimension
	vec3 get_scale() const;
	/// set new scale vector and return whether successful - otherwise setter can retrieve corrected scale vector with get_scale() method
	bool set_scale(const vec3& _scale);
};

	}
}

#include <cgv/config/lib_end.h>