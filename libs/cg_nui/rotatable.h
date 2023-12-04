#pragma once

#include "transformation_matrix_provider.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// syntax check of euler convention
bool is_euler_angle_convention_valid(const char* convention);
/// assert correct syntax of euler convention
void assert_euler_angle_convention(const char* convention);
/// convert euler angle to matrix representation
mat3 euler_angle_to_matrix(const vec3& euler_angles, const char* convention = "XYZ");
/// convert matrix to euler angle representation
vec3 matrix_to_euler_angle(const mat3& matrix, const char* convention = "XYZ");

/// interface for objects that can be rotated
class CGV_API rotatable : public transformation_matrix_provider
{
public:
	/// different representation types for rotations
	enum class rotation_type { euler_angles, spin_vector, quaternion, matrix };
	/// Use internal rotation variable or custom override of get_rotation and set_rotation
	rotatable() {}
	/// return transformation matrix
	mat4 get_transformation_matrix() const;
	/// return inverse of transformation matrix with default implementation via matrix inversion
	mat4 get_inverse_transformation_matrix() const;
	/// return internal representation type
	virtual rotation_type get_rotation_type() const = 0;
	/// get rotation in euler angles of given convention
	virtual vec3 get_rotation_euler_angles(const char* convention = "XYZ") const = 0;
	/// get rotation as spin vector
	virtual vec3 get_rotation_spin_vector() const = 0;
	/// get rotation as quaternion
	virtual quat get_rotation_quaternion() const = 0;
	/// get rotation as matrix
	virtual mat3 get_rotation_matrix() const = 0;
	/// set rotation from euler angles and return whether this was possible
	virtual bool set_rotation_euler_angles(const vec3& euler_angles, const char* convention = "XYZ") = 0;
	/// set rotation from spin vector and return whether this was possible
	virtual bool set_rotation_spin_vector(const vec3& spin_vector) = 0;
	/// set rotation from quaternion and return whether this was possible
	virtual bool set_rotation_quaternion(const quat& quaternion) = 0;
	/// set rotation from matrix and return whether this was possible
	virtual bool set_rotation_matrix(const mat3& matrix) = 0;
};

/// implementation of rotatable with euler angles as internal representation
class CGV_API euler_angle_rotatable : public rotatable
{
protected:
	std::string convention;
	vec3 euler_angles;
public:
	/// construct from quaternion that defaults to identity quaternion
	euler_angle_rotatable(const vec3& eas = vec3(0.0f), const char* _convention = "XYZ") : euler_angles(eas), convention(_convention) {}
	/// return internal representation type
	rotation_type get_rotation_type() const { return rotation_type::euler_angles; }
	/// get rotation in euler angles of given convention
	vec3 get_rotation_euler_angles(const char* _convention = "XYZ") const {
		if (convention == _convention)
			return euler_angles;
		return matrix_to_euler_angle(euler_angle_to_matrix(euler_angles, convention.c_str()), _convention);
	}
	/// get rotation as spin vector
	vec3 get_rotation_spin_vector() const;
	/// get rotation as quaternion
	quat get_rotation_quaternion() const;
	/// get rotation as matrix
	mat3 get_rotation_matrix() const {
		return euler_angle_to_matrix(euler_angles, convention.c_str());
	}
	/// set rotation from euler angles and return whether this was possible
	bool set_rotation_euler_angles(const vec3& _euler_angles, const char* _convention = "XYZ")
	{
		euler_angles = (convention == _convention)?_euler_angles:matrix_to_euler_angle(euler_angle_to_matrix(_euler_angles,_convention),convention.c_str());
		return true;
	}
	/// set rotation from spin vector and return whether this was possible
	bool set_rotation_spin_vector(const vec3& spin_vector);
	/// set rotation from quaternion and return whether this was possible
	bool set_rotation_quaternion(const quat& _quaternion);
	/// set rotation from matrix and return whether this was possible
	bool set_rotation_matrix(const mat3& matrix) {
		euler_angles = matrix_to_euler_angle(matrix, convention.c_str());
		return true;
	}
};
/// implementation of rotatable with quaternion as internal representation
class CGV_API quaternion_rotatable : public rotatable
{
protected:
	quat quaternion;
public:
	/// construct from quaternion that defaults to identity quaternion
	quaternion_rotatable(const quat& q = quat(1.0f, 0.0f, 0.0f, 0.0f)) : quaternion(q) {}
	/// return internal representation type
	rotation_type get_rotation_type() const { return rotation_type::quaternion; }
	/// get rotation in euler angles of given convention
	vec3 get_rotation_euler_angles(const char* convention = "XYZ") const;
	/// get rotation as spin vector
	vec3 get_rotation_spin_vector() const;
	/// get rotation as quaternion
	quat get_rotation_quaternion() const { return quaternion; }
	/// get rotation as matrix
	mat3 get_rotation_matrix() const;
	/// set rotation from euler angles and return whether this was possible
	bool set_rotation_euler_angles(const vec3& euler_angles, const char* convention = "XYZ");
	/// set rotation from spin vector and return whether this was possible
	bool set_rotation_spin_vector(const vec3& spin_vector);
	/// set rotation from quaternion and return whether this was possible
	bool set_rotation_quaternion(const quat& _quaternion) { quaternion = _quaternion; return true; }
	/// set rotation from matrix and return whether this was possible
	bool set_rotation_matrix(const mat3& matrix);
};
	}
}

#include <cgv/config/lib_end.h>