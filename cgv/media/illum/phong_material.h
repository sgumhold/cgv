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
	/// construct default material
	phong_material();
	void set_ambient(const color_type& c);
	void set_diffuse(const color_type& c);
	void set_specular(const color_type& c);
	void set_emission(const color_type& c);
	void set_shininess(float s);
	const color_type& get_ambient() const;
	const color_type& get_diffuse() const;
	const color_type& get_specular() const;
	const color_type& get_emission() const;
	float get_shininess() const;
	color_type& ref_ambient();
	color_type& ref_diffuse();
	color_type& ref_specular();
	color_type& ref_emission();
	float& ref_shininess();
}; //@<

/// provide reference to a standard material
extern CGV_API const phong_material& default_material();

		} //@<
	} //@<
} //@<

#include <cgv/config/lib_end.h> //@<