#pragma once

#include <string>
#include <cgv/data/data_view.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {
		class context;
		class shader_program;

		namespace gl {

// declare some colors by name
extern CGV_API float black[4], white[4], gray[4], green[4], brown[4], dark_red[4], cyan[4], yellow[4], red[4], blue[4];

/// map a type id to a gl enum
extern CGV_API unsigned map_to_gl(TypeId ti);

/// map a component format to a gl enum
extern CGV_API unsigned map_to_gl(cgv::data::ComponentFormat cf);

/// return one of the six cube map sides gl enums
extern CGV_API unsigned get_gl_cube_map_target(unsigned side);

/// generate mipmaps for the currently bound texture, which has the given texture dimension; optionally pass a string to get information on failure
extern CGV_API bool generate_mipmaps(unsigned int dim, std::string* last_error = 0);

/// map the given component format to the best matching available gl component format
extern CGV_API unsigned find_best_texture_format(const cgv::data::component_format& cf, cgv::data::component_format* best_cf = 0, const std::vector<cgv::data::data_view>* palettes = 0);

/// load data to a texture with the glTexImage commands and generate mipmaps if the level parameter is -1, return whether mipmaps were created
extern CGV_API bool load_texture(const cgv::data::const_data_view& data, unsigned gl_tex_format, unsigned level = -1, unsigned cube_side = -1, const std::vector<cgv::data::data_view>* palettes = 0);

/// create a texture from the given data view creating a mipmap pyramid
extern CGV_API unsigned int create_texture(const cgv::data::const_data_view& dv, bool mipmap = true, const std::vector<cgv::data::data_view>* palettes = 0, unsigned tex_id = -1);

/// create a certain texture level from the given data view and optionally specify a cube side of a cube map
extern CGV_API unsigned int create_texture(const cgv::data::const_data_view& dv, unsigned level, unsigned cube_side = -1, const std::vector<cgv::data::data_view>* palettes = 0, unsigned tex_id = -1);

//! replace part or complete data of currently bound texture with the data in the given data view
/*! Texture dimension is derived from the dimension of data view. The level gives the mipmap level in which to replace.
	A level of -1 corresponds to level 0 with recomputation of the mipmaps after replacement.
	x,y and z are offsets for 1D, 2D and 3D textures.
    In case of a cube map, the z parameter must be between 0 and 5 and defines the cube side in which to replace.
	Return value tells whether mipmaps have been recomputed
*/
extern CGV_API bool replace_texture(const cgv::data::const_data_view& data, int level = 0, int x = 0, int y = 0, int z = -1, const std::vector<cgv::data::data_view>* palettes = 0);

/** read the given image file into a texture and return the texture id or -1 in case of failure.
    The aspect ratio of the texture is written into the value pointed to by aspect_ptr. In case
	has_alpha_ptr is provided a boolean telling whether the texture contains alpha values is written
	to this field. */
extern CGV_API unsigned int read_image_to_texture(const std::string& file_name, bool mipmaps = true, double* aspect_ptr = 0, bool* has_alpha_ptr = 0);

/// read several images from one image file that can contain an animation
extern CGV_API bool read_image_to_textures(const std::string& file_name, std::vector<unsigned>& tex_ids, std::vector<float>& durations, bool mipmaps = true, double* aspect_ptr = 0, bool* has_alpha_ptr = 0);


/// cover the current viewport or a rectangle with it with a quad textured by a 1D texture
extern CGV_API void gl_1D_texture_to_screen(bool vary_along_x = true, float xmin = -1.0f, float ymin = -1.0f, float xmax = 1.0f, float ymax = 1.0f);

/// cover the current viewport or a rectangle with a textured quad, where the texture coverage can be adjusted with [u|v][min|max]
extern CGV_API void gl_texture_to_screen(float xmin = -1.0f, float ymin = -1.0f, float xmax = 1.0f, float ymax = 1.0f,
										 float umin =  0.0f, float vmin =  0.0f, float umax = 1.0f, float vmax = 1.0f);

/// set the program variables needed by the lighting.glsl shader
extern CGV_API void set_lighting_parameters(context& ctx, shader_program& prog);

/// return a reference to the singleton textured material shader program, which is constructed on demand only
extern CGV_API shader_program& ref_textured_material_prog(context& ctx);

/// push a shader program onto the textured material stack
extern CGV_API void push_textured_material_prog(shader_program& prog);

/// pop a shader program from the textured material stack
extern CGV_API void pop_textured_material_prog();




		}
	}
}

#include <cgv/config/lib_end.h>