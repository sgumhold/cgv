#version 150

/*
The following interface is implemented in this shader:
//***** begin interface of sphere_lib.glvs ***********************************
void output_sphere_parameter_space(in vec4 sphere, in mat4 MV, in mat4 iMV, in mat4 MVP, in mat3 NM);
//***** end interface of sphere_lib.glvs ***********************************
*/

struct sphere_parameter_space
{
	vec3  m_tilde;
	vec3  x_tilde;
	vec3  y_tilde;
	vec3  e_tilde;
	float inv_e_vs;
	vec3  inv_T_square_e_c_vs;
	vec2  e_zw_clip_vs;
};

out sphere_parameter_space sps;

void output_sphere_parameter_space(in vec4 sphere, in mat4 MV, in mat4 iMV, in mat4 MVP, in mat3 NM)
{
	// compute radius and reciprocal radius
	float inv_R = 1.0 / sphere.w;

	// determine eye point in parameter space
	vec3 e = iMV[3].xyz;
	sps.e_tilde = inv_R * (e - sphere.xyz);

	// compute helper
	float inv_e_square = 1.0 / dot(sps.e_tilde, sps.e_tilde);

	// determine silhoutte center in parameter space
	sps.m_tilde = inv_e_square * sps.e_tilde;

	// determine radius of silhouette in parameter space
	float r = sqrt(1.0 - inv_e_square);

	// compute vector x of length r orthogonal to e in parameter space
	sps.x_tilde = r * normalize(cross(iMV[1].xyz, sps.e_tilde));

	// compute vector y of length r orthogonal to x and e in parameter space
	sps.y_tilde = r * normalize(cross(sps.e_tilde, sps.x_tilde));

	sps.inv_e_vs = sqrt(inv_e_square);

	// compute components to compute normal in eye space
	sps.inv_T_square_e_c_vs = NM * (sps.e_tilde);

	// constant part of depth value
	sps.e_zw_clip_vs = (MVP * iMV[3]).zw;
}
