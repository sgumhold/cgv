#version 330 core

#define CGV_COLOR_SCALE_MAX_COLOR_SCALE_COUNT 4
#define CGV_COLOR_SCALE_ZERO_THRESHOLD 0.000001
#define CGV_COLOR_SCALE_TRANSFORM_LINEAR 0
#define CGV_COLOR_SCALE_TRANSFORM_POW 1
#define CGV_COLOR_SCALE_TRANSFORM_LOG 2

/*
The following interface is implemented in this shader:
//***** begin interface of color_scale.glsl ***********************************
/// gamma adjust value after clamping to [0,1] and in case of uniform color_scale_is_bi_polar[0] accounting for uniform window_zero_position[0]
float color_scale_gamma_mapping(in float v, in float gamma);
/// gamma adjust value after clamping to [0,1] and in case of uniform color_scale_is_bi_polar[idx] accounting for uniform window_zero_position[idx]
float color_scale_gamma_mapping(in float v, in float gamma, int idx);
/// map value with color scale selected in uniform color_scale_index[idx=0|1] to rgb color
vec3 color_scale(in float v, int idx);
/// map value with color scale selected in uniform color_scale_index[0] to rgb color
vec3 color_scale(in float v);
//***** end interface of color_scale.glsl ***********************************
*/



uniform int    color_scale_index[2] = int[2]( 6, 7 );
uniform vec3   color_scale_samples[64];
uniform int    nr_color_scale_samples[2];
uniform int    color_scale_is_bipolar[2] = int[2]( 0, 0 );
uniform float  window_zero_position[2] = float[2]( 0.5, 0.5 );
uniform bool   adjust_asymmetric = false;





uniform sampler2D color_scale_texture;

struct ColorScaleArguments {
	// general arguments
	vec4 unknown_color;
	vec2 domain;
	bool clamped;
	// specific arguments
	bool diverging;
	float midpoint;
	float exponent;
	int transform;
};

uniform ColorScaleArguments color_scale_arguments[CGV_COLOR_SCALE_MAX_COLOR_SCALE_COUNT];

float color_scale_map_range_safe(in float value, in float in_left, in float in_right, in float out_left, in float out_right) {
	float size = in_right - in_left;
	if(abs(size) < CGV_COLOR_SCALE_ZERO_THRESHOLD)
		return out_left;
	return out_left + (out_right - out_left) * ((value - in_left) / size);
}

float color_scale_map_value(in float value, in ColorScaleArguments arguments) {
	vec2 domain = arguments.domain;
	value = clamp(value, domain.x, domain.y);

	float t = 0.0;

	// Todo: Use separate functions for transform types.
	switch(arguments.transform) {
	case CGV_COLOR_SCALE_TRANSFORM_LINEAR:
	{
		if(arguments.diverging) {
			if(value < arguments.midpoint)
				t = color_scale_map_range_safe(value, domain.x, arguments.midpoint, 0.0, 0.5);
			else
				t = color_scale_map_range_safe(value, arguments.midpoint, domain.y, 0.5, 1.0);
		} else {
			t = color_scale_map_range_safe(value, domain.x, domain.y, 0.0, 1.0);
		}
		break;
	}
	case CGV_COLOR_SCALE_TRANSFORM_POW:
	{
		if(arguments.diverging) {
			if(value < arguments.midpoint) {
				t = color_scale_map_range_safe(value, domain.x, arguments.midpoint, 0.0, 1.0);
				t = 0.5 * (1.0 - pow(1.0 - t, arguments.exponent));
			} else {
				t = color_scale_map_range_safe(value, arguments.midpoint, domain.y, 0.0, 1.0);
				t = 0.5 * pow(t, arguments.exponent) + 0.5;
			}
		} else {
			t = color_scale_map_range_safe(value, domain.x, domain.y, 0.0, 1.0);
			t = pow(t, arguments.exponent);
		}
		break;
	}
	case CGV_COLOR_SCALE_TRANSFORM_LOG:
		// Todo: Implement log and symlog (for diverging mapping) transform.
		break;
	}

	return t;
}

vec4 color_scale_sample_texture_continuous(in int index, in float t) {
	vec2 texture_size = vec2(textureSize(color_scale_texture, 0));
	vec2 texel_size = (1.0 / vec2(texture_size));
	float color_scale_offset = float(index) / texture_size.y + 0.5 * texel_size.y;
	return texture(color_scale_texture, vec2(t, color_scale_offset));
}

vec4 color_scale_sample_texture_discrete(in int index, in float x) {
	// Todo: Implement.
	return vec4(0.0);
}

vec4 evaluate_color_scale(in int index, in float value) {
	ColorScaleArguments arguments = color_scale_arguments[index];

	if(!arguments.clamped && (value < arguments.domain.x || value > arguments.domain.y))
		return arguments.unknown_color;

	float t = color_scale_map_value(value, arguments);
	vec4 color = color_scale_sample_texture_continuous(index, t);
	// Todo: Linearize colors?
	//color.rgb = pow(color.rgb, vec3(2.2));
	return color;
}

// Todo: Remove these functions. They are only temporary to allow plot shaders to work with new color scales.
vec3 color_scale(in float v, int idx) {
	return evaluate_color_scale(idx, v).rgb;
}

vec3 color_scale(in float v) {
	return evaluate_color_scale(0, v).rgb;
}

float color_scale_gamma_mapping(in float v, in float gamma, int idx)
{
	if (color_scale_is_bipolar[idx] != 0) {
		float z = window_zero_position[idx];
		float a = max(z, 1.0-z);
		if (adjust_asymmetric) {
			if (v > z || abs(z) < 0.00001)
				a = 1.0 - z;
			else
				a = z;
		}
		if (v < z)
			return z-a*pow((z-v)/a,gamma);
		else
			return z+a*pow((v-z)/a,gamma);
	}
	else
		return pow(v, gamma);
}

float color_scale_gamma_mapping(in float v, in float gamma)
{
	return color_scale_gamma_mapping(v, gamma, 0);
}

void adjust_zero_position(inout float v, in float window_zero)
{
	if (adjust_asymmetric) {
		if (abs(window_zero) < 0.00001 || v > window_zero)
			v = 1.0 - 0.5 * (1.0 - v) / (1.0 - window_zero);
		else
			v *= 0.5 / window_zero;
		return;
	}
	// map v according to scale*v + offset to a new value such that attribute_zero_position maps to 0.5 
	// and in case attribute_zero_position <= 0.5 v=1.0 maps to 1.0 and otherwise v=0.0 maps to 0.0
	if (window_zero <= 0.5)
		v = 1.0 - 0.5 * (1.0 - v) / (1.0 - window_zero);
	else
		v *= 0.5 / window_zero;
}

