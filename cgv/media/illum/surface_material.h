#pragma once

#include <complex>
#include <cgv/media/color.h>

#include "lib_begin.h"

namespace cgv {
	namespace media { 
		namespace illum {

/// different brdf types
enum BrdfType {
	BT_LAMBERTIAN = 0, // use lambertian for diffuse part
	BT_OREN_NAYAR = 1, // use oren nayar for diffuse part
	BT_STRAUSS_DIFFUSE = 2, // use diffuse part of strauss model for diffuse part
					   // reserve one more diffuse models for future use
	BT_PHONG = 4,      // shininess in range [1,128] is computed roughness
	BT_BLINN = 8,      // shininess in range [1,128] is computed roughness
	BT_COOK_TORRANCE = 12,
	BT_STRAUSS = 16
};

/// simple class to hold the material properties of a phong material
class CGV_API surface_material
{
public:
	/// used color type
	typedef color<float,RGB> color_type;
protected:
	/// store brdf type, defaults to BT_COOK_TORRANCE
	BrdfType brdf_type;
	/// diffuse reflectance of surface, defaults to 0.5,0.5,0.5
	color_type diffuse_reflectance;
	/// surface roughness in the range [0,1] (1/2 trace of symmetric 2x2 matrix for anisotropic case where directional roughness is represented in the uv-coordinate system of texcoords), defaults to 0.5
	float roughness;
	/// metalness of surface
	float metalness;
	/// scalar factor to down scale ambient light, defaults to 1
	float ambient_occlusion;
	/// emissive color component, defaults to 0,0,0
	color_type emission;
	/// modulation for transparency, defaults to 0
	float transparency;
	/// complex fraction of complex interior over real exterior index of refraction, defaults to 1.5,0
	std::complex<float> propagation_slow_down;
	/// difference of roughness matrix eigenvalues in range [0,1] relative to \c roughness, i.e. lambda_1 - lambda_2 = roughness_anisotropy*roughness, defaults to 0
	float roughness_anisotropy;
	/// orientation of roughness in range [0,1], where 0 corresponds to u-direction and 0.5 to v direction, defaults to 0
	float roughness_orientation;
	/// specular color used to modulate specular reflection component, should be 1,1,1
	color_type specular_reflectance;
public: //@<
	/// construct default material
	surface_material(
		BrdfType _brdf_type = BrdfType(BT_STRAUSS_DIFFUSE + BT_STRAUSS),
		color_type _diffuse_reflectance = color_type(0.5f, 0.5f, 0.5f),
		float _roughness = 0.5f,
		float _metalness = 0.0f,
		float _ambient_occlusion = 1.0f,
		color_type _emission = color_type(0,0,0),
		float _transparency = 0.0f,
		const std::complex<float>& _propagation_slow_down = std::complex<float>(1.5f,0.0f),
		float _roughness_anisotropy = 0.0f,
		float _roughness_orientation = 0.0f,
		color_type _specular_reflectance = color_type(1,1,1)
	);

	void set_brdf_type(BrdfType brdf_type) { this->brdf_type = brdf_type; }
	BrdfType get_brdf_type() const { return brdf_type; }
	BrdfType& ref_brdf_type() { return brdf_type; }

	void set_ambient_occlusion(float ambient_occlusion) { this->ambient_occlusion = ambient_occlusion; }
	float get_ambient_occlusion() const { return ambient_occlusion; }
	float& ref_ambient_occlusion() { return ambient_occlusion; }

	void set_diffuse_reflectance(color_type diffuse_reflectance) { this->diffuse_reflectance = diffuse_reflectance; }
	color_type get_diffuse_reflectance() const { return diffuse_reflectance; }
	color_type& ref_diffuse_reflectance() { return diffuse_reflectance; }

	void set_specular_reflectance(color_type specular_reflectance) { this->specular_reflectance = specular_reflectance; }
	color_type get_specular_reflectance() const { return specular_reflectance; }
	color_type& ref_specular_reflectance() { return specular_reflectance; }

	void set_emission(color_type emission) { this->emission = emission; }
	color_type get_emission() const { return emission; }
	color_type& ref_emission() { return emission; }

	void set_transparency(float transparency) { this->transparency = transparency; }
	float get_transparency() const { return transparency; }
	float& ref_transparency() { return transparency; }

	void set_roughness(float roughness) { this->roughness = roughness; }
	float get_roughness() const { return roughness; }
	float& ref_roughness() { return roughness; }

	void set_metalness(float metalness) { this->metalness = metalness; }
	float get_metalness() const { return metalness; }
	float& ref_metalness() { return metalness; }

	void set_roughness_anisotropy(float roughness_anisotropy) { this->roughness_anisotropy = roughness_anisotropy; }
	float get_roughness_anisotropy() const { return roughness_anisotropy; }
	float& ref_roughness_anisotropy() { return roughness_anisotropy; }

	void set_roughness_orientation(float roughness_orientation) { this->roughness_orientation = roughness_orientation; }
	float get_roughness_orientation() const { return roughness_orientation; }
	float& ref_roughness_orientation() { return roughness_orientation; }

	void set_propagation_slow_down(std::complex<float> propagation_slow_down) { this->propagation_slow_down = propagation_slow_down; }
	std::complex<float> get_propagation_slow_down() const { return propagation_slow_down; }
	std::complex<float>& ref_propagation_slow_down() { return propagation_slow_down; }
};

enum StandardMaterial
{
	// transparent
	SM_WATER,
	SM_GLASS,

	// metals
	SM_ALUMINUM,
	SM_SILVER,
	SM_COPPER,
	SM_GOLD
};

/// provide reference to a standard material
extern CGV_API const surface_material& get_surface_material(StandardMaterial material_id);

		}
	}
}

#include <cgv/config/lib_end.h> 