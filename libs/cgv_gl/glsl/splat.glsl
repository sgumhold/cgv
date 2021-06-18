#version 150

/*
The following interface is implemented in this shader:
//***** begin interface of splat.glsl ***********************************
/// compute fragment color of point splat with halo
vec4 compute_blended_color_with_halo(
	in float percentual_radial_position,
	in vec4 core_color, in float percentual_core_size,
	in vec4 halo_color, in float percentual_point_size,
	in float percentual_blend_width);
/// prepare spat size parameters
void prepare_splat(
	in float reference_point_radius, in float pixel_extent,
	in float blend_width_in_pixel, in float halo_width_in_pixel, in float percentual_halo_width,
	out float percentual_core_size, out float percentual_point_size,
	out float percentual_blend_width, out float percentual_splat_size);
//***** end interface of splat.glsl ***********************************
*/

float add_color(
	inout vec4 color1, in float min_value1, in float max_value1, 
	in vec4 color2, in float min_value2, in float max_value2, in float percentual_blend_width)
{
	float max_value = min(max_value1, max_value2);
	float min_value = max(min_value1, min_value2);

	float lambda = min(max_value1, max_value2) - max(min_value1, min_value2);
	if (max_value >= min_value) {
		lambda = (max_value - min_value) / percentual_blend_width;
		color1 += lambda * color2;
		return lambda;
	}
	return 0.0;
}

float add_color_if(inout vec4 color1, in float value1, in vec4 color2, float min_value2, float max_value2)
{
	if (value1 >= min_value2 && value1 < max_value2) {
		color1 += color2;
		return 1.0;
	}
	return 0.0;
}

vec4 compute_blended_color_with_halo(
	in float percentual_radial_position, 
	in vec4 core_color, in float percentual_core_size,
	in vec4 halo_color, in float percentual_point_size,	
	in float percentual_blend_width)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
	float min_value1 = percentual_radial_position - 0.5*percentual_blend_width;
	float max_value1 = percentual_radial_position + 0.5*percentual_blend_width;
	float fullness = 0.0;
	if (percentual_blend_width > 0.0) {
		fullness += add_color(color, min_value1, max_value1, halo_color, -percentual_point_size, -percentual_core_size, percentual_blend_width);
		fullness += add_color(color, min_value1, max_value1, core_color, -percentual_core_size, percentual_core_size, percentual_blend_width);
		fullness += add_color(color, min_value1, max_value1, halo_color, percentual_core_size, percentual_point_size, percentual_blend_width);
	}
	else {
		fullness += add_color_if(color, percentual_radial_position, core_color, -percentual_core_size, percentual_core_size);
		fullness += add_color_if(color, percentual_radial_position, halo_color, percentual_core_size, percentual_point_size);
	}
	if (fullness > 0.0)
		color.rgb /= fullness;
	return color;
}

void prepare_splat(
	in float reference_point_radius, in float pixel_extent,
	in float blend_width_in_pixel, in float halo_width_in_pixel, in float percentual_halo_width,
	out float percentual_core_size, out float percentual_point_size,
	out float percentual_blend_width, out float percentual_splat_size)
{
	// compute percentage of one pixel with respect to point radius
	float percentual_pixel_extent = pixel_extent / reference_point_radius;
	// compute percentual width of blending area
	percentual_blend_width = blend_width_in_pixel * percentual_pixel_extent;
	// compute percentual halo size
	float percentual_halo_size;
	if (percentual_halo_width*halo_width_in_pixel >= 0.0)
		percentual_halo_size = max(abs(percentual_halo_width), abs(halo_width_in_pixel*percentual_pixel_extent));
	else
		percentual_halo_size = abs(percentual_halo_width - halo_width_in_pixel * percentual_pixel_extent);
	// compute point radius possibly extended by positive halo width
	percentual_point_size = 1.0 + max(0.0, max(percentual_halo_width, halo_width_in_pixel*percentual_pixel_extent));
	// compute core size
	percentual_core_size = percentual_point_size - percentual_halo_size;
	// compute scaling factor to get splat radius
	percentual_splat_size = percentual_point_size + percentual_blend_width;
}
