#include "phong_material.hh"

namespace cgv {
	namespace media { 
		namespace illum { 

	phong_material::phong_material() : ambient(0.1f,0.1f,0.1f,1), diffuse(0.5f,0.5f,0.5f,1), specular(0.5f, 0.5f, 0.5f,1), emission(0,0,0,1), shininess(50.0f) {}
	void phong_material::set_ambient(const color_type& c) { ambient = c; }
	void phong_material::set_diffuse(const color_type& c) { diffuse = c; }
	void phong_material::set_specular(const color_type& c) { specular = c; }
	void phong_material::set_emission(const color_type& c) { emission = c; }
	void phong_material::set_shininess(float s) { shininess = s; }
	const phong_material::color_type& phong_material::get_ambient() const { return ambient; }
	const phong_material::color_type& phong_material::get_diffuse() const { return diffuse; }
	const phong_material::color_type& phong_material::get_specular() const { return specular; }
	const phong_material::color_type& phong_material::get_emission() const { return emission; }
	float phong_material::get_shininess() const { return shininess; }
	phong_material::color_type& phong_material::ref_ambient() { return ambient; }
	phong_material::color_type& phong_material::ref_diffuse() { return diffuse; }
	phong_material::color_type& phong_material::ref_specular() { return specular; }
	phong_material::color_type& phong_material::ref_emission() { return emission; }
	float& phong_material::ref_shininess() { return shininess; }

	const phong_material& default_material() { static phong_material mat; return mat; }

		} 
	}
}
