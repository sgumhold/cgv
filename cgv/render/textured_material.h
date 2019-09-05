#pragma once

#include <cgv/media/illum/textured_surface_material.h>
#include "texture.h"

#include "lib_begin.h"

namespace cgv {
	namespace render {

/// class that extends obj_material with the management of textures
class CGV_API textured_material : public media::illum::textured_surface_material
{
public:
	/// different test functions for alpha test
	enum AlphaTestFunc { AT_ALWAYS, AT_LESS, AT_EQUAL, AT_GREATER };
protected:
	std::vector<texture*> textures;
	// store context pointer to destruct textures
	context* ctx_ptr;

	float alpha_threshold;
	AlphaTestFunc alpha_test_func;
public:
	/// initialize textures
	textured_material();
	/// ensure that textures are destructed
	virtual ~textured_material();
	/// construct from textured surface material
	textured_material(const media::illum::textured_surface_material& mtl);
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
	/// call this to ensure that the textures specified by image files are loaded - typically done in the init_frame method of a drawable
	bool ensure_textures(context& ctx);
	//! add a reference to a new texture that is managed outside of this class and return its index
	/*! all image file based textures need to be added with add_image_file before calling
	    this function. */
	int add_texture_reference(cgv::render::texture& tex);
	/// virtual method to query number of textures
	size_t get_nr_textures() const { return textures.size(); }

	/// return pointer to ambient texture or 0 if non created
	texture* get_texture(int texture_index) const;
	/// enable all textures with their indices as texture unit
	void enable_textures(context& ctx);
	/// disable material textures
	void disable_textures(context& ctx);
	/// destruct textures
	void destruct_textures(context& ctx);
};

	}
}

#include <cgv/config/lib_end.h>