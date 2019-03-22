#include "surface_material.h"

namespace cgv {
	namespace media { 
		namespace illum { 

surface_material::surface_material(
	BrdfType _brdf_type,
	color_type _diffuse_reflectance,
	float _roughness,
	float _metalness,
	float _ambient_occlusion,
	color_type _emission,
	float _transparency,
	const std::complex<float>& _propagation_slow_down,
	float _roughness_anisotropy,
	float _roughness_orientation,
	color_type _specular_reflectance
) :
	brdf_type(_brdf_type),
	ambient_occlusion(_ambient_occlusion), 
	diffuse_reflectance(_diffuse_reflectance),
	specular_reflectance(_specular_reflectance),
	emission(_emission),
	transparency(_transparency),
	roughness(_roughness),
	metalness(_metalness),
	roughness_anisotropy(_roughness_anisotropy),
	roughness_orientation(_roughness_orientation),
	propagation_slow_down(_propagation_slow_down)
{}

const surface_material& get_surface_material(StandardMaterial material_id)
{

	static surface_material default_materials[]{
		{ BT_COOK_TORRANCE, {0.5f,0.5f,0.5f}, 0.5f, 0.0f, 1.0f,{0.0f,0.0f,0.0f}, 0.0f, {1.3f,0.0f}, 0.0f, 0.0f, {1.0f,1.0f,1.0f} }
	};
	return default_materials[material_id];
}

		} 
	}
}
