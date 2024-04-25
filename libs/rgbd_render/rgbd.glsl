#version 330 core

#define USE_DISTORTION_MAP 0

/*
The following interface is implemented in this shader:
//***** begin interface of rgbd.glsl ***********************************
uint get_depth_width();
uint get_depth_height();
uint lookup_depth(ivec2 xp);
bool construct_point(in vec2 xp, in float depth, out vec3 p);
vec4 lookup_color(vec2 tc);
bool lookup_color(vec3 p, out vec4 c);
bool lookup_color_texcoord(vec3 p, out vec2 tc);
bool lookup_color(vec3 p, float eps, out vec4 c);
//***** end interface of rgbd.glsl ***********************************
*/

//***** begin interface of camera.glsl ***********************************
#define CONVERGENCE 0
#define SUCCESS 0
uniform float inversion_epsilon;
struct calibration
{
	int w;
	int h;
	float max_radius_for_projection;
	vec2 dc;
	float k[6];
	float p[2];
	float skew;
	vec2 c, s;
};
vec2 image_to_pixel_coordinates(in vec2 x, in calibration calib);
vec2 pixel_to_image_coordinates(in vec2 p, in calibration calib);
vec2 texture_to_pixel_coordinates(in vec2 t, in calibration calib);
vec2 pixel_to_texture_coordinates(in vec2 p, in calibration calib);
int apply_distortion_model(in vec2 xd, out vec2 xu, out mat2 J, in calibration calib);
int apply_distortion_model(in vec2 xd, out vec2 xu, out mat2 J, float epsilon, in calibration calib);
int invert_distortion_model(in vec2 xu, inout vec2 xd, bool use_xd_as_initial_guess, in calibration calib);
//***** end interface of camera.glsl ***********************************

uniform float depth_scale = 0.001;
uniform calibration depth_calib;
uniform calibration color_calib;
uniform mat3 color_rotation;
uniform vec3 color_translation;
uniform sampler2D depth_image;
uniform sampler2D color_image;

#if USE_DISTORTION_MAP != 0
uniform sampler2D distortion_map;
#endif

uint get_depth_width()
{
	return uint(depth_calib.w);
}
uint get_depth_height()
{
	return uint(depth_calib.h);
}

uint lookup_depth(ivec2 xp)
{
	return uint(65535.0*texture(depth_image, vec2((xp.x+0.5)/depth_calib.w, (xp.y+0.5)/depth_calib.h)).x);
}

bool construct_point(in vec2 xp, in float depth, out vec3 p)
{
	if (depth == 0)
		return false;
	vec2 xu = pixel_to_image_coordinates(xp, depth_calib);
	vec2 xd = xu;
#if USE_DISTORTION_MAP != 0
	xd = texture(distortion_map, vec2((xp.x+0.5)/depth_calib.w, (xp.y+0.5)/depth_calib.h)).xy;
	if (xd.x < -1000.0)
		return false;
#else
	int dir;
	dir = invert_distortion_model(xu, xd, true, depth_calib);
	if (dir != CONVERGENCE)
		return false;
#endif
	float depth_m = depth_scale * depth;
	p = vec3(depth_m*xd, depth_m);
	return true;
}

bool lookup_color_texcoord(vec3 p, out vec2 tc)
{
	p = ((p + depth_scale * color_translation) * color_rotation);
	vec2 xu;
	vec2 xd = vec2(p[0] / p[2], p[1] / p[2]);
	mat2 J;
	int result = apply_distortion_model(xd, xu, J, color_calib);
	if (result != SUCCESS)
		return false;
	vec2 xp = image_to_pixel_coordinates(xu, color_calib);
	if (xp[0] < 0.0 || xp[1] < 0.0 || xp[0] >= float(color_calib.w) || xp[1] >= float(color_calib.h))
		return false;
	tc = pixel_to_texture_coordinates(xp, color_calib);
	return true;
}

bool lookup_color(vec3 p, float eps, out vec4 c)
{
	p = ((p + depth_scale*color_translation)*color_rotation);
	vec2 xu;
	vec2 xd = vec2(p[0] / p[2], p[1] / p[2]);
	mat2 J;
	int result = apply_distortion_model(xd, xu, J, eps, color_calib);
	if (result != SUCCESS)
		return false;
	vec2 xp = image_to_pixel_coordinates(xu, color_calib);
	if (xp[0] < 0.0 || xp[1] < 0.0 || xp[0] >= float(color_calib.w) || xp[1] >= float(color_calib.h))
		return false;
	c = texture(color_image, pixel_to_texture_coordinates(xp, color_calib));
	return true;
}

bool lookup_color(vec3 p, out vec4 c)
{
	p = ((p + depth_scale*color_translation)*color_rotation);
	vec2 xu;
	vec2 xd = vec2(p[0] / p[2], p[1] / p[2]);
	mat2 J;
	int result = apply_distortion_model(xd, xu, J, color_calib);
	if (result != SUCCESS)
		return false;
	vec2 xp = image_to_pixel_coordinates(xu, color_calib);
	if (xp[0] < 0.0 || xp[1] < 0.0 || xp[0] >= float(color_calib.w) || xp[1] >= float(color_calib.h))
		return false;
	c = texture(color_image, pixel_to_texture_coordinates(xp, color_calib));
	return true;
}
vec4 lookup_color(vec2 tc)
{
	return texture(color_image, tc);
}
