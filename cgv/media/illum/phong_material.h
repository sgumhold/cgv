#pragma once

#include <cgv/media/color.h>
#include "lib_begin.h"

namespace cgv {
	namespace media {
		namespace illum {

/// Stores properties of a phong brdf material.
struct CGV_API phong_material
{
	using color_type = cgv::rgba;

	/// ambient color component
	color_type ambient = { 0.1f, 0.1f, 0.1f, 1.0f };
	/// diffuse color component
	color_type diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
	/// specular color component
	color_type specular = { 0.5f, 0.5f, 0.5f, 1.0f };
	/// emissive color component
	color_type emission = { 0.0f, 0.0f, 0.0f, 1.0f };
	/// exponent of the specular cosine term
	float      shininess = 50.0f;

	/// provide a const reference to a standard phong material
	static const phong_material& get_default();
};

		}
	}
}

#include <cgv/config/lib_end.h>
