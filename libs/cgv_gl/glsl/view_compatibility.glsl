#version 150 compatibility

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

mat4 get_modelview_matrix()
{
	return gl_ModelViewMatrix;
}

mat4 get_modelview_projection_matrix()
{
	return gl_ModelViewProjectionMatrix;
}

mat4 get_projection_matrix()
{
	return gl_ProjectionMatrix;
}

mat4 get_inverse_modelview_matrix()
{
	return gl_ModelViewMatrixInverse;
}

mat4 get_inverse_modelview_projection_matrix()
{
	return gl_ModelViewProjectionMatrixInverse;
}

mat3 get_normal_matrix()
{
	return gl_NormalMatrix;
}

mat3 get_inverse_normal_matrix()
{
	return inverse(gl_NormalMatrix);
}
