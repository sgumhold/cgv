#version 150 

/*
The following interface is implemented in this shader:
//***** begin interface of lights.glsl ***********************************
struct LightSource
{
	int light_source_type;
	vec3 position;
	vec3 emission;
	float ambient_scale;
	vec3 spot_direction;
	float spot_exponent;
	float spot_cos_cutoff;
	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
};
int get_nr_light_sources();
LightSource get_light_source(int i);
void evaluate_light(LightSource L, in vec3 p_eye, out vec3 omega_in, out vec3 radiance_in);
void evaluate_light_scale(LightSource L, in vec3 p_eye, out vec3 omega_in, out vec3 radiance_in, out float scale);
//***** end interface of lights.glsl ***********************************
*/

struct LightSource
{
	int light_source_type;
	vec3 position;
	vec3 emission;
	float ambient_scale;
	vec3 spot_direction;
	float spot_exponent;
	float spot_cos_cutoff;
	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
};

uniform LightSource light_sources[8];
uniform int nr_light_sources = 1;

int get_nr_light_sources() 
{
	return nr_light_sources;
}

LightSource get_light_source(int i)
{
	return light_sources[i];
}

void evaluate_light_scale(LightSource L, in vec3 p_eye, out vec3 omega_in, out vec3 radiance_in, out float scale)
{
	radiance_in = L.emission;
	scale = 1.0;
	if (L.light_source_type == 0) {
		omega_in = normalize(L.position);
		return;
	}
	omega_in = L.position - p_eye;
	float dist = length(omega_in);
	omega_in /= dist;
	float attenuation = 1.0 / (L.constant_attenuation + dist * (L.linear_attenuation + dist * L.quadratic_attenuation ) );
	radiance_in *= attenuation;
	scale *= attenuation;
	if (L.light_source_type == 1)
		return;

	float cos_spot = -dot(omega_in, L.spot_direction);
	if (cos_spot < L.spot_cos_cutoff) {
		radiance_in = vec3(0.0);
		scale = 0.0;
		return;
	}
	radiance_in *= pow(cos_spot, L.spot_exponent);
	scale *= pow(cos_spot, L.spot_exponent);
}

void evaluate_light(LightSource L, in vec3 p_eye, out vec3 omega_in, out vec3 radiance_in)
{
	float scale;
	evaluate_light_scale(L, p_eye, omega_in, radiance_in, scale);
}
