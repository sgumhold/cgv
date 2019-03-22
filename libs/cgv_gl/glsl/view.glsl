#version 150

/*
The following interface is implemented in this shader:
//***** begin interface of view.glsl ***********************************
mat4 get_modelview_matrix();
mat4 get_projection_matrix();
mat4 get_modelview_projection_matrix();
mat4 get_inverse_modelview_matrix();
mat4 get_inverse_modelview_projection_matrix();
mat3 get_normal_matrix();
mat3 get_inverse_normal_matrix();
//***** end interface of view.glsl ***********************************
*/

uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;
uniform mat3 normal_matrix;

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

mat4 get_inverse_modelview_matrix()
{
	return inverse(get_modelview_matrix());
}

mat4 get_inverse_modelview_projection_matrix()
{
	return inverse(get_modelview_projection_matrix());
}

mat3 get_normal_matrix()
{
	return normal_matrix;
}

mat3 get_inverse_normal_matrix()
{
	return inverse(get_normal_matrix());
}
