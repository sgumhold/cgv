#pragma once

#include <cgv/media/illum/phong_material.h>
#include "lib_begin.h"

namespace cgv {
	namespace media {
		namespace illum {

/// Extension of a phong material with support for texture-mapped color channels.
struct CGV_API obj_material : public phong_material
{
	/// different types of textures
	enum TextureType {
		TT_AMBIENT_TEXTURE = 1,
		TT_DIFFUSE_TEXTURE = 2,
		TT_OPACITY_TEXTURE = 4,
		TT_SPECULAR_TEXTURE = 8,
		TT_EMISSION_TEXTURE = 16,
		TT_BUMP_TEXTURE = 32,
		TT_ALL_TEXTURES = 63
	};

	/// name of material
	std::string name = "default";
	/// opacity value
	float opacity = 1.0f;
	/// file name of ambient texture
	std::string ambient_texture_name; 
	/// file name of diffuse texture
	std::string diffuse_texture_name;
	/// file name of opacity texture
	std::string opacity_texture_name;
	/// file name of specular texture
	std::string specular_texture_name; 
	/// file name of emission texture
	std::string emission_texture_name;
	/// scaling factor for bump map
	float bump_scale = 1.0f;
	/// file name of bump map texture
	std::string bump_texture_name;
};

		}
	}
}

#include <cgv/config/lib_end.h>
