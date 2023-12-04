#include "rotatable.h"
#include <cgv/math/quaternion.h>
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace nui {

bool is_euler_angle_convention_valid(const char* convention)
{
	for (unsigned i=0; i<2; ++i)
		if (convention[i] < 'X' || convention[i] > 'Z')
			return false;
	return true;
}
void assert_euler_angle_convention(const char* convention)
{
	assert(is_euler_angle_convention_valid(convention));
}
mat3 euler_angle_to_matrix(const vec3& euler_angles, const char* convention)
{
	assert_euler_angle_convention(convention);
	mat3 R = cgv::math::identity3<float>();
	for (unsigned i = 0; i < 3; ++i) {
		float c = cos(euler_angles[i]), s = sin(euler_angles[i]);
		unsigned j = (i + 1) % 3, k = (i + 2) % 3;
		mat3 Ri(0.0f);
		Ri(i, i) = 1.0f;
		Ri(j, j) = Ri(k, k) = c;
		Ri(j, k) = s;
		Ri(k, j) = -s;
		R *= Ri;
	}
	return R;
}
vec3 matrix_to_euler_angle(const mat3& matrix, const char* convention)
{
	assert_euler_angle_convention(convention);
	if (convention[0] != convention[2] && convention[1] != convention[2] && convention[0] != convention[1]) {
		unsigned i0 = convention[0] - 'X';
		unsigned i1 = convention[1] - 'X';
		unsigned i2 = convention[2] - 'X';
		float ax = atan2(matrix(i1, i2), matrix(i2, i2));
		float cy = sqrt(matrix(i0, i0) * matrix(i0, i0) + matrix(i0, i1) * matrix(i0, i1));
		float ay = atan2(-matrix(i0, i2), cy);
		float cx = cos(ax), sx = sin(ax);
		float az = atan2(sx * matrix(i2, i0) - cx * matrix(i1, i0), cx * matrix(i1, i1) - sx * matrix(i2, i1));
		return vec3(ax, ay, az);
	}
	std::cerr << "rotatable::matrix_to_euler_angle only implemented for permutations of XYZ convention and not for " << convention << std::endl;
	return vec3(0.0f);
}

mat4 rotatable::get_transformation_matrix() const
{
	mat3 m = get_rotation_matrix();
	return mat4(3, 3, &m(0, 0), true);
}
mat4 rotatable::get_inverse_transformation_matrix() const
{
	mat3 m = get_rotation_matrix();
	return mat4(3, 3, &m(0, 0), false);
}
vec3 euler_angle_rotatable::get_rotation_spin_vector() const
{
	vec3 axis;
	float angle = get_rotation_quaternion().put_axis(axis);
	return angle * axis;
}
quat euler_angle_rotatable::get_rotation_quaternion() const
{
	return quat(get_rotation_matrix());
}
bool euler_angle_rotatable::set_rotation_spin_vector(const vec3& spin_vector)
{
	set_rotation_matrix(cgv::math::rotate3s<float>(spin_vector));
	return true;
}
bool euler_angle_rotatable::set_rotation_quaternion(const quat& q)
{
	mat3 m;
	q.put_matrix(m);
	set_rotation_matrix(m);
	return true;
}
vec3 quaternion_rotatable::get_rotation_euler_angles(const char* convention) const
{
	return matrix_to_euler_angle(get_rotation_matrix(), convention);
}
vec3 quaternion_rotatable::get_rotation_spin_vector() const
{
	vec3 axis;
	float angle = quaternion.put_axis(axis);
	return angle * axis;
}
mat3 quaternion_rotatable::get_rotation_matrix() const
{
	mat3 M;
	quaternion.put_matrix(M);
	return M;
}
bool quaternion_rotatable::set_rotation_euler_angles(const vec3& euler_angles, const char* convention)
{
	set_rotation_matrix(euler_angle_to_matrix(euler_angles, convention));
	return true;
}
bool quaternion_rotatable::set_rotation_spin_vector(const vec3& spin_vector)
{
	set_rotation_matrix(cgv::math::rotate3s<float>(spin_vector));
	return true;
}
bool quaternion_rotatable::set_rotation_matrix(const mat3& matrix)
{
	quaternion.set(matrix);
	return true;
}
	}
}

#include <cgv/config/lib_end.h>