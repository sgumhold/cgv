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
	vec2 delta;
	if (x_axis_log_scale) {
		delta.x = extent.x * (log(pnt.x) - 0.5*(log(domain_min_pnt.x) + log(domain_max_pnt.x))) / (log(domain_max_pnt.x) - log(domain_min_pnt.x));
	}
	else {
		delta.x = extent.x * (pnt.x - 0.5*(domain_min_pnt.x + domain_max_pnt.x)) / (domain_max_pnt.x - domain_min_pnt.x);
	}
	if (y_axis_log_scale) {
		delta.y = extent.y * (log(pnt.y) - 0.5*(log(domain_min_pnt.y) + log(domain_max_pnt.y))) / (log(domain_max_pnt.y) - log(domain_min_pnt.y));
	}
	else {
		delta.y = extent.y * (pnt.y - 0.5*(domain_min_pnt.y + domain_max_pnt.y)) / (domain_max_pnt.y - domain_min_pnt.y);
	}
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