#version 150

const float PI = 3.14159265358979323846;

/*
The following interface is implemented in this shader:
//***** begin interface of brdf.glsl ***********************************
struct Material {
	int brdf_type;
	vec3 diffuse_reflectance;
	float roughness;
	float metalness;
	float ambient_occlusion;
	vec3 emission;
	float transparency;
	vec2 propagation_slow_down;
	float roughness_anisotropy;
	float roughness_orientation;
	vec3 specular_reflectance;
};

float oren_nayar_brdf(vec3 n, vec3 l, vec3 v, float roughness);
float phong_specular_brdf(vec3 normal, vec3 omega_in, vec3 omega_out, float roughness);
float blinn_specular_brdf(vec3 normal, vec3 omega_in, vec3 omega_out, float roughness);
vec3 cooktorrance_specular_brdf(vec3 n, vec3 l, vec3 v, float roughness, vec3 f0);
vec3 evaluate_material(Material material, vec3 normal, vec3 omega_in, vec3 omega_out);
//***** end interface of brdf.glsl ***********************************
*/

struct Material {
	int brdf_type;
	vec3 diffuse_reflectance;
	float roughness;
	float metalness;
	float ambient_occlusion;
	vec3 emission;
	float transparency;
	vec2 propagation_slow_down;
	float roughness_anisotropy;
	float roughness_orientation;
	vec3 specular_reflectance;
};

float oren_nayar_brdf(vec3 n, vec3 l, vec3 v, float roughness)
{
	float roughsqaure = roughness * roughness;
	float vdotn = dot(v, n);
	float ldotn = dot(l, n);
	float sintan = sqrt(max(0.0,(1.0 - vdotn * vdotn) * (1.0 - ldotn * ldotn))) / max(vdotn, ldotn);
	float anglediff = dot(normalize(v - n * vdotn), normalize(l - n * ldotn));
	float a = 1.0 - 0.5 * roughsqaure / (roughsqaure + 0.33);
	float b = 0.45 * roughsqaure / (roughsqaure + 0.09);
	return max(ldotn * (a + b * max(anglediff, 0.0) * sintan), 0.0);
}

float phong_specular_brdf(vec3 normal, vec3 omega_in, vec3 omega_out, float roughness)
{
	vec3 omega_reflect = -reflect(omega_in, normal);
	float fs = dot(omega_reflect, omega_out);
	if (fs <= 0.0)
		return 0.0;
	float m = 1.0 / (roughness + 1.0 / 128.0);
	fs = pow(fs, m);

	return (m+2.0)/(2.0*PI)*fs;
}

float blinn_specular_brdf(vec3 normal, vec3 omega_in, vec3 omega_out, float roughness)
{
	vec3 omega_half = normalize(omega_in + omega_out);
	float fs = dot(omega_half, normal);
	if (fs <= 0.0)
		return 0.0;
	float m = 1.0 / (roughness + 1.0 / 128.0);
	fs = pow(fs, m);
	return (m+2.0)*(m+4.0)/(8.0*PI*(pow(2.0,-0.5*m)+m))*fs;
}

vec3 cooktorrance_specular_brdf(vec3 n, vec3 l, vec3 v, float roughness, vec3 f0) 
{
	float m2 = roughness * roughness+0.0001;
	vec3 h = normalize(l + v);
	float nh = dot(n, h);
	float nl = dot(n, l);
	float nv = dot(n, v);
	float vh = dot(v, h);
	float cos2a = nh * nh;
	float tan2a = (1.0 - cos2a) / cos2a;
	float d = 1.0 / (m2 * cos2a * cos2a) * exp(-tan2a / m2);
	float g = clamp(min(2.0*nh*nv / vh, 2.0*nh*nl / vh), 0.0, 1.0);
	vec3 f = f0 + (vec3(1.0) - f0)*pow(1.0 - vh, 5.0);
	return d * f*g / (nv*PI);
}

float G_strauss(float a)
{
	float kg = 1.01;
	float t0 = 1.0 / pow(abs(   1.0   - kg), 2.0);
	float t1 = 1.0 / pow(abs(2.0*a/PI - kg), 2.0);
	float t2 = 1.0 / pow(kg, 2.0);
	return (t0 - t1) / (t0 - t2);
}

float strauss_diffuse_coeff(float roughness, float metalness, float transparency)
{
	float s = 1.0 - roughness;
	float o = 1.0 - transparency;
	float rd = (1.0 - pow(s, 3.0))*o;
	float d = 1.0 - metalness * s;
	return rd * d;
}

vec3 strauss_specular_brdf(vec3 N, vec3 L, vec3 V, float roughness, float metalness, float transparency, vec3 C)
{
	roughness = 0.999*roughness + 0.001;
	vec3 R = -reflect(L, N);
	float theta_L = acos(clamp(-1.0, 1.0, dot(L, N)));
	float theta_V = acos(clamp(-1.0, 1.0, dot(V, N))); 
	float G_L = G_strauss(theta_L);
	float G_V = G_strauss(theta_V);
	float kf = 1.12;
	float inv_sqr_kf = 1.0 / (kf*kf);

	float x_minus_kf = 2.0 * theta_L / PI - kf;
	float F = (1.0/(x_minus_kf * x_minus_kf) - inv_sqr_kf) /
		      (1.0/((1.0 - kf) * (1.0 - kf)) - inv_sqr_kf);
	float j = F*G_L*G_V;
	float s = 1.0 - roughness;
	float o = 1.0 - transparency;
	float rd = (1.0 - pow(s, 3.0))*o;
	float kj = 0.1;
	float rn = o - rd;
	float rj = min(1.0, rn + (rn + kj)*j);
	float rs = rj * pow(max(0.0,dot(V, R)), 3.0 / roughness);
	return rs * (vec3(1.0) + metalness * (1 - F)*(C - vec3(1.0)));
}

vec3 evaluate_material(Material material, vec3 normal, vec3 omega_in, vec3 omega_out)
{
	vec3 res = vec3(0.0);
	// check whether light illuminates surface point
	float fd = dot(normal, omega_in);
	if (fd < 0.0)
		return res;
	
	// compute diffuse brdf component
	switch (material.brdf_type % 4) {
	case 0: res = fd * material.diffuse_reflectance; break;
	case 1: res = oren_nayar_brdf(normal, omega_in, omega_out, material.roughness) * material.diffuse_reflectance; break;
	case 2: res = fd * strauss_diffuse_coeff(material.roughness, material.metalness, material.transparency) * material.diffuse_reflectance; break;
	}
	// compute specular reflectance
	vec3 spec = material.specular_reflectance;
	switch (material.brdf_type / 4) {
	case 0: return res;
	case 1: spec *= fd * phong_specular_brdf(normal, omega_in, omega_out, material.roughness); break;
	case 2: spec *= fd * blinn_specular_brdf(normal, omega_in, omega_out, material.roughness); break;
	case 3: spec = cooktorrance_specular_brdf(normal, omega_in, omega_out, material.roughness, material.specular_reflectance); break;
	case 4: spec *= strauss_specular_brdf(normal, omega_in, omega_out, material.roughness, material.metalness, material.transparency, material.diffuse_reflectance); break;
	}

	res += spec;
	return res;
}