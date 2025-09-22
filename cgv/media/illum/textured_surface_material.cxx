#include "textured_surface_material.h"
#include <map>
#include <string>

namespace cgv {
	namespace media { 
		namespace illum { 

textured_surface_material::textured_surface_material(const obj_material& obj_mat) {
	name = obj_mat.name;
	brdf_type = BrdfType(BT_LAMBERTIAN + BT_PHONG);
	obj_material::color_type a = obj_mat.ambient, d = obj_mat.diffuse;
	float la = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	float ld = sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
	ambient_occlusion = la >= ld ? la : la / ld;
	diffuse_reflectance = obj_mat.diffuse;
	specular_reflectance = obj_mat.specular;
	emission = obj_mat.emission;
	roughness = std::max(0.0f, std::min(1.0f, 1.0f / (obj_mat.shininess + 0.992307f) - 0.0077524f));
	transparency = 1.0f - obj_mat.opacity;
	bump_scale = obj_mat.bump_scale;

	std::map<std::string, int> file_name_map;

	const auto& add_texture = [this, &file_name_map](const std::string& texture_file_name, int& texture_index) {
		if(!texture_file_name.empty()) {
			// Insert the texture file name and index into the map if it does not already exist.
			if(file_name_map.find(texture_file_name) == file_name_map.end())
				file_name_map[texture_file_name] = add_image_file(texture_file_name);
			// Get the index which is either the newly added index or an index from previous entries with the same texture name.
			texture_index = file_name_map[texture_file_name];
		}
	};

	add_texture(obj_mat.ambient_texture_name, ambient_index);
	add_texture(obj_mat.diffuse_texture_name, diffuse_index);
	add_texture(obj_mat.opacity_texture_name, transparency_index);
	add_texture(obj_mat.specular_texture_name, specular_index);
	add_texture(obj_mat.emission_texture_name, emission_index);
	add_texture(obj_mat.bump_texture_name, bump_index);
}

/// add a new image and return its index
int textured_surface_material::add_image_file(const std::string& file_name) {
	int idx = static_cast<int>(image_file_names.size());
	image_file_names.push_back(file_name);
	return idx;
}

		}
	}
}
