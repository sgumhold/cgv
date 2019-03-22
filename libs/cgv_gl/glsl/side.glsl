#version 150 

uniform int culling_mode = 0;
uniform int map_color_to_material = 0;
uniform int illumination_mode = 2;

/*
The following interface is implemented in this shader:
//***** begin interface of side.glsl ***********************************
bool keep_this_side(in vec3 position, in vec3 normal, out int side);
void update_material_color_and_transparency(inout vec3 mat_color, inout float transparency, in int side, in vec4 color);
void update_normal(inout vec3 normal, in int side);
//***** end interface of side.glsl ***********************************
*/

bool keep_this_side(in vec3 position, in vec3 normal, out int side)
{
	side = (dot(normal, position) < 0.0) ? 1 : 0;
	return (culling_mode == 0) || (side != culling_mode-1);
}

void update_material_color_and_transparency(inout vec3 mat_color, inout float transparency, in int side, in vec4 color)
{
	// overwrite material color with color if color mapping is turned on
	if (side == 1) {
		if ((map_color_to_material & 1) == 1) {
			mat_color = color.rgb;
			transparency = 1.0-color.a;
		}
	}
	else {
		if ((map_color_to_material & 2) == 2) {
			mat_color = color.rgb;
			transparency = 1.0-color.a;
		}
	}
}

void update_normal(inout vec3 normal, in int side)
{
	// based on illumination mode and side, overwrite normal with negated normal if needed
	if (illumination_mode > 0) {
		// negate normal for back faces in double sided illumination
		if (illumination_mode == 2 && side == 0) {
			normal = -normal;
		}
	}
}
