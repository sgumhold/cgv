#pragma once

#include <cgv/media/illum/phong_material.h> //@<
#include "lib_begin.h" //@<

namespace cgv { //@<
	namespace media { //@<
		namespace illum { //@<

///@>extension of a phong material with support for texture mapped color channels
class CGV_API obj_material : public phong_material
{
protected: //@<
	///@> name of material
	std::string name;
	///@> opacity value
	float opacity;
	///@> file name of ambient texture
	std::string ambient_texture_name; 
	///@> file name of diffuse texture
	std::string diffuse_texture_name;
	///@> file name of opacity texture
	std::string opacity_texture_name;
	///@> file name of specular texture
	std::string specular_texture_name; 
	///@> file name of emission texture
	std::string emission_texture_name;
	///@> scaling factor for bump map
	float bump_scale;
	///@> file name of bump map texture
	std::string bump_texture_name;
public: //@<
	///@> different types of textures
	enum TextureType { 
		TT_AMBIENT_TEXTURE = 1,
		TT_DIFFUSE_TEXTURE = 2,
		TT_OPACITY_TEXTURE = 4,
		TT_SPECULAR_TEXTURE = 8,
		TT_EMISSION_TEXTURE = 16,
		TT_BUMP_TEXTURE = 32,
		TT_ALL_TEXTURES = 63
	};
	/// define default material
	obj_material() : name("default"), opacity(1), bump_scale(1) {}
	/// set opacity value
	void   set_opacity(float o) { opacity = o; }
	/// return opacity value
	float  get_opacity() const  { return opacity; }
	/// return reference to opacity value
	float& ref_opacity()        { return opacity; }
	/// set name value
	void         set_name(std::string o) { name = o; }
	/// return name value
	const std::string& get_name() const  { return name; }
	/// return reference to name value
	std::string&       ref_name()        { return name; }
	/// set ambient_texture_name value
	virtual void set_ambient_texture_name(std::string o) { ambient_texture_name = o; }
	/// return ambient_texture_name value
	const std::string& get_ambient_texture_name() const  { return ambient_texture_name; }
	/// return reference to ambient_texture_name value
	std::string&       ref_ambient_texture_name()        { return ambient_texture_name; }
	/// set diffuse_texture_name value
	virtual void set_diffuse_texture_name(std::string o) { diffuse_texture_name = o; }
	/// return diffuse_texture_name value
	const std::string& get_diffuse_texture_name() const  { return diffuse_texture_name; }
	/// return reference to diffuse_texture_name value
	std::string&       ref_diffuse_texture_name()        { return diffuse_texture_name; }
	/// set opacity_texture_name value
	virtual void set_opacity_texture_name(std::string o) { opacity_texture_name = o; }
	/// return opacity_texture_name value
	const std::string& get_opacity_texture_name() const { return opacity_texture_name; }
	/// return reference to opacity_texture_name value
	std::string&       ref_opacity_texture_name() { return opacity_texture_name; }
	/// set specular_texture_name value
	virtual void set_specular_texture_name(std::string o) { specular_texture_name = o; }
	/// return specular_texture_name value
	const std::string& get_specular_texture_name() const  { return specular_texture_name; }
	/// return reference to specular_texture_name value
	std::string&       ref_specular_texture_name()        { return specular_texture_name; }
	/// set emission_texture_name value
	virtual void set_emission_texture_name(std::string o) { emission_texture_name = o; }
	/// return emission_texture_name value
	const std::string& get_emission_texture_name() const  { return emission_texture_name; }
	/// return reference to emission_texture_name value
	std::string&       ref_emission_texture_name()        { return emission_texture_name; }
	/// set bump_texture_name value
	virtual void set_bump_texture_name(std::string b)     { bump_texture_name = b; }
	/// return bump_texture_name value
	const std::string& get_bump_texture_name() const      { return bump_texture_name; }
	/// return reference to bump_texture_name value
	std::string&       ref_bump_texture_name()            { return bump_texture_name; }
	/// set scale of bumps
	void   set_bump_scale(float bs) { bump_scale = bs; }
	/// return bump scale
	float  get_bump_scale() const  { return bump_scale; }
	/// return reference to bump scale
	float& ref_bump_scale()        { return bump_scale; }
};

		}
	}
}

#include <cgv/config/lib_end.h> //@<