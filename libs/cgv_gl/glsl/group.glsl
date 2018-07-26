#version 150

/*
The following interface is implemented in this shader:
//***** begin interface of group.glsl ***********************************
vec4 group_color(in vec4 color, int group_index);
vec3 group_transformed_position(in vec3 position, int group_index);
vec3 group_transformed_normal(in vec3 nml, int group_index);
void right_multiply_group_normal_matrix(inout mat3 NM, int group_index);
void right_multiply_group_position_matrix(inout mat4 PM, int group_index);
void right_multiply_group_normal_matrix_and_rotation(inout mat3 NM, int group_index, vec4 rotation);
void right_multiply_group_position_matrix_and_rigid(inout mat4 PM, int group_index, vec4 rotation, vec3 translation);
//***** end interface of group.glsl ***********************************
*/

uniform bool use_group_color;
uniform bool use_group_transformation;
uniform vec4 group_colors[250];
uniform vec3 group_translations[250];
uniform vec4 group_rotations[250];

//***** begin interface of quaternion.glsl ***********************************
vec4 unit_quaternion();
vec3 rotate_vector_with_quaternion(in vec3 preimage, in vec4 q);
vec3 inverse_rotate_vector_with_quaternion(in vec3 v, in vec4 q);
void quaternion_to_axes(in vec4 q, out vec3 x, out vec3 y, out vec3 z);
void quaternion_to_matrix(in vec4 q, out mat3 M);
void rigid_to_matrix(in vec4 q, in vec3 t, out mat4 M);
//***** end interface of quaternion.glsl ***********************************

vec4 group_color(in vec4 color, int group_index)
{
	if (use_group_color) {
		return group_colors[group_index];
	}
	else {
		return color;
	}
}

vec3 group_transformed_position(in vec3 position, int group_index)
{
	if (use_group_transformation) {
		return rotate_vector_with_quaternion(position.xyz, group_rotations[group_index]) + group_translations[group_index];
	}
	else {
		return position;
	}
}

vec3 group_transformed_normal(in vec3 nml, int group_index)
{
	// apply group rotation to normal
	if (use_group_transformation) {
		return rotate_vector_with_quaternion(nml, group_rotations[group_index]);
	}
	else {
		return nml;
	}
}

void right_multiply_rotation_to_matrix(inout mat3 M, vec4 q)
{
	mat3 R;
	quaternion_to_matrix(q, R);
	M *= R;
}

void right_multiply_rigid_to_matrix(inout mat4 H, vec4 q, vec3 t)
{
	mat4 R;
	rigid_to_matrix(q, t, R);
	H *= R;
}

void right_multiply_group_normal_matrix(inout mat3 NM, int group_index)
{
	// apply group rotation to normal
	if (use_group_transformation) {
		right_multiply_rotation_to_matrix(NM, group_rotations[group_index]);
	}
}

void right_multiply_group_position_matrix(inout mat4 PM, int group_index)
{
	// apply group rotation to normal
	if (use_group_transformation) {
		right_multiply_rigid_to_matrix(PM, group_rotations[group_index], group_translations[group_index]);
	}
}

void right_multiply_group_normal_matrix_and_rotation(inout mat3 NM, int group_index, vec4 rotation)
{
	// apply group rotation to normal
	if (use_group_transformation) {
		right_multiply_rotation_to_matrix(NM, group_rotations[group_index]);
	}
	right_multiply_rotation_to_matrix(NM, rotation);
}

void right_multiply_group_position_matrix_and_rigid(inout mat4 PM, int group_index, vec4 rotation, vec3 translation)
{
	// apply group rotation to normal
	if (use_group_transformation) {
		right_multiply_rigid_to_matrix(PM, group_rotations[group_index], group_translations[group_index]);
	}
	right_multiply_rigid_to_matrix(PM, rotation, translation);
}