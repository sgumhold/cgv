#version 330 core

//////
//
// Shader module interface definition
//

///***** begin interface of rcc.glsl ******************************************/
// --- link dependencies ---------------
/* transform.glsl */
// --- structs -------------------------
// Representation of a ray-centric coordinate system
struct RCC {
	// The model transformation from ray-centric into whatever reference coordiante system was used.
	mat4 model;
	// The system transformation from whatever reference coordiante system was used into ray-centric coordinates.
	mat4 system;
	// The index of the coordiante axis that coincides with the ray.
	int axis;
	// The indices of the first and second non-ray axes.
	int non_axes[2];
};
// --- functions -----------------------
mat4   rcc_calc_model_transform_x (const vec3 orig, const vec3 dir);
mat4   rcc_calc_system_transform_x (const vec3 orig, const vec3 dir);
mat4   rcc_calc_model_transform_y (const vec3 orig, const vec3 dir);
mat4   rcc_calc_system_transform_y (const vec3 orig, const vec3 dir);
mat4   rcc_calc_model_transform_z (const vec3 orig, const vec3 dir);
mat4   rcc_calc_system_transform_z (const vec3 orig, const vec3 dir);
RCC    rcc_create_x (const vec3 orig, const vec3 dir);
RCC    rcc_create_y (const vec3 orig, const vec3 dir);
RCC    rcc_create_z (const vec3 orig, const vec3 dir);
///***** end interface of rcc.glsl ********************************************/



//////
//
// Private imports
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

// calculate the ray-centric coordinate system model transformation for the given ray with the ray direction
// (assumed to be normalized) coinciding with the local x-axis.
mat4 rcc_calc_model_transform_x (const vec3 orig, const vec3 dir)
{
	mat4 M;
	M[0] = vec4(dir, 0);
	make_orthonormal_basis(M[1].xyz, M[2].xyz, dir);
	M[2].w = M[1].w = 0;
	M[3] = vec4(orig, 1);
	return M;
}

// calculate the ray-centric coordinate system transformation for the given ray with the ray direction
// (assumed to be normalized) coinciding with the local x-axis.
mat4 rcc_calc_system_transform_x (const vec3 orig, const vec3 dir) {
	mat2x3 axes = make_orthonormal_basis(dir);
	return make_local_frame(dir, axes[0], axes[1], orig);
}

// calculate the ray-centric coordinate system model transformation for the given ray with the ray direction
// (assumed to be normalized) coinciding with the local y-axis.
mat4 rcc_calc_model_transform_y (const vec3 orig, const vec3 dir)
{
	mat4 M;
	M[0] = vec4(dir, 0);
	make_orthonormal_basis(M[0].xyz, M[2].xyz, dir);
	M[2].w = M[0].w = 0;
	M[3] = vec4(orig, 1);
	return M;
}

// calculate the ray-centric coordinate system transformation for the given ray with the ray direction
// (assumed to be normalized) coinciding with the local y-axis.
mat4 rcc_calc_system_transform_y (const vec3 orig, const vec3 dir) {
	mat2x3 axes = make_orthonormal_basis(dir);
	return make_local_frame(axes[0], dir, axes[1], orig);
}

// calculate the ray-centric coordinate system model transformation for the given ray with the ray direction
// (assumed to be normalized) coinciding with the local z-axis.
mat4 rcc_calc_model_transform_z (const vec3 orig, const vec3 dir)
{
	mat4 M;
	M[0] = vec4(dir, 0);
	make_orthonormal_basis(M[0].xyz, M[1].xyz, dir);
	M[1].w = M[0].w = 0;
	M[3] = vec4(orig, 1);
	return M;
}

// calculate the ray-centric coordinate system transformation for the given ray with the ray direction
// (assumed to be normalized) coinciding with the local z-axis.
mat4 rcc_calc_system_transform_z (const vec3 orig, const vec3 dir) {
	mat2x3 axes = make_orthonormal_basis(dir);
	return make_local_frame(axes[0], axes[1], dir, orig);
}

// Computes a ray-centric coordinate system for the given ray where the ray direction (assumed to be
// normalized) coincides with the local x-axis.
RCC rcc_create_x (const vec3 orig, const vec3 dir)
{
	RCC rcc;
	rcc.axis = 0; rcc.non_axes[0] = 1; rcc.non_axes[1] = 2;
	rcc.model = rcc_calc_model_transform_x(orig, dir);
	make_local_frame(rcc.system, dir, rcc.model[1].xyz, rcc.model[2].xyz, orig);
	return rcc;
}

// Computes a ray-centric coordinate system for the given ray where the ray direction (assumed to be
// normalized) coincides with the local y-axis.
RCC rcc_create_y (const vec3 orig, const vec3 dir)
{
	RCC rcc;
	rcc.axis = 1; rcc.non_axes[0] = 0; rcc.non_axes[1] = 2;
	rcc.model = rcc_calc_model_transform_y(orig, dir);
	make_local_frame(rcc.system, rcc.model[0].xyz, dir, rcc.model[2].xyz, orig);
	return rcc;
}

// Computes a ray-centric coordinate system for the given ray where the ray direction (assumed to be
// normalized) coincides with the local z-axis.
RCC rcc_create_z (const vec3 orig, const vec3 dir)
{
	RCC rcc;
	rcc.axis = 2; rcc.non_axes[0] = 0; rcc.non_axes[1] = 1;
	rcc.model = rcc_calc_model_transform_z(orig, dir);
	make_local_frame(rcc.system, rcc.model[0].xyz, rcc.model[1].xyz, dir, orig);
	return rcc;
}
