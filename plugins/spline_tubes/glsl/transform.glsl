#version 330 core

//////
//
// Shader module interface definition
//

///***** begin interface of transform.glsl ************************************/
vec3   ortho_vec (const vec3 v);
void   make_orthonormal_basis (out vec3 e0, out vec3 e1, const vec3 ref);
mat2x3 make_orthonormal_basis (const vec3 ref);
void   make_orthonormal_system_x (out mat3 M, const vec3 ref);
mat3   make_orthonormal_system_x (const vec3 ref);
void   make_orthonormal_system_y (out mat3 M, const vec3 ref);
mat3   make_orthonormal_system_y (const vec3 ref);
void   make_orthonormal_system_z (out mat3 M, const vec3 ref);
mat3   make_orthonormal_system_z (const vec3 ref);
void   make_local_frame (out mat4 M, const vec3 x, const vec3 y, const vec3 z, const vec3 o);
mat4   make_local_frame (const vec3 x, const vec3 y, const vec3 z, const vec3 o);
///***** end interface of transforms.glsl *************************************/



//////
//
// Functions
//

// Finds some vector that is guaranteed to be orthogonal to the given 3D vector.
vec3 ortho_vec (const vec3 v) {
	float k = fract(abs(v.x) + .5);
	return vec3(-v.y, v.x - k*v.z, k*v.y);
}

// Finds two 3D unit vectors that are orthogonal to each other and the given 3D reference unit vector.
void make_orthonormal_basis (out vec3 e0, out vec3 e1, const vec3 ref) {
	e1 = normalize(ortho_vec(ref));
	e0 = cross(e1, ref);
}

// Finds two 3D unit vectors that are orthogonal to each other and the given 3D reference unit vector.
mat2x3 make_orthonormal_basis (const vec3 ref) {
	vec3 e1 = normalize(ortho_vec(ref));
	return mat2x3(e1, cross(e1, ref));
}

// Creates a system transformation into an oriented space spanned by the given 3D reference unit vector as x-axis and two
// to-be-calculated 3D unit vectors orthogonal to it as y- and z-axes.
void make_orthonormal_system_x (out mat3 M, const vec3 ref)
{
	vec3 e1 = normalize(ortho_vec(ref)),
	     e0 = cross(e1, ref);
	M[0] = vec3(ref.x, e0.x, e1.x);
	M[1] = vec3(ref.y, e0.y, e1.y);
	M[2] = vec3(ref.z, e0.z, e1.z);
}

// Creates a system transformation into an oriented space spanned by the given 3D reference unit vector as x-axis and two
// to-be-calculated 3D unit vectors orthogonal to it as y- and z-axes.
mat3 make_orthonormal_system_x (const vec3 ref)
{
	vec3 e1 = normalize(ortho_vec(ref)),
	     e0 = cross(e1, ref);
	return mat3(
		vec3(ref.x, e0.x, e1.x), vec3(ref.y, e0.y, e1.y), vec3(ref.z, e0.z, e1.z)
	);
}

// Creates a system transformation into an oriented space spanned by the given 3D reference unit vector as y-axis and two
// to-be-calculated 3D unit vectors orthogonal to it as x- and z-axes.
void make_orthonormal_system_y (out mat3 M, const vec3 ref)
{
	vec3 e1 = normalize(ortho_vec(ref)),
	     e0 = cross(e1, ref);
	M[0] = vec3(e0.x, ref.x, e1.x);
	M[1] = vec3(e0.y, ref.y, e1.y);
	M[2] = vec3(e0.z, ref.z, e1.z);
}

// Creates a system transformation into an oriented space spanned by the given 3D reference unit vector as y-axis and two
// to-be-calculated 3D unit vectors orthogonal to it as x- and z-axes.
mat3 make_orthonormal_system_y (const vec3 ref)
{
	vec3 e1 = normalize(ortho_vec(ref)),
	     e0 = cross(e1, ref);
	return mat3(
		vec3(e0.x, ref.x, e1.x), vec3(e0.y, ref.y, e1.y), vec3(e0.z, ref.z, e1.z)
	);
}

// Creates a system transformation into an oriented space spanned by the given 3D reference unit vector as z-axis and two
// to-be-calculated 3D unit vectors orthogonal to it as x- and y-axes.
void make_orthonormal_system_z (out mat3 M, const vec3 ref)
{
	vec3 e1 = normalize(ortho_vec(ref)),
	     e0 = cross(e1, ref);
	M[0] = vec3(e0.x, e1.x, ref.x);
	M[1] = vec3(e0.y, e1.y, ref.y);
	M[2] = vec3(e0.z, e1.z, ref.z);
}

// Creates a system transformation into an oriented space spanned by the given 3D reference unit vector as z-axis and two
// to-be-calculated 3D unit vectors orthogonal to it as x- and y-axes.
mat3 make_orthonormal_system_z (const vec3 ref)
{
	vec3 e1 = normalize(ortho_vec(ref)),
	     e0 = cross(e1, ref);
	return mat3(
		vec3(e0.x, e1.x, ref.x), vec3(e0.y, e1.y, ref.y), vec3(e0.z, e1.z, ref.z)
	);
}

// Calculates a system transformation into the space defined by the given origin and basis vectors.
void make_local_frame (out mat4 M, const vec3 x, const vec3 y, const vec3 z, const vec3 o)
{
	M[0] = vec4(x.x, y.x, z.x, 0);
	M[1] = vec4(x.y, y.y, z.y, 0);
	M[2] = vec4(x.z, y.z, z.z, 0);
	M[3] = vec4(-dot(x, o), -dot(y, o), -dot(z, o), 1);
}

// Calculates a system transformation into the space defined by the given origin and basis vectors.
mat4 make_local_frame (const vec3 x, const vec3 y, const vec3 z, const vec3 o) {
	return mat4(
		vec4(x.x, y.x, z.x, 0),  vec4(x.y, y.y, z.y, 0), vec4(x.z, y.z, z.z, 0),
		vec4(-dot(x, o), -dot(y, o), -dot(z, o), 1)
	);
}
