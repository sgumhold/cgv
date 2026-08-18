#version 420 core

#define CGV_COLOR_SCALE_MAX_COLOR_SCALE_COUNT 4
#define CGV_COLOR_SCALE_ZERO_THRESHOLD 0.000001
#define CGV_COLOR_SCALE_SAMPLE_MODE_DISCRETE_FLAG 0x01000000
#define CGV_COLOR_SCALE_CLAMPED_FLAG 0x00010000
#define CGV_COLOR_SCALE_DIVERGING_FLAG 0x00020000
#define CGV_COLOR_SCALE_TRANSFORM_MASK 0xFFFF
#define CGV_COLOR_SCALE_TRANSFORM_LINEAR 0
#define CGV_COLOR_SCALE_TRANSFORM_POW 1
#define CGV_COLOR_SCALE_TRANSFORM_LOG 2

/*
The following interface is implemented in this shader:
//***** begin interface of color_scale.glsl ***********************************
struct ColorScaleArguments {
	vec2 domain;
	uint unknown_color;
	float midpoint;
	float exponent;
	float log_base;
	float log_midpoint;
	float log_lower_bound;
	float log_upper_bound;
	float log_sign;
	uint flags;
	int indexed_color_count;
};

/// map value from range [in_left,in_right] to [out_left,out_right] with correct handling of edge-case where the input range is empty
float color_scale_map_range_safe(in float value, in float in_left, in float in_right, in float out_left, in float out_right);
/// map value from the color scale domain to [0,1] using the given mapping arguments
float color_scale_map_value(in float value, in ColorScaleArguments arguments);
/// return linearly interpolated color of indexed color scale sampled at position t in [0,1]; no mapping is applied
vec4 color_scale_sample_texture_continuous(in int index, in float t);
/// return nearest color of indexed discrete color scale with size sampled at position t in [0,1]; no mapping is applied
vec4 color_scale_sample_texture_discrete(in int index, in int size, in float t);
/// map value to rgba color and opacity through color scale given by index
vec4 evaluate_color_scale(in int index, in float value);
//***** end interface of color_scale.glsl ***********************************
*/

uniform sampler2D color_scale_texture;

struct ColorScaleArguments {
	vec2 domain;
	uint unknown_color; // packed 8-bit per channel rgba
	float midpoint;
	float exponent;
	float log_base;
	float log_midpoint;
	float log_lower_bound;
	float log_upper_bound;
	float log_sign;
	uint flags; // properties in 4 bytes using layout [1,1,2] bytes encode sample_mode | mapping_options | transform
	int indexed_color_count;
};

layout(std140) uniform color_scale_argument_block {
	ColorScaleArguments color_scale_arguments[CGV_COLOR_SCALE_MAX_COLOR_SCALE_COUNT];
};

bool color_scale_is_flag_set(in uint flags, in uint flag) {
	return (flags & flag) != uint(0);
}

float color_scale_map_range_safe(in float value, in float in_left, in float in_right, in float out_left, in float out_right) {
	float size = in_right - in_left;
	if(abs(size) < CGV_COLOR_SCALE_ZERO_THRESHOLD)
		return out_left;
	return out_left + (out_right - out_left) * ((value - in_left) / size);
}

float color_scale_map_value(in float value, in ColorScaleArguments arguments) {
	vec2 domain = arguments.domain;
	if(color_scale_is_flag_set(arguments.flags, CGV_COLOR_SCALE_CLAMPED_FLAG))
		value = clamp(value, domain.x, domain.y);

	float t = 0.0;

	bool is_diverging = color_scale_is_flag_set(arguments.flags, CGV_COLOR_SCALE_DIVERGING_FLAG);
	uint transform = arguments.flags & CGV_COLOR_SCALE_TRANSFORM_MASK;

	if(color_scale_is_flag_set(arguments.flags, CGV_COLOR_SCALE_SAMPLE_MODE_DISCRETE_FLAG)) {
		is_diverging = false;
		transform = CGV_COLOR_SCALE_TRANSFORM_LINEAR;
	}

	switch(transform) {
	case CGV_COLOR_SCALE_TRANSFORM_LINEAR:
	{
		if(is_diverging) {
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
		if(is_diverging) {
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
		t = log(arguments.log_sign * value) / arguments.log_base;
		if(is_diverging) {
			if(value < arguments.midpoint) {
				t = color_scale_map_range_safe(t, arguments.log_lower_bound, arguments.log_midpoint, 0.0, 0.5);
			} else {
				t = color_scale_map_range_safe(t, arguments.log_midpoint, arguments.log_upper_bound, 0.5, 1.0);
			}
		} else {
			t = color_scale_map_range_safe(t, arguments.log_lower_bound, arguments.log_upper_bound, 0.0, 1.0);
		}
		if(isnan(t))
			t = 0.0;
		t *= arguments.log_sign;

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

vec4 color_scale_sample_texture_discrete(in int index, in int size, in float t) {
	int x = clamp(int(t * float(size)), 0, size - 1);
	return texelFetch(color_scale_texture, ivec2(x, index), 0);
}

vec4 evaluate_color_scale(in int index, in float value) {
	ColorScaleArguments arguments = color_scale_arguments[index];

	bool is_clamped = color_scale_is_flag_set(arguments.flags, CGV_COLOR_SCALE_CLAMPED_FLAG);
	if(!is_clamped && (value < arguments.domain.x || value > arguments.domain.y))
		return unpackUnorm4x8(arguments.unknown_color);

	vec4 color = vec4(0.0);
	float t = color_scale_map_value(value, arguments);
	if(color_scale_is_flag_set(arguments.flags, CGV_COLOR_SCALE_SAMPLE_MODE_DISCRETE_FLAG))
		color = color_scale_sample_texture_discrete(index, arguments.indexed_color_count, t);
	else
		color = color_scale_sample_texture_continuous(index, t);

	// Todo: Linearize colors?
	//color.rgb = pow(color.rgb, vec3(2.2));
	return color;
}
