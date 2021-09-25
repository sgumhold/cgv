#version 450

/*
The following interface is implemented in this shader:
//***** begin interface of shape2d.glsl ***********************************
uniform ivec2 resolution;
uniform mat3 modelview2d_matrix = mat3(1.0);

uniform vec4 color;
uniform vec4 border_color = vec4(1.0);
uniform float border_width = 0.0;
uniform float border_radius = 0.0;
uniform float ring_width = 0.0;
uniform float feather_width = 1.0;
uniform float feather_origin = 0.5;
uniform vec2 tex_scaling = vec2(1.0);

uniform bool use_color = true;
uniform bool use_blending = false;
uniform bool use_smooth_feather = false;
uniform bool apply_gamma = true;

float get_feather_width();
vec2 get_adjusted_size(vec2 size);
vec4 get_color();
void override_color(in vec4 color);
vec4 transform_world_to_window_space(vec2 p);
//***** end interface of shape2d.glsl ***********************************
*/

// canvas parameters
uniform ivec2 resolution;
uniform mat3 modelview2d_matrix = mat3(1.0);
uniform float feather_scale = 1.0;

// appearance
uniform vec4 color;
uniform vec4 border_color = vec4(1.0);
uniform float border_width = 0.0;
uniform float border_radius = 0.0;
uniform float ring_width = 0.0;
uniform float feather_width = 1.0;
uniform float feather_origin = 0.5;
uniform vec2 tex_scaling = vec2(1.0);

// render options
uniform bool use_color = true;
uniform bool use_blending = false;
uniform bool use_smooth_feather = false;
uniform bool apply_gamma = true;

// local variables
vec4 final_color = color;
bool force_use_color = use_color;

// returns the adjusted feather width
float get_feather_width() {
	return feather_scale * feather_width;
}

// return the adjusted size of the shape, which is half the actual size minus the border radius plus feather sizes
vec2 get_adjusted_size(vec2 size) {
	return 0.5 * size - border_radius + feather_origin*get_feather_width();
}

// return the current color that will be used when use_color is enabled
vec4 get_color() {
	return final_color;
}

// override the color that will be used when use_color is enabled
void override_color(in vec4 color) {
	final_color = color;
}

// transform a world space position into window space
vec4 transform_world_to_window_space(vec2 p) {
	// apply modelview transformation
	vec2 pos = (modelview2d_matrix * vec3(p, 1.0)).xy;

	// transform to window space
	pos = (2.0*pos) / resolution;
	return vec4(pos - 1.0, 0.0, 1.0);
}
