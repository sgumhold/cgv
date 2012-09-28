#pragma once

#include <cgv/media/illum/obj_material.hh>
#include "texture.h"

#include "lib_begin.h"

namespace cgv {
	namespace render {

/// class that extends obj_material with the management of textures
class CGV_API textured_material : public media::illum::obj_material
{
public:
	/// different test functions for alpha test
	enum AlphaTestFunc { AT_ALWAYS, AT_LESS, AT_EQUAL, AT_GREATER };
protected:
	texture* ambient_texture;
	texture* diffuse_texture;
	texture* specular_texture;
	texture* emission_texture;
	texture* bump_texture;
	float alpha_threshold;
	AlphaTestFunc alpha_test_func;
public:
	/// initialize textures
	textured_material();
	/// construct from obj_material
	textured_material(const media::illum::obj_material& mtl);
	/// call this to ensure that the textures are loaded - typically done in the init_frame method of a drawable
	void ensure_textures(context& ctx);
	/// configure the alpha test that is performed in case alpha values are given in the textures
	void set_alpha_test(AlphaTestFunc _alpha_test_func = AT_GREATER, float _alpha_threshold = 0.0f);
	/// return the currently set alpha test function
	AlphaTestFunc get_alpha_test_func() const;
	/// return the currently used alpha threshold used by the comparison alpha test functions
	float get_alpha_threshold() const;
	/// return reference to currently set alpha test function
	AlphaTestFunc& ref_alpha_test_func();
	/// return reference to currently used alpha threshold used by the comparison alpha test functions
	float& ref_alpha_threshold();
	/// enable by modulating opacities of material with given opacity value
	void enable(context& ctx, float opacity = 1.0f);
	/// disable material
	void disable(context& ctx);
	/// destruct textures
	void destruct_textures(context& ctx, TextureType textures = TT_ALL_TEXTURES);
	/// return pointer to ambient texture or 0 if non created
	texture* get_ambient_texture() const;
	/// return pointer to diffuse texture or 0 if non created
	texture* get_diffuse_texture() const;
	/// return pointer to specular texture or 0 if non created
	texture* get_specular_texture() const;
	/// return pointer to emission texture or 0 if non created
	texture* get_emission_texture() const;
	/// return pointer to bump map texture or 0 if non created
	texture* get_bump_texture() const;
};

	}
}

#include <cgv/config/lib_end.h>