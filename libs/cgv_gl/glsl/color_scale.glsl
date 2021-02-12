#version 150

uniform int    color_scale_index = 7;
uniform vec3   color_scale_samples[32];
uniform int nr_color_scale_samples;
uniform bool color_scale_is_bipolar = false;
uniform float window_zero_position = 0.5;

/*
The following interface is implemented in this shader:
//***** begin interface of color_scale.glsl ***********************************
/// adjust value with a gamma mapping, which clamps value to [0,1] and accounts for window zero positions in case of bipolar color scales
float color_scale_gamma_mapping(in float v, in float gamma);
/// map value with currently selected color scale to rgb color
vec3 color_scale(in float v);
//***** end interface of color_scale.glsl ***********************************
*/

vec3 hue_scale(in float v)
{
	float HH = 6.0 * v;
	float F = mod(HH, 1.0);
	float G = 1.0 - F;
	switch (int(HH)) {
	case 0: return vec3(1.0, F, 0.0);
	case 1: return vec3(G, 1.0, 0.0);
	case 2: return vec3(0.0, 1.0, F);
	case 3: return vec3(0.0, G, 1.0);
	case 4: return vec3(F, 0.0, 1.0);
	case 5: return vec3(1.0, 0.0, G);
	}
	return vec3(0.5, 0.5, 0.5);
}

vec3 hue_luminance_scale(in float v)
{
	float HH = 6.0 * v;
	float F = mod(HH, 1.0);
	int I = int(HH);
	float LL = 0.5 * v + 0.25;
	float mx = (LL <= 0.5) ? 2.0 * LL : 1.0;
	float mn = 2 * LL - mx;
	float DM = mx - mn;
	switch (I) {
	case 0: return vec3(mx, mn + F * DM, mn);
	case 1: return vec3(mn + (1.0 - F) * DM, mx, mn);
	case 2: return vec3(mn, mx, mn + F * DM);
	case 3: return vec3(mn, mn + (1.0 - F) * DM, mx);
	case 4: return vec3(mn + F * DM, mn, mx);
	case 5: return vec3(mx, mn, mn + (1.0 - F) * DM);
	}
	return vec3(0.5, 0.5, 0.5);
}
vec3 sampled_color_scale(in float value)
{
	// first check if values needs to be clamped to 0
	if (value <= 0.0)
		return color_scale_samples[0];
	// than check if values needs to be clamped to 1 and make sure that values is really smaller than 1
	if (value > 0.99999)
		return color_scale_samples[nr_color_scale_samples - 1];
	float f,v;
	int i;
	if (color_scale_is_bipolar) {
		// scale value up to [0,n-2]
		v = value * (nr_color_scale_samples - 2);
		// compute index of smaller sampled necessary for linear interpolation
		i = int(v);
		// compute fractional part
		f = v - float(i);
		// correct indices in second half
		if (i+1 >= nr_color_scale_samples/2)
			++i;
	}
	else {
		// scale value up to [0,n-1]
		v = value * (nr_color_scale_samples - 1);
		// compute index of smaller sampled necessary for linear interpolation
		i = int(v);
		// compute fractional part
		f = v - float(i);
	}
	// return affine combination of two adjacent samples
	return (1.0 - f) * color_scale_samples[i] + f * color_scale_samples[i + 1];
}

float color_scale_gamma_mapping(in float v, in float gamma)
{
	if (color_scale_is_bipolar) {
		float amplitude = max(window_zero_position, 1.0 - window_zero_position);
		if (v < window_zero_position)
			return window_zero_position - pow((window_zero_position - v) / amplitude, gamma) * amplitude;
		else
			return pow((v - window_zero_position) / amplitude, gamma) * amplitude + window_zero_position;
	}
	else
		return pow(v, gamma);
}

void adjust_zero_position(inout float v)
{
	// map v according to scale*v + offset to a new value such that attribute_zero_position maps to 0.5 
	// and in case attribute_zero_position <= 0.5 v=1.0 maps to 1.0 and otherwise v=0.0 maps to 0.0
	if (window_zero_position <= 0.5)
		v = 1.0 - 0.5 * (1.0 - v) / (1.0 - window_zero_position);
	else
		v *= 0.5 / window_zero_position;
}

vec3 color_scale(in float v)
{
	if (color_scale_is_bipolar)
		adjust_zero_position(v);
	switch (color_scale_index) {
	case 0: return vec3(v, 0, 0);
	case 1: return vec3(0, v, 0);
	case 2: return vec3(0, 0, v);
	case 3: return vec3(v, v, v);
	case 4:
		if (v < 0.333333333)
			return vec3(3.0 * v, 0.0, 0.0);
		if (v < 0.666666666)
			return vec3(1.0, 3.0 * v - 1.0, 0.0);
		return vec3(1.0, 1.0, 3.0 * v - 2.0);
	case 5: return hue_scale(v);
	case 6: return hue_luminance_scale(v);
	case 7: return sampled_color_scale(v);
	}
	return vec3(v, v, v);
}
