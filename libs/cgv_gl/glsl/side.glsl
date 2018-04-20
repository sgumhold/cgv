#version 150 compatibility

uniform int culling_mode;
uniform int map_color_to_material;
uniform int illumination_mode;

int surface_side_handling(in vec3 position, inout vec3 normal, inout vec4 color)
{
	// sign of dot product between normal and view vector tells us side that we see
	float sign_indicator = dot(normal, position);
	int side = (sign_indicator < 0.0) ? 1 : 0;
	// perform face culling
	switch (culling_mode) {
	case 1: // backface culling
		if (sign_indicator > 0.0) {
			return -1;
		}
		break;
	case 2: // frontface culling
		if (sign_indicator < 0.0) {
			return -1;
		}
		break;
	}
	// based on illumination mode and side, overwrite color with material color if needed, and overwrite normal with negated normal if needed
	if (illumination_mode > 0) {

		// negate normal for back faces in double sided illumination
		if (illumination_mode == 2 && side == 0) {
			normal = -normal;
		}
		// overwrite color with diffuse material color if no color mapping turned on
		if (side == 1) {
			if ((map_color_to_material & 1) == 0) {
				color = gl_FrontMaterial.diffuse;
			}
		}
		else {
			if ((map_color_to_material & 2) == 0) {
				color = gl_BackMaterial.diffuse;
			}
		}
	}
	return side;
}

