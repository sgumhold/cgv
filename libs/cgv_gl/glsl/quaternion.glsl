#version 150 compatibility

/*
The following interface is implemented in this shader:
//***** begin interface of quaternion.glsl ***********************************
vec4 unit_quaternion();
vec3 rotate_vector_with_quaternion(in vec3 preimage, in vec4 q);
vec3 inverse_rotate_vector_with_quaternion(in vec3 v, in vec4 q);
void quaternion_to_axes(in vec4 q, out vec3 x, out vec3 y, out vec3 z);
void quaternion_to_matrix(in vec4 q, out mat3 M);
void rigid_to_matrix(in vec4 q, in vec3 t, out mat4 M);
//***** end interface of quaternion.glsl ***********************************
*/

vec4 unit_quaternion()
{
	return vec4(0.0, 0.0, 0.0, 1.0);
}

vec3 rotate_vector_with_quaternion(in vec3 preimage, in vec4 q)
{
	vec3 image = cross(q.xyz, preimage);
	return dot(preimage, q.xyz)*q.xyz + q.w*(q.w*preimage + 2.0*image) + cross(q.xyz,image);
}

vec3 inverse_rotate_vector_with_quaternion(in vec3 v, in vec4 q)
{
	vec3 tmp = cross(-q.xyz, v);
	return dot(v, -q.xyz)*(-q.xyz) + q.w*(q.w*v + 2.0*tmp) + cross(-q.xyz,tmp);
}

void quaternion_to_axes(in vec4 q, out vec3 x, out vec3 y, out vec3 z)
{
	x = vec3(1.0-2.0*q.y*q.y-2.0*q.z*q.z, 2.0*q.x*q.y + 2.0*q.w*q.z, 2.0*q.x*q.z - 2.0*q.w*q.y);
	y = vec3(2.0*q.x*q.y - 2.0*q.w*q.z, 1.0-2.0*q.x*q.x-2.0*q.z*q.z, 2.0*q.y*q.z + 2.0*q.w*q.x);
	z = vec3(2.0*q.x*q.z + 2.0*q.w*q.y, 2.0*q.y*q.z - 2.0*q.w*q.x, 1.0-2.0*q.x*q.x-2.0*q.y*q.y);
}

void quaternion_to_matrix(in vec4 q, out mat3 M)
{
	quaternion_to_axes(q, M[0], M[1], M[2]);
}

void rigid_to_matrix(in vec4 q, in vec3 t, out mat4 M)
{
	vec3 x, y, z;
	quaternion_to_axes(q, x, y, z);
	M[0] = vec4(x, 0.0);
	M[1] = vec4(y, 0.0);
	M[2] = vec4(z, 0.0);
	M[3] = vec4(t, 1.0);
}