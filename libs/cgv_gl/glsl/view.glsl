#version 150

/*
The following interface is implemented in this shader:
//***** begin interface of view.glsl ***********************************
mat4 get_modelview_matrix();
mat4 get_projection_matrix();
mat4 get_inverse_projection_matrix();
mat4 get_modelview_projection_matrix();
vec3 get_eye_world();
mat4 get_inverse_modelview_matrix();
mat4 get_inverse_modelview_projection_matrix();
mat3 get_normal_matrix();
mat3 get_inverse_normal_matrix();
//***** end interface of view.glsl ***********************************
*/

uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;
uniform mat4 inverse_projection_matrix;
uniform mat3 normal_matrix;
uniform mat4 inverse_modelview_matrix;
uniform mat3 inverse_normal_matrix;

mat4 get_modelview_matrix()
{
	return modelview_matrix;
}

mat4 get_modelview_projection_matrix()
{
	return projection_matrix*modelview_matrix;
}

mat4 get_projection_matrix()
{
	return projection_matrix;
}

mat4 get_inverse_projection_matrix()
{
	return inverse_projection_matrix;
}

mat4 get_inverse_modelview_matrix()
{
	return inverse_modelview_matrix;
}

vec3 get_eye_world()
{
	vec4 heye = get_inverse_modelview_matrix()[3];
	return heye.xyz / heye.w;
}

mat4 get_inverse_modelview_projection_matrix()
{
	return inverse_modelview_matrix * inverse_projection_matrix;
}

mat3 get_normal_matrix()
{
	return normal_matrix;
}

mat3 get_inverse_normal_matrix()
{
	return inverse_normal_matrix;
}
