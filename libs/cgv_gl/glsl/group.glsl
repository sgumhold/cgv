#version 150

#define MAX_NR_GROUPS 8

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

#ifdef USE_GROUP_COLOR
uniform vec4 group_colors[MAX_NR_GROUPS];
#endif

#ifdef USE_GROUP_TRANSFORMATION
uniform vec3 group_translations[MAX_NR_GROUPS];
uniform vec4 group_rotations[MAX_NR_GROUPS];
#endif

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
#ifdef USE_GROUP_COLOR
	return group_colors[group_index];
#else
	return color;
#endif
}

vec3 group_transformed_position(in vec3 position, int group_index)
{
#ifdef USE_GROUP_TRANSFORMATION
		return rotate_vector_with_quaternion(position.xyz, group_rotations[group_index]) + group_translations[group_index];
#else
		return position;
#endif
}

vec3 group_transformed_normal(in vec3 nml, int group_index)
{
	// apply group rotation to normal
#ifdef USE_GROUP_TRANSFORMATION
	return rotate_vector_with_quaternion(nml, group_rotations[group_index]);
#else
		return nml;
#endif
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
#ifdef USE_GROUP_TRANSFORMATION
	// apply group rotation to normal
	right_multiply_rotation_to_matrix(NM, group_rotations[group_index]);
#endif
}

void right_multiply_group_position_matrix(inout mat4 PM, int group_index)
{
#ifdef USE_GROUP_TRANSFORMATION
	// apply group rotation to normal
	right_multiply_rigid_to_matrix(PM, group_rotations[group_index], group_translations[group_index]);
#endif
}

void right_multiply_group_normal_matrix_and_rotation(inout mat3 NM, int group_index, vec4 rotation)
{
	// apply group rotation to normal
#ifdef USE_GROUP_TRANSFORMATION
	right_multiply_rotation_to_matrix(NM, group_rotations[group_index]);
#endif
	right_multiply_rotation_to_matrix(NM, rotation);
}

void right_multiply_group_position_matrix_and_rigid(inout mat4 PM, int group_index, vec4 rotation, vec3 translation)
{
	// apply group rotation to normal
#ifdef USE_GROUP_TRANSFORMATION
	right_multiply_rigid_to_matrix(PM, group_rotations[group_index], group_translations[group_index]);
#endif
	right_multiply_rigid_to_matrix(PM, rotation, translation);
}