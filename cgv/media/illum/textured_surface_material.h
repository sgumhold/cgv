#pragma once

#include <vector>
#include "surface_material.h"
#include "obj_material.h"

#include "lib_begin.h"

namespace cgv {
	namespace media {
		namespace illum {

/// Stores properties of a surface material with support for texturing.
class CGV_API textured_surface_material : public surface_material
{
public:
	/// name of material
	std::string name = "default";
	/// whether textures are in sRGB format
	bool sRGBA_textures = false;
	/// index of image from which diffuse_reflectance should be mapped, -1 corresponds to no mapping
	int diffuse_index = -1;
	//! index of image from which roughness should be mapped, -1 corresponds to no mapping
	/*! In case of 3 or 4 component textures roughness is mapped anisotropically from the xyz components.
	    For 2 component textures, roughness is mapped from the y-component. */
	int roughness_index = -1;
	//! index of image from which metalness should be mapped, -1 corresponds to no mapping
	/*! In case of 4 component textures metalness is mapped from the w component.
		For 2 component textures, metalness is mapped from the x-component. */
	int metalness_index = -1;
	//! index of image from which ambient_occlusion should be mapped, -1 corresponds to no mapping
	/*! In case of 4 component textures ambient_occlusion is mapped from the w component.*/
	int ambient_index = -1;
	/// index of image from which emission should be mapped, -1 corresponds to no mapping
	int emission_index = -1;
	//! index of image from which transparency should be mapped, -1 corresponds to no mapping
	/*! In case of 4 component textures transparency is mapped from the w component.*/
	int transparency_index = -1;
	/// index of image from which diffuse_reflectance should be mapped, -1 corresponds to no mapping
	int propagation_slow_down_index = -1;
	/// index of image from which specular_reflectance should be mapped, -1 corresponds to no mapping
	int specular_index = -1;
	/// index of image from which normals should be mapped, -1 corresponds to no mapping
	int normal_index = -1;
	//! index of image from which bumps should be mapped, -1 corresponds to no mapping
	/*! If no normal mapping is applies, bump map is also used for normal mapping. */
	int bump_index = -1;
	///scaling factor for bump map
	float bump_scale = 0.1f;

	/// construct using default values
	textured_surface_material() {}
	/// construct from obj material
	textured_surface_material(const obj_material& obj_mat);

	/// return number of image files
	unsigned get_nr_image_files() const { return unsigned(image_file_names.size()); }
	/// add a new image and return its index
	int add_image_file(const std::string& file_name);
	/// return the name of the i-th image file
	std::string get_image_file_name(int i) const { return image_file_names[i]; }
	/// set the image file name of i-th image file
	void set_image_file_name(int i, std::string image_file_name) { image_file_names[i] = image_file_name; }
	/// return reference to image file name of i-th image file
	std::string& ref_image_file_name(int i) { return image_file_names[i]; }
	/// virtual method to query number of textures
	virtual size_t get_nr_textures() const { return get_nr_image_files(); }

protected:
	/// vector of image file names
	std::vector<std::string> image_file_names;
};
		}
	}
}

#include <cgv/config/lib_end.h>
