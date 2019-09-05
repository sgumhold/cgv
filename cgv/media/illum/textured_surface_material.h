#pragma once

#include <vector>
#include "surface_material.h"
#include "obj_material.hh"

#include "lib_begin.h"

namespace cgv {
	namespace media {
		namespace illum {

/// simple class to hold the material properties of a phong material
class CGV_API textured_surface_material : public surface_material
{
protected:
	/// name of material
	std::string name;
	/// whether textures are in sRGB format
	bool sRGBA_textures;
	/// vector of image file names
	std::vector<std::string> image_file_names;
	/// index of image from which diffuse_reflectance should be mapped, -1 corresponds to no mapping
	int diffuse_index;
	//! index of image from which roughness should be mapped, -1 corresponds to no mapping
	/*! In case of 3 or 4 component textures roughness is mapped anisotropically from the xyz components.
	    For 2 component textures, roughness is mapped from the y-component. */
	int roughness_index;
	//! index of image from which metalness should be mapped, -1 corresponds to no mapping
	/*! In case of 4 component textures metalness is mapped from the w component.
		For 2 component textures, metalness is mapped from the x-component. */
	int metalness_index;
	//! index of image from which ambient_occlusion should be mapped, -1 corresponds to no mapping
	/*! In case of 4 component textures ambient_occlusion is mapped from the w component.*/
	int ambient_index;
	/// index of image from which emission should be mapped, -1 corresponds to no mapping
	int emission_index;
	//! index of image from which transparency should be mapped, -1 corresponds to no mapping
	/*! In case of 4 component textures transparency is mapped from the w component.*/
	int transparency_index;
	/// index of image from which diffuse_reflectance should be mapped, -1 corresponds to no mapping
	int propagation_slow_down_index;
	/// index of image from which specular_reflectance should be mapped, -1 corresponds to no mapping
	int specular_index;
	/// index of image from which normals should be mapped, -1 corresponds to no mapping
	int normal_index;
	//! index of image from which bumps should be mapped, -1 corresponds to no mapping
	/*! If no normal mapping is applies, bump map is also used for normal mapping. */
	int bump_index;
	///scaling factor for bump map
	float bump_scale;
public: //@<
	/// define default material
	textured_surface_material(
		const std::string& _name = "default",
		BrdfType _brdf_type = BrdfType(BT_STRAUSS_DIFFUSE + BT_STRAUSS),
		color_type _diffuse_reflectance = 0.5f,
		float _roughness = 0.5f,
		float _metalness = 0.0f,
		float _ambient_occlusion = 1.0f,
		color_type _emission = color_type(0, 0, 0),
		float _transparency = 0.0f,
		const std::complex<float>& _propagation_slow_down = std::complex<float>(1.5f, 0.0f),
		float _roughness_anisotropy = 0.0f,
		float _roughness_orientation = 0.0f,
		color_type _specular_reflectance = color_type(1, 1, 1),
		float _bump_scale = 0.1f
	);
	/// convert obj material
	textured_surface_material(const obj_material& obj_mat);
	/// set the name of the material
	void set_name(std::string o) { name = o; }
	/// return name value
	const std::string& get_name() const { return name; }
	/// return reference to name value
	std::string& ref_name() { return name; }
	/// set whether textures are interpreted in sRGB format
	void set_sRGBA_textures(bool do_set = true);
	/// return whether textures are interpreted in sRGB format
	bool get_sRGBA_textures() const { return sRGBA_textures; }
	/// return reference to whether textures are interpreted in sRGB format
	bool& ref_sRGBA_textures() { return sRGBA_textures; }
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
	///
	void set_diffuse_index(int i) { diffuse_index = i; }
	int  get_diffuse_index() const { return diffuse_index; }
	int& ref_diffuse_index() { return diffuse_index; }

	void set_roughness_index(int i) { roughness_index = i; }
	int  get_roughness_index() const { return roughness_index; }
	int& ref_roughness_index() { return roughness_index; }

	void set_metalness_index(int i) { metalness_index = i; }
	int  get_metalness_index() const { return metalness_index; }
	int& ref_metalness_index() { return metalness_index; }

	void set_ambient_index(int i) { ambient_index = i; }
	int  get_ambient_index() const { return ambient_index; }
	int& ref_ambient_index() { return ambient_index; }

	void set_emission_index(int i) { emission_index = i; }
	int  get_emission_index() const { return emission_index; }
	int& ref_emission_index() { return emission_index; }

	void set_transparency_index(int i) { transparency_index = i; }
	int  get_transparency_index() const { return transparency_index; }
	int& ref_transparency_index() { return transparency_index; }

	void set_specular_index(int i) { specular_index = i; }
	int  get_specular_index() const { return specular_index; }
	int& ref_specular_index() { return specular_index; }

	void set_normal_index(int i) { normal_index = i; }
	int  get_normal_index() const { return normal_index; }
	int& ref_normal_index() { return normal_index; }

	void set_bump_index(int i) { bump_index = i; }
	int  get_bump_index() const { return bump_index; }
	int& ref_bump_index() { return bump_index; }

	/// set scale of bumps
	void   set_bump_scale(float bs) { bump_scale = bs; }
	/// return bump scale
	float  get_bump_scale() const { return bump_scale; }
	/// return reference to bump scale
	float& ref_bump_scale() { return bump_scale; }
};


		}
	}
}

#include <cgv/config/lib_end.h> 