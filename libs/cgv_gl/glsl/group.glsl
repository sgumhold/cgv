#version 150

uniform bool use_group_color;
uniform bool use_group_transformation;
uniform vec4 group_colors[250];
uniform vec3 group_translations[250];
uniform vec4 group_rotations[250];

vec4 group_color(in vec4 color, int group_index)
{
	if (use_group_color) {
		return group_colors[group_index];
	}
	else {
		return color;
	}
}

vec3 group_transformed_position(in vec3 position, int group_index)
{
	if (use_group_transformation) {
		vec4 q = group_rotations[group_index];
		vec3 p = position.xyz;
		vec3 image = cross(q.xyz, p);
		return dot(p, q.xyz)*q.xyz + q.w*(q.w*p + 2.0 * image) + cross(q.xyz, image) + group_translations[group_index];
	}
	else {
		return position;
	}
}

vec3 group_transformed_normal(in vec3 nml, int group_index)
{
	// apply group rotation to normal
	if (use_group_transformation) {
		vec4 q = group_rotations[group_index];
		vec3 nml_image = cross(q.xyz, nml);
		return dot(nml, q.xyz)*q.xyz + q.w*(q.w*nml + 2.0 * nml_image) + cross(q.xyz, nml_image);
	}
	else {
		return nml;
	}
}

void group_transform_normal_matrix(inout mat3 NM, int group_index)
{
	// apply group rotation to normal
	if (use_group_transformation) {
		vec4 q = group_rotations[group_index];
		vec3 x = vec3(1.0, 0.0, 0.0);
		vec3 tmp = cross(q.xyz, x);
		x = NM * (dot(x, q.xyz)*q.xyz + q.w*(q.w*x + 2.0 * tmp) + cross(q.xyz, tmp));
		vec3 y = vec3(0.0, 1.0, 0.0);
		tmp = cross(q.xyz, y);
		y = NM * (dot(y, q.xyz)*q.xyz + q.w*(q.w*y + 2.0 * tmp) + cross(q.xyz, tmp));
		vec3 z = vec3(0.0, 0.0, 1.0);
		tmp = cross(q.xyz, z);
		z = NM * (dot(z, q.xyz)*q.xyz + q.w*(q.w*z + 2.0 * tmp) + cross(q.xyz, tmp));
		NM[0] = x;
		NM[1] = y;
		NM[2] = z;
	}
}

void group_transform_position_matrix(inout mat4 PM, int group_index)
{
	// apply group rotation to normal
	if (use_group_transformation) {
		vec4 q = group_rotations[group_index];
		mat3 PM3 = mat3(PM);
		vec3 x = vec3(1.0, 0.0, 0.0);
		vec3 tmp = cross(q.xyz, x);
		x = PM3 * (dot(x, q.xyz)*q.xyz + q.w*(q.w*x + 2.0 * tmp) + cross(q.xyz, tmp));
		vec3 y = vec3(0.0, 1.0, 0.0);
		tmp = cross(q.xyz, y);
		y = PM3 * (dot(y, q.xyz)*q.xyz + q.w*(q.w*y + 2.0 * tmp) + cross(q.xyz, tmp));
		vec3 z = vec3(0.0, 0.0, 1.0);
		tmp = cross(q.xyz, z);
		z = PM3 * (dot(z, q.xyz)*q.xyz + q.w*(q.w*z + 2.0 * tmp) + cross(q.xyz, tmp));
		vec4 t = PM * vec4(group_translations[group_index], 1.0);
		PM[0].xyz = x;
		PM[1].xyz = y;
		PM[2].xyz = z;
		PM[3]     = t;
	}
}