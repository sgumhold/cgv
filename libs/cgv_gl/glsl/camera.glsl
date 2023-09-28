#version 330 core

#define SUCCESS 0
#define CONVERGENCE 0
#define OUT_OF_BOUNDS 1
#define DIVISION_BY_ZERO 2
#define DIVERGENCE 3
#define MAX_ITERATIONS_REACHED 4

/*
The following interface is implemented in this shader:
//***** begin interface of camera.glsl ***********************************
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
int invert_distortion_model(in vec2 xu, out vec2 xd, bool use_xd_as_initial_guess, in calibration calib);
//***** end interface of camera.glsl ***********************************
*/

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

uniform float inversion_epsilon = 1e-6;
uniform int max_nr_iterations = 20;
uniform float slow_down = 1.0;

vec2 image_to_pixel_coordinates(in vec2 x, in calibration calib) 
{
	return vec2(calib.s[0] * x[0] + calib.skew * x[1] + calib.c[0], calib.s[1] * x[1] + calib.c[1]);
}

vec2 pixel_to_image_coordinates(in vec2 p, in calibration calib)
{
	float y = (p[1] - calib.c[1]) / calib.s[1]; 
	return vec2((p[0] - calib.c[0] - calib.skew*y)/calib.s[0], y);
}

vec2 texture_to_pixel_coordinates(in vec2 t, in calibration calib)
{
	return vec2(calib.w * t.x, calib.h * t.y);
}
vec2 pixel_to_texture_coordinates(in vec2 p, in calibration calib)
{
	return vec2(p.x / calib.w, p.y / calib.h);
}

int apply_distortion_model(in vec2 xd, out vec2 xu, out mat2 J, float epsilon, in calibration calib)
{
	vec2 od = xd - calib.dc;
	float xd2 = od[0]*od[0];
	float yd2 = od[1]*od[1];
	float xyd = od[0]*od[1];
	float rd2 = xd2 + yd2;
	// ensure to be within valid projection radius
	if (rd2 > calib.max_radius_for_projection * calib.max_radius_for_projection)
		return OUT_OF_BOUNDS;
	float v = 1.0 + rd2 * (calib.k[3] + rd2 * (calib.k[4] + rd2 * calib.k[5]));
	float u = 1.0 + rd2 * (calib.k[0] + rd2 * (calib.k[1] + rd2 * calib.k[2]));
	// check for division by very small number or zero
	if (abs(v) < epsilon*abs(u))
		return DIVISION_BY_ZERO;
	float inv_v = 1.0 / v;
	float f = u * inv_v;
	xu[0] = f*od[0] + 2.0*xyd*calib.p[0] + (3.0*xd2 + yd2)*calib.p[1];
	xu[1] = f*od[1] + 2.0*xyd*calib.p[1] + (xd2 + 3.0*yd2)*calib.p[0];

	float du = calib.k[0] + rd2*(2.0*calib.k[1] + 3.0*rd2*calib.k[2]);
	float dv = calib.k[3] + rd2*(2.0*calib.k[4] + 3.0*rd2*calib.k[5]);
	float df = (du*v - dv*u)*inv_v*inv_v;
	J[0][0] =       f + 2.0*(df*xd2 +     od[1]*calib.p[0] + 3.0*od[0]*calib.p[1]);
	J[1][1] =       f + 2.0*(df*yd2 + 3.0*od[1]*calib.p[0] +     od[0]*calib.p[1]);
	J[1][0] = J[0][1] = 2.0*(df*xyd +     od[0]*calib.p[0] +     od[1]*calib.p[1]);
	return SUCCESS;
}

int apply_distortion_model(in vec2 xd, out vec2 xu, out mat2 J, in calibration calib)
{
	return apply_distortion_model(xd, xu, J, inversion_epsilon, calib);
}

/// invert model for image coordinate inversion
int invert_distortion_model(in vec2 xu, inout vec2 xd, bool use_xd_as_initial_guess, in calibration calib) 
{
	// start with approximate inversion
	if (!use_xd_as_initial_guess) {
		vec2 od = xu - calib.dc;
		float xd2 = od[0] * od[0];
		float yd2 = od[1] * od[1];
		float xyd = od[0] * od[1];
		float rd2 = xd2 + yd2;
		float inverse_radial = 1.0 + rd2 * (calib.k[3] + rd2 * (calib.k[4] + rd2 * calib.k[5]));
		float enumerator     = 1.0 + rd2 * (calib.k[0] + rd2 * (calib.k[1] + rd2 * calib.k[2]));
		if (abs(enumerator) >= inversion_epsilon)
			inverse_radial /= enumerator;
		od *= inverse_radial;
		od -= vec2((yd2 + 3.0 * xd2) * calib.p[1] + 2.0 * xyd * calib.p[0], (xd2 + 3.0 * yd2) * calib.p[0] + 2.0 * xyd * calib.p[1]);
		xd = od + calib.dc;
	}
	// iteratively improve approximation
	vec2 xd_best = xd;
	float err_best = 10000.0;
	for (int i = 0; i < max_nr_iterations; ++i) {
		mat2 J;
		vec2 xu_i;
		int dr = apply_distortion_model(xd, xu_i, J, inversion_epsilon, calib);
		if (dr == DIVISION_BY_ZERO) {
			xd = xd_best;
			return DIVISION_BY_ZERO;
		}
		if (dr == OUT_OF_BOUNDS) {
			xd = xd_best;
			return OUT_OF_BOUNDS;
		}
		// check for convergence
		vec2 dxu = xu - xu_i;
		float err = dot(dxu,dxu);
		if (err < inversion_epsilon * inversion_epsilon)
			return CONVERGENCE;
		// check for divergence
		if (err > err_best) {
			xd = xd_best;
			return DIVERGENCE;
		}
		// improve approximation before the last iteration
		xd_best = xd;
		err_best = err;
		vec2 dxd = inverse(J) * dxu;
		xd += slow_down * dxd;
	}
	return MAX_ITERATIONS_REACHED;
}
