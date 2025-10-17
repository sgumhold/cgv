#pragma once

#include <complex>
#include <cgv/media/color.h>

#include "lib_begin.h"

namespace cgv {
	namespace media { 
		namespace illum {

/// different brdf types
enum BrdfType {
	BT_LAMBERTIAN = 0,		// use lambertian for diffuse part
	BT_OREN_NAYAR = 1,		// use oren nayar for diffuse part
	BT_STRAUSS_DIFFUSE = 2,	// use diffuse part of strauss model for diffuse part
							// reserve one more diffuse models for future use
	BT_PHONG = 4,			// shininess in range [1,128] is computed roughness
	BT_BLINN = 8,			// shininess in range [1,128] is computed roughness
	BT_COOK_TORRANCE = 12,
	BT_STRAUSS = 16
};

/// Stores properties of a surface material.
struct CGV_API surface_material {
	using color_type = cgv::rgb;
	/// store brdf type, defaults to BT_STRAUSS_DIFFUSE + BT_STRAUSS
	BrdfType brdf_type = BrdfType(BT_STRAUSS_DIFFUSE + BT_STRAUSS);
	/// diffuse reflectance of surface, defaults to 0.5,0.5,0.5
	color_type diffuse_reflectance = { 0.5f };
	/// surface roughness in the range [0,1] (1/2 trace of symmetric 2x2 matrix for anisotropic case where directional roughness is represented in the uv-coordinate system of texcoords), defaults to 0.5
	float roughness = 0.5f;
	/// metalness of surface, defaults to 0
	float metalness = 0.0f;
	/// scalar factor to down scale ambient light, defaults to 1
	float ambient_occlusion = 1.0f;
	/// emissive color component, defaults to 0,0,0
	color_type emission = { 0.0f };
	/// modulation for transparency, defaults to 0
	float transparency = 0.0f;
	/// complex fraction of complex interior over real exterior index of refraction, defaults to 1.5,0
	std::complex<float> propagation_slow_down = { 1.5f, 0.0f };
	/// difference of roughness matrix eigenvalues in range [0,1] relative to \c roughness, i.e. lambda_1 - lambda_2 = roughness_anisotropy*roughness, defaults to 0
	float roughness_anisotropy = 0.0f;
	/// orientation of roughness in range [0,1], where 0 corresponds to u-direction and 0.5 to v direction, defaults to 0
	float roughness_orientation = 0.0f;
	/// specular color used to modulate specular reflection component, should be 1,1,1
	color_type specular_reflectance = { 1.0f };
};

		}
	}
}

#include <cgv/config/lib_end.h>
