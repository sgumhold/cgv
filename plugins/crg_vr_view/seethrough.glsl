#version 150

uniform int eye;
uniform int frame_split;
uniform vec2 extent_texcrd;
uniform vec2 center_left;
uniform vec2 center_right;

vec2 compute_seethrough_texture_coordinates(in vec4 tex_position)
{
	vec2 texcrd = tex_position.xy / tex_position.w;
	texcrd.y = -texcrd.y;

	vec2 center = center_left;
	if (frame_split == 1) { // vertical split
		if (eye == 0)
			center.y += 0.5;
		else
			center = center_right;
	}
	else { // horizontal split
		if (eye == 1) {
			center = center_right;
			center.x += 0.5;
		}			
	}
	texcrd= texcrd*extent_texcrd+center;
	return texcrd;
}