#pragma once

#include <string>
#include <cgv/data/data_view.h>
#include <cgv/render/context.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {
		class context;
		class shader_program;
		class texture;
		enum TextureSampling;

		namespace gl {

// declare some colors by name
extern CGV_API float black[4], white[4], gray[4], green[4], brown[4], dark_red[4], cyan[4], yellow[4], red[4], blue[4];

/// map a type id to a gl enum
extern CGV_API unsigned map_to_gl(TypeId ti);

/// map a component format to a gl enum, return -1 of this was not possible
extern CGV_API unsigned map_to_gl(cgv::data::ComponentFormat cf, cgv::data::ComponentIntegerInterpretation cii = cgv::data::CII_DEFAULT);

/// return OpenGL material side constant
extern CGV_API unsigned map_to_gl(MaterialSide ms);

/// return one of the six cube map sides gl enums
extern CGV_API unsigned get_gl_cube_map_target(unsigned side);

/// generate mipmaps for the currently bound texture, which has the given texture dimension; optionally pass a string to get information on failure
extern CGV_API bool generate_mipmaps(unsigned int dim, bool is_array = false, std::string* last_error = 0);

/// map the given component format to the best matching available gl component format
extern CGV_API unsigned find_best_texture_format(const cgv::data::component_format& cf, cgv::data::component_format* best_cf = 0, const std::vector<cgv::data::data_view>* palettes = 0);

/// load data to a texture with the glTexImage commands and generate mipmaps if the level parameter is -1, return whether mipmaps were created
extern CGV_API bool load_texture(const cgv::data::const_data_view& data, unsigned gl_tex_format, unsigned level = -1, unsigned cube_side = -1, int num_array_layers = 0, const std::vector<cgv::data::data_view>* palettes = 0);

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

//! cover the current viewport with a textured quad using the textured default shader program or the one passed in the third parameter
/*! A shader program passed in the third parameter must have the vertex attributes
    in vec4 position;
	in vec2 texcoord
*/
extern CGV_API bool cover_screen(context& ctx, shader_program* prog_ptr = 0, bool flip_tex_v_coord = false, float xmin = -1.0f, float ymin = -1.0f, float xmax = 1.0f, float ymax = 1.0f,
	float umin = 0.0f, float vmin = 0.0f, float umax = 1.0f, float vmax = 1.0f);

DEPRECATED("deprecated, use cover_screen instead.") extern CGV_API void gl_texture_to_screen(float xmin = -1.0f, float ymin = -1.0f, float xmax = 1.0f, float ymax = 1.0f,
										 float umin =  0.0f, float vmin =  0.0f, float umax = 1.0f, float vmax = 1.0f);

/// set the program variables needed by the lighting.glsl shader
DEPRECATED("deprecated, use cgv::render::context based light management.") extern CGV_API void set_lighting_parameters(context& ctx, shader_program& prog);

/// return a reference to the singleton textured material shader program, which is constructed on demand only
DEPRECATED("deprecated, use cgv::render::context::ref_surface_shader_prog() instead.") extern CGV_API shader_program& ref_textured_material_prog(context& ctx);

/// push a shader program onto the textured material stack
DEPRECATED("deprecated, use automatic cgv::render::context based shader_program stack.") extern CGV_API void push_textured_material_prog(shader_program& prog);

/// pop a shader program from the textured material stack
DEPRECATED("deprecated, use automatic cgv::render::context based shader_program stack.") extern CGV_API void pop_textured_material_prog();

//! complete the given shader program that is assumed to have a working fragment shader.
/*! The function adds the rest of the pipeline and provides the input <in vec3 tex_coord>
    to the fragment shader. The output of the fragment shader is stored in the 3D texture
	passed to \c render_to_texture3D(). After this call the shader program is managed by
	the caller and can be used once or several times in the function \c render_to_texture3D().
	In case of failure, false is returned and in case the \c error_message string is provided 
	an error message is assigned to \c error_message. */
extern CGV_API bool complete_program_form_render_to_texture3D(cgv::render::context& ctx, cgv::render::shader_program& prog, std::string* error_message = 0);

//! Render to the given target 3D texture with the given shader program that must be completed with the function \c complete_program_form_render_to_texture3D().
/*! The program needs to be enabled before calling this function and all uniforms necessary to the fragment shader implementation of the caller should be 
    set. The fragment shader is called once per texel with input \c tex_coord of type \c vec3 and the result is stored in the given texture, which cannot be 
	enabled and used as input to the fragment shader. The \c texture_sampling parameter steers the computation of the \c tex_coord input. If additional textures
	are provided, they are attached as additional targets during render to texture. Then the fragment shader should access the different targets through the
	\c gl_FragData[] array. All target textures must have the same resolution. */
extern CGV_API bool render_to_texture3D(context& ctx, shader_program& prog, TextureSampling texture_sampling, texture& target_tex, texture* target_tex2 = 0, texture* target_tex3 = 0, texture* target_tex4 = 0);

/// different program interfaces
enum ProgramInterface {
	PI_UNIFORM,
	PI_UNIFORM_BLOCK,
	PI_PROGRAM_INPUT,
	PI_PROGRAM_OUTPUT,
	PI_VERTEX_SUBROUTINE, PI_TESS_CONTROL_SUBROUTINE, PI_TESS_EVALUATION_SUBROUTINE, PI_GEOMETRY_SUBROUTINE, PI_FRAGMENT_SUBROUTINE, PI_COMPUTE_SUBROUTINE,
	PI_VERTEX_SUBROUTINE_UNIFORM, PI_TESS_CONTROL_SUBROUTINE_UNIFORM, PI_TESS_EVALUATION_SUBROUTINE_UNIFORM, PI_GEOMETRY_SUBROUTINE_UNIFORM, PI_FRAGMENT_SUBROUTINE_UNIFORM, PI_COMPUTE_SUBROUTINE_UNIFORM,
	PI_TRANSFORM_FEEDBACK_VARYING,
	PI_BUFFER_VARIABLE,
	PI_SHADER_STORAGE_BLOCK
};
/// print program resources for given interface
extern CGV_API void print_program_ressources(shader_program& prog, const std::string& interface_name, ProgramInterface prog_intf);


		}
	}
}

#include <cgv/config/lib_end.h>
