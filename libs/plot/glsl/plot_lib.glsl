#version 330 core

/*
The following interface is implemented in this shader:
//***** begin interface of plot_lib.glsl ***********************************
uniform float attribute_min[8];
uniform float attribute_max[8];
bool discard_vertex(inout float attributes[8]);
float tick_space_from_attribute_space(int ai, float value);
float attribute_space_from_tick_space(int ai, float value);
float window_space_from_tick_space(int ai, float value);
float tick_space_from_window_space(int ai, float value);
vec3 plot_space_from_window_space(vec3 pnt);
vec3 window_space_from_plot_space(vec3 pnt);
vec3 world_space_from_plot_space(vec3 pnt);
vec3 map_color(in float v_window, int idx);
vec3 map_color(in float attributes[8], in vec3 base_color, int idx);
float map_opacity(in float v_window, int idx);
float map_opacity(in float attributes[8], in float base_opacity, int idx);
float map_size(in float v_window, int idx);
float map_size(in float attributes[8], in float base_size, int idx);
vec3 map_color(in float v_window);
vec3 map_color(in float attributes[8], in vec3 base_color);
float map_opacity(in float v_window);
float map_opacity(in float attributes[8], in float base_opacity);
float map_size(in float v_window);
float map_size(in float attributes[8], in float base_size);
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

//***** begin interface of color_scale.glsl ***********************************
/// gamma adjust value after clamping to [0,1] and in case of uniform color_scale_is_bi_polar[0] accounting for uniform window_zero_position[0]
float color_scale_gamma_mapping(in float v, in float gamma);
/// gamma adjust value after clamping to [0,1] and in case of uniform color_scale_is_bi_polar[idx] accounting for uniform window_zero_position[idx]
float color_scale_gamma_mapping(in float v, in float gamma, int idx);
/// map value with color scale selected in uniform color_scale_index[idx=0|1] to rgb color
vec3 color_scale(in float v, int idx);
/// map value with color scale selected in uniform color_scale_index[0] to rgb color
vec3 color_scale(in float v, int idx);
//***** end interface of color_scale.glsl ***********************************

// for first three attributes the mode of handling data that is out of range
// 0 .. keep
// 1 .. discard
// 2 .. clamp
uniform ivec4 out_of_range_mode = ivec4(0, 0, 0, 0);

// tick space transform
uniform int axis_log_scale[8];
uniform float axis_log_minimum[8];

// window transform
uniform float attribute_min[8];
uniform float attribute_max[8];

// world transform
uniform vec3 extent;
uniform vec3 center_location;
uniform vec4 orientation = vec4(0.0, 0.0, 0.0, 1.0);
uniform float feature_offset;

// color mapping
const int MAX_NR_COLOR_MAPPINGS = 2;
uniform int   color_mapping[MAX_NR_COLOR_MAPPINGS] = int[MAX_NR_COLOR_MAPPINGS]( -1, -1 );
uniform float color_scale_gamma[MAX_NR_COLOR_MAPPINGS] = float[MAX_NR_COLOR_MAPPINGS](1.0, 1.0 );

// opacity mapping
const int MAX_NR_OPACITY_MAPPINGS = 2;
uniform int   opacity_mapping[MAX_NR_OPACITY_MAPPINGS] = int[MAX_NR_OPACITY_MAPPINGS]( -1, -1 );
uniform float opacity_gamma[MAX_NR_OPACITY_MAPPINGS] = float[MAX_NR_OPACITY_MAPPINGS]( 1.0, 1.0 );
uniform int  opacity_is_bipolar[MAX_NR_OPACITY_MAPPINGS] = int[MAX_NR_OPACITY_MAPPINGS]( 0, 0 );
uniform float opacity_window_zero_position[MAX_NR_OPACITY_MAPPINGS] = float[MAX_NR_OPACITY_MAPPINGS]( 0.5, 0.5 );
uniform float opacity_min[MAX_NR_OPACITY_MAPPINGS] = float[MAX_NR_OPACITY_MAPPINGS]( 0.1, 0.1 );
uniform float opacity_max[MAX_NR_OPACITY_MAPPINGS] = float[MAX_NR_OPACITY_MAPPINGS]( 1.0, 1.0 );

// size mapping
const int MAX_NR_SIZE_MAPPINGS = 2;
uniform int   size_mapping[MAX_NR_SIZE_MAPPINGS] = int[MAX_NR_SIZE_MAPPINGS]( -1, -1 );
uniform float size_gamma[MAX_NR_SIZE_MAPPINGS] = float[MAX_NR_SIZE_MAPPINGS]( 1.0, 1.0 );
uniform float size_max[MAX_NR_SIZE_MAPPINGS] = float[MAX_NR_SIZE_MAPPINGS]( 1.0, 1.0 );
uniform float size_min[MAX_NR_SIZE_MAPPINGS] = float[MAX_NR_SIZE_MAPPINGS]( 0.2, 0.2 );

bool discard_vertex(inout float attributes[8])
{
	for (int ai = 0; ai < 4; ++ai) {
		// create a local variable first to prevent AMD driver compile error
		int mode = out_of_range_mode[ai];
		switch (mode) {
		case 0: continue;
		case 1:
			if (attributes[ai] < attribute_min[ai] || attributes[ai] > attribute_max[ai])
				return true;
			break;
		case 2:
			attributes[ai] = clamp(attributes[ai], attribute_min[ai], attribute_max[ai]);
			break;
		}
	}
	return false;
}

float tick_space_from_attribute_space(int ai, float value)
{
	if (axis_log_scale[ai] == 0)
		return value;
	if (value > axis_log_minimum[ai])
		return log(value)/log(10.0);
	if (value < -axis_log_minimum[ai])
		return (2.0 * log(axis_log_minimum[ai]) - log(-value))/ log(10.0) - 2.0;
	return log(axis_log_minimum[ai])/log(10.0) + value / axis_log_minimum[ai] - 1.0;
}

float attribute_space_from_tick_space(int ai, float value)
{
	if (axis_log_scale[ai] == 0)
		return value;
	if (value > log(axis_log_minimum[ai])/log(10.0))
		return pow(10.0f, value);
	if (value < log(axis_log_minimum[ai])/log(10.0) - 2.0)
		return -pow(10.0f, 2.0 * log(axis_log_minimum[ai])/log(10.0) - 2.0 - value);
	return axis_log_minimum[ai] * (value + 1.0 - log(axis_log_minimum[ai])/log(10.0));
}

float window_space_from_tick_space(int ai, float value)
{
	float min_value = attribute_min[ai];
	float max_value = attribute_max[ai];
	if (axis_log_scale[ai] != 0) {
		min_value = tick_space_from_attribute_space(ai, min_value);
		max_value = tick_space_from_attribute_space(ai, max_value);
	}
	return (value - min_value) / (max_value - min_value);
}
float tick_space_from_window_space(int ai, float value)
{
	float min_value = attribute_min[ai];
	float max_value = attribute_max[ai];
	if (axis_log_scale[ai] != 0) {
		min_value = tick_space_from_attribute_space(ai, min_value);
		max_value = tick_space_from_attribute_space(ai, max_value);
	}
	return value * (max_value - min_value) + min_value;
}
vec3 plot_space_from_window_space(vec3 pnt)
{
	return extent * (pnt - 0.5f);
}
vec3 window_space_from_plot_space(vec3 pnt)
{
	return pnt / extent + 0.5f;
}
vec3 world_space_from_plot_space(vec3 pnt)
{
	return center_location + rotate_vector_with_quaternion(pnt + vec3(0.0, 0.0, feature_offset), orientation);
}

vec3 map_color(in float v, int idx)
{
	return color_scale(color_scale_gamma_mapping(v, color_scale_gamma[idx], idx), idx);
}

vec3 map_color(in float v)
{
	return map_color(v, 0);
}

vec3 map_color(in float attributes[8], in vec3 base_color, int idx)
{
	if (idx >= MAX_NR_COLOR_MAPPINGS || color_mapping[idx] < 0 || color_mapping[idx] > 7)
		return base_color;
	// simple window transform
	float v = window_space_from_tick_space(color_mapping[idx],
				tick_space_from_attribute_space(color_mapping[idx], attributes[color_mapping[idx]]));
	return map_color(v,idx);
}

vec3 map_color(in float attributes[8], in vec3 base_color)
{
	return map_color(attributes, base_color, 0);
}

float opacity_gamma_mapping(in float v, in float gamma, int idx)
{
	if (opacity_is_bipolar[idx] != 0) {
		float amplitude = max(opacity_window_zero_position[idx], 1.0 - opacity_window_zero_position[idx]);
		if (v < opacity_window_zero_position[idx])
			return opacity_window_zero_position[idx] - pow((opacity_window_zero_position[idx] - v) / amplitude, gamma) * amplitude;
		else
			return pow((v - opacity_window_zero_position[idx]) / amplitude, gamma) * amplitude + opacity_window_zero_position[idx];
	}
	else
		return pow(v, gamma);
}

float opacity_gamma_mapping(in float v, in float gamma)
{
	return opacity_gamma_mapping(v, gamma, 0);
}

float map_opacity(in float v, int idx)
{
	return (opacity_max[idx] - opacity_min[idx]) * opacity_gamma_mapping(v, opacity_gamma[idx], idx) + opacity_min[idx];
}

float map_opacity(in float v)
{
	return map_opacity(v, 0);
}

float map_opacity(in float attributes[8], in float base_opacity, int idx)
{
	if (idx >= MAX_NR_OPACITY_MAPPINGS || opacity_mapping[idx] < 0 || opacity_mapping[idx] > 7)
		return base_opacity;
	// simple window transform
	float v = window_space_from_tick_space(opacity_mapping[idx],
		tick_space_from_attribute_space(opacity_mapping[idx], attributes[opacity_mapping[idx]]));
	return map_opacity(v, idx) * base_opacity;
}

float map_opacity(in float attributes[8], in float base_opacity)
{
	return map_opacity(attributes, base_opacity, 0);
}

float map_size(in float v, int idx)
{
	return ((size_max[idx] - size_min[idx]) * pow(v, size_gamma[idx]) + size_min[idx]);
}

float map_size(in float v)
{
	return map_size(v, 0);
}

float map_size(in float attributes[8], in float base_size, int idx)
{
	if (idx >= MAX_NR_SIZE_MAPPINGS || size_mapping[idx] < 0 || size_mapping[idx] > 7)
		return base_size;
	// simple window transform
	float v = window_space_from_tick_space(size_mapping[idx],
		tick_space_from_attribute_space(size_mapping[idx], attributes[size_mapping[idx]]));
	return map_size(v, idx) * base_size;
}

float map_size(in float attributes[8], in float base_size)
{
	return map_size(attributes, base_size, 0);
}
