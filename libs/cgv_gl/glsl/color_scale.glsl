#version 150

uniform int    color_scale_index = 7;
uniform float  color_scale_gamma = 1.0;
uniform vec3   color_scale_samples[32];
uniform int nr_color_scale_samples;

/*
The following interface is implemented in this shader:
//***** begin interface of color_scale.glsl ***********************************
vec3 color_scale(in float v);
//***** end interface of color_scale.glsl ***********************************
*/

vec3 hue_scale(in float v)
{
	float HH = 6.0 * v;
	float F = mod(HH, 1.0);
	float G = 1.0-F;
	switch (int(HH)) {
	case 0: return vec3(1.0,   F, 0.0);
	case 1: return vec3(  G, 1.0, 0.0);
	case 2: return vec3(0.0, 1.0,   F);
	case 3: return vec3(0.0,   G, 1.0);
	case 4: return vec3(  F, 0.0, 1.0);
	case 5: return vec3(1.0, 0.0,   G);
	}
	return vec3(0.5, 0.5, 0.5);
}

vec3 hue_luminance_scale(in float v)
{
	float HH = 6.0 * v;
	float F = mod(HH, 1.0);
	int I = int(HH);
	float LL = 0.5 * v + 0.25;
	float mx = (LL <= 0.5) ? 2.0*LL : 1.0;
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
	// scale value up to [0,n-1]
	float v = value * (nr_color_scale_samples - 1);
	// compute index of smaller sampled necessary for linear interpolation
	int i = int(v);
	// compute fractional part
	float f = v - float(i);
	// return affine combination of two adjacent samples
	return (1.0 - f) * color_scale_samples[i] + f * color_scale_samples[i + 1];
}

vec3 color_scale(in float v)
{
	v = pow(clamp(v, 0.0, 1.0), color_scale_gamma);
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
