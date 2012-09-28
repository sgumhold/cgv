#pragma once

#include <cgv/media/color.h> //@<
#include "lib_begin.h" //@<

namespace cgv { //@<
	namespace media { //@<
		namespace illum { //@<

///@>simple class to hold the material properties of a phong material
class CGV_API phong_material
{
public: //@<
	///@>used color type
	typedef color<float,RGB,OPACITY> color_type;
protected: //@<
	///@>ambient color component
	color_type ambient;
	///@>diffuse color component
	color_type diffuse;
	///@>specular color component
	color_type specular;
	///@>emissive color component
	color_type emission;
	///@>exponent of the specular cosine term
	float      shininess;
public: //@<
	/// define default material
	phong_material() : ambient(0.1f,0.1f,0.1f,1), diffuse(0.5f,0.5f,0.5f,1), specular(1,1,1,1), emission(0,0,0,1), shininess(50.0f) {}
	void set_ambient(const color_type& c) { ambient = c; }
	void set_diffuse(const color_type& c) { diffuse = c; }
	void set_specular(const color_type& c) { specular = c; }
	void set_emission(const color_type& c) { emission = c; }
	void set_shininess(float s) { shininess = s; }
	const color_type& get_ambient() const { return ambient; }
	const color_type& get_diffuse() const { return diffuse; }
	const color_type& get_specular() const { return specular; }
	const color_type& get_emission() const { return emission; }
	float get_shininess() const { return shininess; }
	color_type& ref_ambient() { return ambient; }
	color_type& ref_diffuse() { return diffuse; }
	color_type& ref_specular() { return specular; }
	color_type& ref_emission() { return emission; }
	float& ref_shininess() { return shininess; }
}; //@<

		} //@<
	} //@<
} //@<

#include <cgv/config/lib_end.h> //@<