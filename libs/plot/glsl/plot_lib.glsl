#version 150

/*
The following interface is implemented in this shader:
//***** begin interface of plot_lib.glsl ***********************************
uniform vec3 domain_min_pnt;
uniform vec3 domain_max_pnt;
uniform vec3 extent;
uniform vec4 orientation;
uniform vec3 center_location;
vec3 map_plot_to_plot3(in vec2 pnt);
vec4 map_plot_to_world(in vec2 pnt);
vec4 map_plot_to_eye(in vec2 pnt);
vec4 map_plot_to_screen(in vec2 pnt);
vec3 map_plot_to_plot3(in vec3 pnt);
vec4 map_plot_to_world3(in vec3 pnt);
vec4 map_plot_to_eye3(in vec3 pnt);
vec4 map_plot_to_screen3(in vec3 pnt);
//***** end interface of plot_lib.glsl ***********************************
*/


//***** begin interface of view.glsl ***********************************
mat4 get_modelview_matrix();
mat4 get_projection_matrix();
mat4 get_modelview_projection_matrix();
mat4 get_inverse_modelview_matrix();
mat4 get_inverse_modelview_projection_matrix();
mat3 get_normal_matrix();
mat3 get_inverse_normal_matrix();
//***** end interface of view.glsl ***********************************

//***** begin interface of quaternion.glsl ***********************************
vec4 unit_quaternion();
vec3 rotate_vector_with_quaternion(in vec3 preimage, in vec4 q);
vec3 inverse_rotate_vector_with_quaternion(in vec3 v, in vec4 q);
void quaternion_to_axes(in vec4 q, out vec3 x, out vec3 y, out vec3 z);
void quaternion_to_matrix(in vec4 q, out mat3 M);
void rigid_to_matrix(in vec4 q, in vec3 t, out mat4 M);
//***** end interface of quaternion.glsl ***********************************

uniform bool x_axis_log_scale = false;
uniform bool y_axis_log_scale = false;
uniform bool z_axis_log_scale = false;
uniform vec3 domain_min_pnt;
uniform vec3 domain_max_pnt;
uniform vec3 extent;
uniform vec4 orientation = vec4(0.0, 0.0, 0.0, 1.0);
uniform vec3 center_location;

float convert_to_log_space(float val, float min_val, float max_val)
{
	return log(max(val, 1e-6f*max_val)) / log(10.0);
}

float compute_delta(float val, float min_val, float max_val, bool log_scale)
{
	if (log_scale) {
		float log_val     = convert_to_log_space(val, min_val, max_val);
		float log_min_val = convert_to_log_space(min_val, min_val, max_val);
		float log_max_val = convert_to_log_space(max_val, min_val, max_val);
		return (log_val - 0.5*(log_min_val + log_max_val)) / (log_max_val - log_min_val);
	}
	else
		return (val - 0.5*(min_val+max_val)) / (max_val - min_val);
}

vec3 map_plot_to_plot3(in vec2 pnt)
{
	return extent * vec3(
		compute_delta(pnt.x, domain_min_pnt.x, domain_max_pnt.x, x_axis_log_scale),
		compute_delta(pnt.y, domain_min_pnt.y, domain_max_pnt.y, y_axis_log_scale), 0.0);
}

vec4 map_plot_to_world(in vec2 pnt)
{
	return vec4(center_location + rotate_vector_with_quaternion(map_plot_to_plot3(pnt), orientation), 1.0);
}

vec4 map_plot_to_eye(in vec2 pnt)
{
	return get_modelview_matrix() * map_plot_to_world(pnt);
}

vec4 map_plot_to_screen(in vec2 pnt)
{
	return get_modelview_projection_matrix() * map_plot_to_world(pnt);
}

vec3 map_plot_to_plot3(in vec3 pnt)
{
	return extent * vec3(
		compute_delta(pnt.x, domain_min_pnt.x, domain_max_pnt.x, x_axis_log_scale),
		compute_delta(pnt.y, domain_min_pnt.y, domain_max_pnt.y, y_axis_log_scale),
		compute_delta(pnt.z, domain_min_pnt.z, domain_max_pnt.z, z_axis_log_scale));
}

vec4 map_plot_to_world3(in vec3 pnt)
{
	return vec4(center_location + rotate_vector_with_quaternion(map_plot_to_plot3(pnt), orientation), 1.0);
}

vec4 map_plot_to_eye3(in vec3 pnt)
{
	return get_modelview_matrix() * map_plot_to_world3(pnt);
}

vec4 map_plot_to_screen3(in vec3 pnt)
{
	return get_modelview_projection_matrix() * map_plot_to_world3(pnt);
}

