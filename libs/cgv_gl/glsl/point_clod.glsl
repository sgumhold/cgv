#version 430

uniform float CLOD;
uniform float scale;
uniform float spacing; //root_spacings
uniform float pointSize;
uniform float minMilimeters;
uniform vec2 screenSize;
uniform vec4 pivot; //pivot point in view space, usually the camera position

//some special parameters affecting point size calculation
uniform float density_decrease= 0.5;
uniform float min_density = 0.3;
uniform float blendRangeFactor = 0.8;

float rand(float n) {
	return fract(cos(n) * 234561.987);
}

// Interface implemented by this shader
/*
//***** begin interface of point_clod.glsl **********************************
void compute_point_size(in vec3 position, in vec4 color, in mat4 central_transform, in mat4 transform,in mat4 model_view_matrix, in mat4 projection_matrix,out float point_size, out float radius, out float linear_depth);
//***** begin interface of point_clod.glsl **********************************
*/

// position: point position
// point color with lod encoded in alpha channel
// transform: = projection_matrix * model_view_matrix;
// central transform: in case of vr devices this can be the same transform used for point reduction 
void compute_point_size(in vec3 position, in vec4 color, in mat4 central_transform, in mat4 transform,in mat4 model_view_matrix, in mat4 projection_matrix,
		out float point_size, out float radius, out float linear_depth) {
	vec4 tpos = transform * vec4(position, 1.0);
	linear_depth = tpos.w;

	vec4 projected = tpos / tpos.w;
	vec4 centralProjected = central_transform * vec4(position, 1.0);
	centralProjected.xyz = centralProjected.xyz / centralProjected.w;
	
	vec4 viewPosition = model_view_matrix * vec4(position, 1.0);
	
	float d = distance(viewPosition.xyz, pivot.xyz);
	float dc = length(centralProjected.xy);

	float level = mod(color.a * 255, 128);
	float random = rand(position.x + position.y + position.z);
	// use random to produce smooth transition in continuous level of detail rendring
	float pointSpacing = scale * spacing / pow(2, level + random);
	
	// targetSpacingVR = targetSpacingDesktop/max(1-a·dc, minDensity)
	float targetSpacing = (d * CLOD) / (1000 * max(1 - density_decrease * dc, min_density));

	const float minPixels = 1;
	const float maxPixels = 120;
	float sizeMultiplier = pointSize;

	float minMilimeters = scale * minMilimeters / sizeMultiplier;

	{ // point size based on target spacing
		float ws = max(targetSpacing, minMilimeters / 1000.0);
		//expected distance between points (between the center of points, at this time, the points have no size)
		float l = sizeMultiplier * 2.0 * ws;
		//calculate required size to fill spaces
		vec4 v1 = viewPosition;
		vec4 v2 = vec4(v1.x + l, v1.y + l, v1.z, 1.0);

		vec4 vp1 = projection_matrix * v1;
		vec4 vp2 = projection_matrix * v2;

		//w-clip converts to screen space
		vec2 vs1 = vp1.xy / vp1.w;
		vec2 vs2 = vp2.xy / vp2.w;

		//calculate  distance 
		float ds = distance(vs1, vs2);
		float dp = ds * screenSize.y;

		point_size = (dp / 1) * 1;

		//point size in pixels
		point_size = clamp(point_size, minPixels, maxPixels);
		//radius is still in view space
		radius = ws; //radius is unused right now
	}

	{ // adjust point size within blend-in range
		float zeroAt = pointSpacing;
		float fullAt = blendRangeFactor * pointSpacing;

		float u = (targetSpacing - fullAt) / (zeroAt - fullAt);
		u = 1.0 - clamp(u, 0.0, 1.0);

		point_size = point_size * u;
	}
}