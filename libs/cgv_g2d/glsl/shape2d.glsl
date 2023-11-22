#version 430

/*
The following interface is implemented in this shader:
//***** begin interface of shape2d.glsl ***********************************
uniform ivec2 resolution;
uniform mat3 modelview2d_matrix = mat3(1.0);
uniform bool origin_upper_left = false;
uniform float zoom_factor = 1.0;

uniform vec4 fill_color;
uniform vec4 border_color = vec4(1.0);
uniform float border_width = 0.0;
uniform float border_radius = 0.0;
uniform float ring_width = 0.0;
uniform float feather_width = 1.0;
uniform float feather_origin = 0.5;
uniform vec2 tex_scaling = vec2(1.0);

uniform bool use_fill_color = true;
uniform bool use_texture = false;
uniform bool use_texture_alpha = true;
uniform bool use_blending = false;
uniform bool use_smooth_feather = false;
uniform bool apply_gamma = true;

float get_feather_width();
vec2 get_adjusted_size(vec2 size);
vec4 get_active_color(vec4 color);
vec4 transform_world_to_window_space(vec2 p);
//***** end interface of shape2d.glsl ***********************************
*/

// canvas parameters
uniform ivec2 resolution;
uniform mat3 modelview2d_matrix = mat3(1.0);
uniform bool origin_upper_left = false;
uniform float zoom_factor = 1.0;

// appearance
uniform vec4 fill_color;
uniform vec4 border_color = vec4(1.0);
uniform float border_width = 0.0;
uniform float border_radius = 0.0;
uniform float ring_width = 0.0;
uniform float feather_width = 1.0;
uniform float feather_origin = 0.5;
uniform vec2 tex_scaling = vec2(1.0);
uniform vec2 tex_offset = vec2(0.0);

// render options
uniform bool use_fill_color = true;
uniform bool use_texture = false;
uniform bool use_texture_alpha = true;
uniform bool use_blending = false;
uniform bool use_smooth_feather = false;
uniform bool apply_gamma = true;

// returns the adjusted feather width
float get_feather_width() {
	return feather_width / zoom_factor;
}

// return the adjusted size of the shape, which is half the actual size minus the border radius plus feather sizes
vec2 get_adjusted_size(vec2 size) {
	return 0.5 * size - border_radius + feather_origin*get_feather_width();
}

// return the active color: fill color if use fill color is true and color otherwise
vec4 get_active_color(vec4 color) {
	return use_fill_color ? fill_color : color;
}

// transform a world space position into window space
vec4 transform_world_to_window_space(vec2 p) {
	// apply modelview transformation
	vec2 pos = (modelview2d_matrix * vec3(p, 1.0)).xy;

	// transform to window space
	pos /= resolution;
	if(origin_upper_left)
		pos.y = 1.0 - pos.y;
	return vec4(2.0 * pos - 1.0, 0.0, 1.0);
}
