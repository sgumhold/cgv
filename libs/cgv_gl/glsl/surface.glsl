#version 150

// use cgv/render/context/IlluminationMode enum (0 .. off, 1 .. one sided, 2 .. two sided)
uniform int illumination_mode = 2;
uniform bool sRGBA_textures = true;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;
uniform sampler2D tex5;

uniform int diffuse_index = -1;
uniform int roughness_index = -1;
uniform int metalness_index = -1;
uniform int ambient_index = -1;
uniform int emission_index = -1;
uniform int transparency_index = -1;
// not yet implemented: uniform int propagation_slow_down_index = -1;
uniform int specular_index = -1;
uniform bool multiply_diffuse_texture_color = false;

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
vec4 lookup_texture(int ti, vec2 texcoords, bool is_color);
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

//***** begin interface of bump_map.glsl ***********************************
void update_normal_from_material(in vec3 s, inout vec3 N, in Material M, in vec2 tc);
//***** end interface of bump_map.glsl ***********************************


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
		// add ambient contribution
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
	if (illumination_mode == 0) {
		vec3 col = material.diffuse_reflectance;
		float tra = material.transparency;
		update_material_color_and_transparency(col, tra, side, color);
		return vec4(col, 1.0-tra);
	}
	Material M = material;
	update_material_color_and_transparency(M.diffuse_reflectance, M.transparency, side, color);
	update_normal(normal_eye, side);
	return compute_reflected_radiance(M, position_eye, normal_eye);
}

vec4 lookup_texture(int ti, vec2 texcoords, bool is_color)
{
	vec4 rgba = vec4(0.0);
	switch (ti) {
	case 0: rgba = texture(tex0, texcoords); break;
	case 1: rgba = texture(tex1, texcoords); break;
	case 2: rgba = texture(tex2, texcoords); break;
	case 3: rgba = texture(tex3, texcoords); break;
	case 4: rgba = texture(tex4, texcoords); break;
	case 5: rgba = texture(tex5, texcoords); break;
	}
	if (is_color && sRGBA_textures)
		rgba.rgb = pow(rgba.rgb, vec3(2.2));
	return rgba;
}

void update_material_from_texture(inout Material M, in vec2 texcoords)
{
	int trans_idx = transparency_index;
	int metal_idx = metalness_index;
	int ambie_idx = ambient_index;

	if (diffuse_index > -1) {
		vec4 col = lookup_texture(diffuse_index, texcoords, true);
		if (multiply_diffuse_texture_color)
			M.diffuse_reflectance *= col.rgb;
		else
			M.diffuse_reflectance = col.rgb;
		if (trans_idx == diffuse_index) {
			M.transparency = 1.0-col.a;
			trans_idx = -1;
		}
		else if (ambie_idx == diffuse_index) {
			M.ambient_occlusion = col.a;
			ambie_idx = -1;
		}
	}
	if (emission_index > -1) {
		vec4 col = lookup_texture(emission_index, texcoords, true);
		M.emission = col.rgb;
		if (trans_idx == emission_index) {
			M.transparency = 1.0 - col.a;
			trans_idx = -1;
		}
		else if (ambie_idx == emission_index) {
			M.ambient_occlusion = col.a;
			ambie_idx = -1;
		}
	}
	if (specular_index > -1) {
		vec4 col = lookup_texture(specular_index, texcoords, true);
		M.specular_reflectance = col.rgb;
		if (trans_idx == specular_index) {
			M.transparency = 1.0 - col.a;
			trans_idx = -1;
		}
		else if (ambie_idx == specular_index) {
			M.ambient_occlusion = col.a;
			ambie_idx = -1;
		}
	}
	if (roughness_index > -1) {
		vec4 col = lookup_texture(roughness_index, texcoords, false);
		M.roughness = col.r;
		if (roughness_index == metal_idx) {
			M.metalness = col.g;
			metal_idx = -1;
			if (roughness_index == ambie_idx) {
				M.ambient_occlusion = col.b;
				ambie_idx = -1;
				if (roughness_index == trans_idx) {
					M.transparency = 1.0 - col.a;
					trans_idx = -1;
				}
			}
			else {
				if (roughness_index == trans_idx) {
					M.transparency = 1.0 - col.b;
					trans_idx = -1;
				}
			}
		}
	}
	if (metal_idx > -1) {
		vec4 col = lookup_texture(metal_idx, texcoords, false);
		M.metalness = col.r;
		if (metal_idx == ambie_idx) {
			M.ambient_occlusion = col.g;
			ambie_idx = -1;
			if (metal_idx == trans_idx) {
				M.transparency = 1.0 - col.b;
				trans_idx = -1;
			}
		}
		else {
			if (metal_idx == trans_idx) {
				M.transparency = 1.0 - col.g;
				trans_idx = -1;
			}
		}
	}
	if (ambie_idx > -1) {
		vec4 col = lookup_texture(ambie_idx, texcoords, true);
		M.ambient_occlusion = col.r;
	}
	if (trans_idx > -1) {
		vec4 col = lookup_texture(trans_idx, texcoords, false);
		M.transparency = 1.0 - col.r;
	}
}

vec4 compute_reflected_appearance_texture(vec3 position_eye, vec3 normal_eye, vec2 texcoords, vec4 color, int side)
{
	Material M = material;
	update_material_from_texture(M, texcoords);
	if (illumination_mode == 0) {
		vec3 col = M.diffuse_reflectance;
		float tra = M.transparency;
		update_material_color_and_transparency(col, tra, side, color);
		return vec4(col, 1.0 - tra);
	}
	update_material_color_and_transparency(M.diffuse_reflectance, M.transparency, side, color);
	update_normal_from_material(position_eye, normal_eye, M, texcoords);
	update_normal(normal_eye, side);
	return compute_reflected_radiance(M, position_eye, normal_eye);
}

