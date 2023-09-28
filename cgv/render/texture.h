#pragma once

#include <cgv/data/data_format.h>
#include <cgv/data/data_view.h>
#include <cgv/render/context.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {

/** the texture class encapsulates all functionality independent
    of the rendering api. */
class CGV_API texture : public texture_base, public cgv::data::data_format
{
protected:
	mutable bool state_out_of_date;
	int  tex_unit;
	bool complete_create(const context& ctx, bool created);
public:
	using render_component::last_error;

	/**@name methods that can be called without context */
	//@{
	/** construct from description string (which defaults to rgba format)
	    and most commonly used texture parameters. The description can define a component
		 format as described in \c cgv::data::component_format::set_component_format()
		 or a data format as described in \c cgv::data::data_format::set_data_format(). */
	texture(const std::string& description = "uint8[R,G,B,A]", 
			  TextureFilter _mag_filter = TF_LINEAR, 
		     TextureFilter _min_filter = TF_LINEAR, 
			  TextureWrap   _wrap_s = TW_CLAMP_TO_EDGE,
			  TextureWrap   _wrap_t = TW_CLAMP_TO_EDGE,
			  TextureWrap   _wrap_r = TW_CLAMP_TO_EDGE);
	/** destruct texture, the destructor can be called without context 
	    if the destruct method has been called or no creation has 
		 taken place */
	~texture();
	/// change the data format and clear internal format
	bool set_data_format(const std::string& description);
	/// change component format and clear internal format
	void set_component_format(const component_format& cf);
	/// change component format and clear internal format
	void set_component_format(const std::string& description);
	/// set the texture wrap behaviour in s direction
	void set_wrap_s(TextureWrap _wrap_s);
	/// set the texture wrap behaviour in t direction
	void set_wrap_t(TextureWrap _wrap_t);
	/// set the texture wrap behaviour in r direction
	void set_wrap_r(TextureWrap _wrap_r);
	/// return the texture wrap behaviour in s direction
	TextureWrap get_wrap_s() const;
	/// return the texture wrap behaviour in t direction
	TextureWrap get_wrap_t() const;
	/// return the texture wrap behaviour in r direction
	TextureWrap get_wrap_r() const;
	/// set the border color
	void set_border_color(const float* rgba) { set_border_color(rgba[0],rgba[1],rgba[2],rgba[3]); }
	/// set the border color
	void set_border_color(float r, float g, float b, float a = 1.0f);
	/** set the minification filters, if minification is set to TF_ANISOTROP, 
	    the second floating point parameter specifies the degree of anisotropy */
	void set_min_filter(TextureFilter _min_filter, float _anisotropy = 2.0f);
	/// return the minification filter
	TextureFilter get_min_filter() const;
	/// return the currently set anisotropy
	float get_anisotropy() const;
	/// set the magnification filter
	void set_mag_filter(TextureFilter _mag_filter);
	/// return the magnification filter
	TextureFilter get_mag_filter() const;
	/// set priority with which texture is kept in GPU memory
	void set_priority(float _priority);
	/// return the priority with which texture is kept in GPU memory
	float get_priority() const;
	/// set the texture compare mode and function
	void set_compare_mode(bool use_compare_function);
	/// get the texture compare mode and function
	bool get_compare_mode() const;
	/// set the texture compare function
	void set_compare_function(CompareFunction compare_function);
	/// get the texture compare function
	CompareFunction get_compare_function() const;
	/// check whether mipmaps have been created
	bool mipmaps_created() const;
	/// set the number of multi samples for textures of type TT_MULTISAMPLE_2D and TT_MULTISAMPLE_2D_ARRAY
	void set_nr_multi_samples(unsigned _nr_samples);
	/// return number of multi samples for textures of type TT_MULTISAMPLE_2D and TT_MULTISAMPLE_2D_ARRAY
	unsigned get_nr_multi_samples() const { return nr_multi_samples; }
	/// whether multi sampling uses fixed sample locations
	bool use_fixed_sample_locations() const { return fixed_sample_locations; }
	/// set whether multi sampling uses fixed sample locations
	void set_fixed_sample_locations(bool use);
	/// ensure the the texture state is synchronized with the GPU settings
	void ensure_state(const context& ctx) const;
	/// check whether textue is enabled
	bool is_enabled() const;
	/// return the currently used texture unit and -1 for current
	int get_tex_unit() const;
	//@}

	/**@name methods that can be called only with a context */
	//@{
	/// find the format that matches the one specified in the component format best
	void find_best_format(const context& ctx, const std::vector<cgv::data::data_view>* palettes = 0);
	/** create the texture of dimension and resolution specified in 
	    the data format base class. If no type is given, determine it
		from the format. If dimensions are given, they overwrite the
		dimensions in the texture format. */
	bool create(const context& ctx, TextureType _tt = TT_UNDEF, unsigned width=-1, unsigned height=-1, unsigned depth=-1);
	/** create the texture from an image file. If the file_name is not
	    given ask the user interactively for a file name. The image dimensions
		are written to the optionally passed pointers image_width_ptr and 
		image_height_ptr. In order to support mipmapping, the width and height 
		of the texture must be powers of two. If the clear_color_ptr is given,
		the texture is automatically extended in size to powers of two. clear_color_ptr 
		points to four bytes that specify a RGBA color used to fill texels that are inserted
		to ensure dimensions of powers of two. For creation of a side of a cubemap specify
		the cube_side parameter. */
	bool create_from_image(const context& ctx, const std::string& file_name = "",
		                    int* image_width_ptr = 0, int* image_height_ptr = 0, 
							unsigned char* clear_color_ptr = 0, int level = -1, int cube_side = -1);
	/** same as previous method but use the passed data format and data view to
	    store the content of the image. No pointers to width and height can be passed as
		this information is available in the data_format after the call. */
	bool create_from_image(cgv::data::data_format& df, cgv::data::data_view& dv, const context& ctx, const std::string& file_name = "",
		                   unsigned char* clear_color_ptr = 0, int level = -1, int cube_side = -1);
	//! Helper function that determins the individual file names for a cubemap.
	/*! Returns whether the file_names specification was correct. See comment to create_from_images for
	    the syntax of the file_names parameter. */
	static bool deduce_file_names(const std::string& file_names, std::vector<std::string>& deduced_names);
	//! Create a cube map from six files specified in the file_names parameter.
	/*! The file_names parameter can specify different file names by the use of the wildchar
	    symbol '*' or by curly brackets. When using '*' the files are loaded in alphanumerical
		order. The image files defining the cubemap sides must be in the order positive  x, negative
		x, positive y, negative y, positive z and negative z. Some examples of specifying file names:
		- "cm_{xp,xn,yp,yn,zp,zn}.png" defines the file names "cm_xp.png", "cm_xn.png", "cm_yp.png", 
		  "cm_yn.png", "cm_zp.png" and "cm_zn.png"
        - "cm_*.png" issues a glob command resulting in an alphanumerically ordered file name list. Be
		  aware that the naming of the previous example would result in a wrong ordering of the file names.
	    Mixing of '*' and "{,}" syntax is not allowed. The static method deduce_file_names is used to
		deduce the individual file names from the file_names parameter and can be used also when the correct
		path to the cubemap files needs to be found.

		Different levels of the mipmap pyramid can be created in separate calls. If
	    the level parameter is not specified or -1, the images are load to level 0 and
		a mipmap pyramid is built automatically. This method has no support for rescaling
		to power of two dimensions. If a mipmap is used, the image files should already have
		power of two dimensions. */  
	bool create_from_images(const context& ctx, const std::string& file_names, int level = -1);
	/// write the content of the texture to a file. This method needs support for frame buffer objects.
	bool write_to_file(context& ctx, const std::string& file_name, unsigned int z_or_cube_side = -1, float depth_map_gamma = 1.0f, const std::string& options = "") const;
	/** create storage for mipmaps without computing the mipmap contents */
	bool create_mipmaps(const context& ctx);
	/** generate mipmaps automatically, only supported if 
	    framebuffer objects are supported by the GPU */
	bool generate_mipmaps(const context& ctx);
	/** create texture from the currently set read buffer, where
	    x, y, width and height define the to be used rectangle of 
		 the read buffer. The dimension and resolution of the texture
		 format are updated automatically. If level is not specified 
		 or set to -1 mipmaps are generated. */
	bool create_from_buffer(const context& ctx, int x, int y, int width, int height, int level = -1);
	/** create texture from data view. Use dimension and resolution
	    of data view but the component format of the texture.
	    If level is not specified or set to -1 mipmaps are generated. 
		If cube_side is specified, and data view is 2D, create one of
		the six sides of a cubemap.
		If num_array_layers is not zero a texture array is created.
		Set num_array_layers to -1 to automatically choose the layer
		number based on the data view dimensions and size, e.g. a 2D/3D
		data view creates a 1D/2D array with layer count equal to height/depth.
		Set num_array_layers to > 0 to manually specify the layer count.
		This can be used to create a one layer 1D/2D texture array from a 1D/2D data view.
		Cubemap arrays are currently not suported. */
	bool create(const context& ctx, const cgv::data::const_data_view& data, int level = -1, int cube_side = -1, int num_array_layers = 0, const std::vector<cgv::data::data_view>* palettes = 0);
	/** replace a block within a 1d texture with the given data. 
	    If level is not specified, level 0 is set and if a mipmap 
		 has been created before, coarser levels are updated also. */
	bool replace(const context& ctx, int x, const cgv::data::const_data_view& data, int level = -1, const std::vector<cgv::data::data_view>* palettes = 0);
	/** replace a block within a 2d texture with the given data. 
	    If level is not specified, level 0 is set and if a mipmap 
		 has been created before, coarser levels are updated also. */
	bool replace(const context& ctx, int x, int y, const cgv::data::const_data_view& data, int level = -1, const std::vector<cgv::data::data_view>* palettes = 0);
	/** replace a block within a 3d texture or a side of a cube map
	    with the given data. 
	    If level is not specified, level 0 is set and if a mipmap 
		has been created before, coarser levels are updated also. */
	bool replace(const context& ctx, int x, int y, int z_or_cube_side, const cgv::data::const_data_view& data, int level = -1, const std::vector<cgv::data::data_view>* palettes = 0);
	/// replace a block within a 2d texture from the current read buffer.
	bool replace_from_buffer(const context& ctx, int x, int y, int x_buffer,
		int y_buffer, int width, int height, int level = -1);
	/// replace a block within a 3d texture or a side from a cube map from the current read buffer.
	bool replace_from_buffer(const context& ctx, int x, int y, int z_or_cube_side, int x_buffer,
			int y_buffer, int width, int height, int level);
	/// replace within a slice of a volume or a side of a cube map from the given image
	bool replace_from_image(const context& ctx, const std::string& file_name, int x, int y, int z_or_cube_side, int level);
	/** same as previous method but use the passed data format and data view to
	    store the content of the image. */
	bool replace_from_image(cgv::data::data_format& df, cgv::data::data_view& dv, const context& ctx,
		                    const std::string& file_name, int x, int y, int z_or_cube_side, int level);
	/// destruct the texture and free texture memory and handle
	bool destruct(const context& ctx);
	//@}

	/**@name methods that change the current gpu context */
	//@{
	/** enable this texture in the given texture unit, -1 corresponds to 
	    the current unit. */
	bool enable(const context& ctx, int tex_unit = -1);
	/// disable texture and restore state from before last enable call
	bool disable(const context& ctx);

	/// @brief Binds this texture as an image texture to the given texture unit.
	/// @param ctx The graphics context.
	/// @param tex_unit The texture unit the texture is bound to.
	/// @param level The mipmap level of the texture to bind.
	/// @param bind_array Only for array textures. If true, the complete texture array is bound and the layer ignored. If false, a single layer as specified by layer is bound.
	/// @param layer Only for array textures. The array layer to bind.
	/// @param access The access type: read, write, read and write.
	/// @return True if the texture was successfully bound as an image, false otherwise.
	bool bind_as_image(const context& ctx, int tex_unit, int level = 0, bool bind_array = false, int layer = 0, AccessType access = AT_WRITE_ONLY);
	//@}
};

	}
}

#include <cgv/config/lib_end.h>
