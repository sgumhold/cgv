#include "textured_material.h"

//#include <cgv_gl/gl/gl.h>

namespace cgv {
	namespace render {

/// initialize textures
textured_material::textured_material()
{
	ambient_texture = 0;
	diffuse_texture = 0;
	specular_texture = 0;
	emission_texture = 0;
	bump_texture = 0;
	alpha_test_func = AT_GREATER;
	alpha_threshold = 0.0f;

}

/// construct from obj_material
textured_material::textured_material(const media::illum::obj_material& mtl)
	: media::illum::obj_material(mtl)
{
	ambient_texture = 0;
	diffuse_texture = 0;
	specular_texture = 0;
	emission_texture = 0;
	bump_texture = 0;
	alpha_test_func = AT_GREATER;
	alpha_threshold = 0.0f;
}

void ensure_texture(context& ctx, texture*& T, const std::string& file_name)
{
	if (T)
		return;
	if (file_name.empty())
		return;
	T = new texture("uint8[L]");
	if (!T->create_from_image(ctx, file_name)) {
		std::cerr << "could not create texture from file '" << file_name << "': " << T->get_last_error() << std::endl;
		delete T;
		T = 0;
	}
	else {
		T->set_wrap_s(TW_REPEAT);
		T->set_wrap_t(TW_REPEAT);
	}
}

/// call this to ensure that the textures are loaded - typically done in the init_frame method of a drawable
void textured_material::ensure_textures(context& ctx)
{
	ensure_texture(ctx, ambient_texture, ambient_texture_name);
	ensure_texture(ctx, diffuse_texture, diffuse_texture_name);
	ensure_texture(ctx, specular_texture, specular_texture_name);
	ensure_texture(ctx, emission_texture, emission_texture_name);
	ensure_texture(ctx, bump_texture, bump_texture_name);
}

/// configure the alpha test that is performed in case alpha values are given in the textures
void textured_material::set_alpha_test(AlphaTestFunc _alpha_test_func, float _alpha_threshold)
{
	alpha_test_func = _alpha_test_func;
	alpha_threshold = _alpha_threshold;
}


/// return the currently set alpha test function
textured_material::AlphaTestFunc textured_material::get_alpha_test_func() const
{
	return alpha_test_func;
}


/// return the currently used alpha threshold used by the comparison alpha test functions
float textured_material::get_alpha_threshold() const
{
	return alpha_threshold;
}

/// return reference to currently set alpha test function
textured_material::AlphaTestFunc& textured_material::ref_alpha_test_func()
{
	return alpha_test_func;
}
/// return reference to currently used alpha threshold used by the comparison alpha test functions
float& textured_material::ref_alpha_threshold()
{
	return alpha_threshold;
}

void destruct_texture(context& ctx, texture*& T)
{
	if (!T)
		return;
	T->destruct(ctx);
	delete T;
	T = 0;
}

/// destruct textures
void textured_material::destruct_textures(context& ctx, TextureType textures)
{
	if (textures & TT_AMBIENT_TEXTURE)
		destruct_texture(ctx, ambient_texture);
	if (textures & TT_DIFFUSE_TEXTURE)
		destruct_texture(ctx, diffuse_texture);
	if (textures & TT_SPECULAR_TEXTURE)
		destruct_texture(ctx, specular_texture);
	if (textures & TT_EMISSION_TEXTURE)
		destruct_texture(ctx, emission_texture);
	if (textures & TT_BUMP_TEXTURE)
		destruct_texture(ctx, bump_texture);
}

/// enable by modulating opacities of material with given opacity value
void textured_material::enable(context& ctx, float alpha)
{
	ctx.enable_material(*this, MS_FRONT_AND_BACK, alpha*opacity);
}

/// disable material
void textured_material::disable(context& ctx)
{
	ctx.disable_material(*this);
}

/// return pointer to ambient texture or 0 if non created
texture* textured_material::get_ambient_texture() const
{
	return ambient_texture;
}
/// return pointer to diffuse texture or 0 if non created
texture* textured_material::get_diffuse_texture() const
{
	return diffuse_texture;
}
/// return pointer to specular texture or 0 if non created
texture* textured_material::get_specular_texture() const
{
	return specular_texture;
}
/// return pointer to emission texture or 0 if non created
texture* textured_material::get_emission_texture() const
{
	return emission_texture;
}
/// return pointer to bump map texture or 0 if non created
texture* textured_material::get_bump_texture() const
{
	return bump_texture;
}

	}
}

