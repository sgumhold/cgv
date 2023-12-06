#pragma once

#include "transformation_matrix_provider.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// interface for objects that can be positioned with a translation
class CGV_API translatable : public transformation_matrix_provider
{
public:
	/// default construction is empty
	translatable();
	/// return position representad by translation vector
	virtual vec3 get_position() const = 0;
	/// set new position and return whether successful - otherwise setter can retrieve corrected position with get_position() method
	virtual bool set_position(const vec3& position) = 0;
};

/// default implementation of translatable interface by storing a position
class CGV_API default_translatable : public translatable
{
protected:
	/// position that defines the translation
	vec3 position;
public:
	/// default construction is empty
	default_translatable(const vec3& _position);
	/// return transformation matrix
	mat4 get_transformation_matrix() const;
	/// return inverse transformation matrix
	mat4 get_inverse_transformation_matrix() const;
	/// return position representad by translation vector
	vec3 get_position() const;
	/// set new position and return whether successful - otherwise setter can retrieve corrected position with get_position() method
	bool set_position(const vec3& _position);
};



	}
}

#include <cgv/config/lib_end.h>