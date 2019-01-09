#version 150

uniform int illumination_mode = 2;

/*
The following interface is implemented in this shader:
//***** begin interface of surface.glsl ***********************************
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

Material get_material();
vec4 compute_reflected_radiance(in Material M, vec3 position_eye, vec3 normal_eye);
vec4 compute_reflected_appearance(vec3 position_eye, vec3 normal_eye, vec4 color, int side);
vec4 compute_reflected_appearance_texture(vec3 position_eye, vec3 normal_eye, vec2 texcoords, vec4 color, int side);
//***** end interface of surface.glsl ***********************************
*/

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
//***** end interface of lights.glsl ***********************************


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

//***** begin interface of side.glsl ***********************************
bool keep_this_side(in vec3 position, in vec3 normal, out int side);
void update_material_color_and_transparency(inout vec3 mat_color, inout float transparency, in int side, in vec4 color);
void update_normal(inout vec3 normal, in int side);
//***** end interface of side.glsl ***********************************

uniform Material material;

Material get_material()
{
	return material;
}

vec4 compute_reflected_radiance(in Material M, vec3 position_eye, vec3 normal_eye)
{
	vec4 res = vec4(0.0, 0.0, 0.0, 1.0-M.transparency);
	for (int i = 0; i < get_nr_light_sources(); ++i) {
		LightSource L = get_light_source(i);
		// add ambient constribution
		res.rgb += M.diffuse_reflectance * M.ambient_occlusion * L.ambient_scale * L.emission;
		// compute direct lighting
		vec3 omega_in;
		vec3 radiance_in;
		evaluate_light(L, position_eye, omega_in, radiance_in);
		// evaluate material 
		res.rgb += evaluate_material(M, normal_eye, omega_in, normalize(-position_eye))*radiance_in;
		// add emission
	}
	res.rgb += M.emission;
	return res;
}

vec4 compute_reflected_appearance(vec3 position_eye, vec3 normal_eye, vec4 color, int side)
{
	if (illumination_mode == 0)
		return color;
	Material M = material;
	update_material_color_and_transparency(M.diffuse_reflectance, M.transparency, side, color);
	update_normal(normal_eye, side);
	return compute_reflected_radiance(M, position_eye, normal_eye);
}

vec4 compute_reflected_appearance_texture(vec3 position_eye, vec3 normal_eye, vec2 texcoords, vec4 color, int side)
{
	if (illumination_mode == 0)
		return color;
	Material M = material;
	update_material_color_and_transparency(M.diffuse_reflectance, M.transparency, side, color);
	update_normal(normal_eye, side);
	return compute_reflected_radiance(M, position_eye, normal_eye);
}

