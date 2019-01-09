#include "gl_tools.h"

#include <cgv/media/image/image_reader.h>

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

using namespace cgv::data;
using namespace cgv::media::image;

namespace cgv {
	namespace render {
		namespace gl {

unsigned map_material_side(MaterialSide ms)
{
	static unsigned material_side_mapping[] = { GL_FALSE, GL_FRONT, GL_BACK, GL_FRONT_AND_BACK };
	return material_side_mapping[ms];
}


unsigned int read_image_to_texture(const std::string& file_name, bool mipmaps, double* aspect_ptr, bool* has_alpha_ptr)
{
	std::vector<data_format> palette_formats;
	std::vector<data_view> palettes;
	data_format df;
	data_view dv;
	image_reader ir(df, &palette_formats);
	if (!ir.read_image(file_name,dv,&palettes)) {
		std::cerr << ir.get_last_error().c_str() << std::endl;
		return -1;
	}
	if (aspect_ptr)
		*aspect_ptr = (double)df.get_width()/df.get_height();
	if (has_alpha_ptr) {
		if (palette_formats.size() > 0 && df.get_nr_components() == 1 && df.get_component_name(0) == "0")
			*has_alpha_ptr = palette_formats[0].get_component_index("A") != (unsigned int)-1;
		else
			*has_alpha_ptr = df.get_component_index("A") != (unsigned int)-1;
	}
	return create_texture(dv, mipmaps, &palettes);
}

bool read_image_to_textures(const std::string& file_name, std::vector<unsigned>& tex_ids, std::vector<float>& durations, bool mipmaps, double* aspect_ptr, bool* has_alpha_ptr)
{
	std::vector<data_format> palette_formats;
	std::vector<data_view> palettes;
	data_view dv;
	data_format df;
	image_reader ir(df, &palette_formats);
	// open image
	if (!ir.open(file_name)) {
		std::cerr << ir.get_last_error().c_str() << std::endl;
		return false;
	}
	palettes.resize(palette_formats.size());
	// query number of images
	unsigned nr = ir.get_nr_images();
	tex_ids.resize(nr);
	// create textures
	glGenTextures(nr,&tex_ids.front());
	// read images and create textures
	durations.clear();
	for (unsigned i=0; i<nr; ++i) {
		// query palettes
		for (unsigned j=0; j<palette_formats.size(); ++j) {
			if (!ir.read_palette(j, palettes[j]))
				return false;
		}
		durations.push_back(ir.get_image_duration());
		if (durations.back() == 0)
			durations.back() = 0.04f;
		if (!ir.read_image(dv)) {
			std::cerr << ir.get_last_error().c_str() << std::endl;
			return false;
		}
		create_texture(dv,mipmaps,&palettes,tex_ids[i]);
	}
	if (aspect_ptr)
		*aspect_ptr = (double)df.get_width()/df.get_height();
	if (has_alpha_ptr) {
		if (palette_formats.size() > 0 && df.get_nr_components() == 1 && df.get_component_name(0) == "0")
			*has_alpha_ptr = palette_formats[0].get_component_index("A") != (unsigned int)-1;
		else
			*has_alpha_ptr = df.get_component_index("A") != (unsigned int)-1;
	}
	return true;
}

/// set the program variables needed by the lighting.glsl shader
void set_lighting_parameters(context& ctx, shader_program& prog)
{
	GLint lv;
	glGetIntegerv(GL_LIGHT_MODEL_LOCAL_VIEWER, &lv);
	prog.set_uniform(ctx, "local_viewer", (bool)(lv != 0));
	unsigned n = ctx.get_max_nr_lights();
	cgv::math::vec<int> enabled(n); 
	for (unsigned i=0; i<n; ++i)
		glGetIntegerv(GL_LIGHT0+i, &enabled(i));
	prog.set_uniform_array(ctx, "lights_enabled", enabled);
}


std::vector<shader_program*>& ref_shader_prog_stack()
{
	static std::vector<shader_program*> sps;
	return sps;
}

/// push a shader program onto the textured material stack
void push_textured_material_prog(shader_program& prog)
{
	ref_shader_prog_stack().push_back(&prog);
}

/// pop a shader program from the textured material stack
void pop_textured_material_prog()
{
	ref_shader_prog_stack().pop_back();
}

shader_program& ref_textured_material_prog(context& ctx)
{
	static shader_program prog;
	if (!ref_shader_prog_stack().empty())
		return *ref_shader_prog_stack().back();

	if (!prog.is_created()) {
		if (!prog.build_program(ctx, "textured_material.glpr")) {
			std::cerr << "could not build textured material shader program" << std::endl;
			exit(0);
		}
		else {
			std::cout << "successfully built textured material shader program" << std::endl;
		}
	}
	return prog;
}


		}
	}
}