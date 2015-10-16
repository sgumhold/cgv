#version 150 compatibility

uniform float plot_scale;
uniform vec3 x_axis;
uniform vec3 y_axis;
uniform bool x_axis_log_scale;
uniform bool y_axis_log_scale;
uniform vec2 extent;
uniform vec2 domain_min_pnt;
uniform vec2 domain_max_pnt;
uniform vec3 center_location;

vec4 map_plot_to_world(in vec2 pnt)
{
	vec2 delta = extent*(pnt - 0.5*(domain_min_pnt + domain_max_pnt) )/(domain_max_pnt - domain_min_pnt);
	vec3 p     = center_location + delta.x * x_axis + delta.y * y_axis;
	return vec4(p,1.0);
}

vec4 map_plot_to_eye(in vec2 pnt)
{
	return gl_ModelViewMatrix * map_plot_to_world(pnt);
}

vec4 map_plot_to_screen(in vec2 pnt)
{
	return gl_ModelViewProjectionMatrix * map_plot_to_world(pnt);
}