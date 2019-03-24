#include "textured_surface_material.h"
#include <map>
#include <string>

namespace cgv {
	namespace media { 
		namespace illum { 

/// define default material
textured_surface_material::textured_surface_material(
	const std::string& _name,
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
	color_type _specular_reflectance,
	float _bump_scale) : name(_name), surface_material(_brdf_type, _diffuse_reflectance, _roughness, _metalness, _ambient_occlusion, _emission, _transparency, _propagation_slow_down, _roughness_anisotropy, _roughness_orientation, _specular_reflectance), bump_scale(1)
{
	diffuse_index = -1;
	roughness_index = -1;
	metalness_index = -1;
	ambient_index = -1;
	emission_index = -1;
	transparency_index = -1;
	propagation_slow_down_index = -1;
	normal_index = -1;
	bump_index = -1;
	specular_index = -1;
	sRGBA_textures = false;
}

/// set whether textures are interpreted in sRGB format
void textured_surface_material::set_sRGBA_textures(bool do_set)
{
	sRGBA_textures = do_set;
}

/// convert obj material
textured_surface_material::textured_surface_material(const obj_material& obj_mat)
{
	sRGBA_textures = false;
	diffuse_index = -1;
	roughness_index = -1;
	metalness_index = -1;
	ambient_index = -1;
	emission_index = -1;
	transparency_index = -1;
	propagation_slow_down_index = -1;
	normal_index = -1;
	bump_index = -1;
	specular_index = -1;
	set_name(obj_mat.get_name());
	set_brdf_type(BrdfType(BT_LAMBERTIAN + BT_PHONG));
	obj_material::color_type a = obj_mat.get_ambient(), d = obj_mat.get_diffuse();
	float la = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	float ld = sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
	set_ambient_occlusion(la >= ld ? la : la/ld);
	set_diffuse_reflectance(obj_mat.get_diffuse());
	set_specular_reflectance(obj_mat.get_specular());
	set_emission(obj_mat.get_emission());
	set_roughness(1.0f/obj_mat.get_shininess() - 1.0f/128.0f);
	set_transparency(1.0f - obj_mat.get_opacity());
	set_bump_scale(obj_mat.get_bump_scale());
	std::map<std::string, int> file_name_map;
	if (!obj_mat.get_ambient_texture_name().empty()) {
		if (file_name_map.find(obj_mat.get_ambient_texture_name()) == file_name_map.end())
			file_name_map[obj_mat.get_ambient_texture_name()] = add_image_file(obj_mat.get_ambient_texture_name());
		set_ambient_index(file_name_map[obj_mat.get_ambient_texture_name()]);
	}
	if (!obj_mat.get_diffuse_texture_name().empty()) {
		if (file_name_map.find(obj_mat.get_diffuse_texture_name()) == file_name_map.end())
			file_name_map[obj_mat.get_diffuse_texture_name()] = add_image_file(obj_mat.get_diffuse_texture_name());
		set_diffuse_index(file_name_map[obj_mat.get_diffuse_texture_name()]);
	}
	if (!obj_mat.get_opacity_texture_name().empty()) {
		if (file_name_map.find(obj_mat.get_opacity_texture_name()) == file_name_map.end())
			file_name_map[obj_mat.get_opacity_texture_name()] = add_image_file(obj_mat.get_opacity_texture_name());
		set_transparency_index(file_name_map[obj_mat.get_opacity_texture_name()]);
	}
	if (!obj_mat.get_specular_texture_name().empty()) {
		if (file_name_map.find(obj_mat.get_specular_texture_name()) == file_name_map.end())
			file_name_map[obj_mat.get_specular_texture_name()] = add_image_file(obj_mat.get_specular_texture_name());
		set_specular_index(file_name_map[obj_mat.get_specular_texture_name()]);
	}
	if (!obj_mat.get_emission_texture_name().empty()) {
		if (file_name_map.find(obj_mat.get_emission_texture_name()) == file_name_map.end())
			file_name_map[obj_mat.get_emission_texture_name()] = add_image_file(obj_mat.get_emission_texture_name());
		set_emission_index(file_name_map[obj_mat.get_emission_texture_name()]);
	}
	if (!obj_mat.get_bump_texture_name().empty()) {
		if (file_name_map.find(obj_mat.get_bump_texture_name()) == file_name_map.end())
			file_name_map[obj_mat.get_bump_texture_name()] = add_image_file(obj_mat.get_bump_texture_name());
		set_bump_index(file_name_map[obj_mat.get_bump_texture_name()]);
	}
}

/// add a new image and return its index
int textured_surface_material::add_image_file(const std::string& file_name)
{
	int idx = int(image_file_names.size());
	image_file_names.push_back(file_name);
	return idx;
}



		}
	}
}
