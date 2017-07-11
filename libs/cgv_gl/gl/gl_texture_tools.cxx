#include "gl_tools.h"

#include <cgv_gl/gl/gl.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/shader_program.h>
#include <iostream>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

using namespace cgv::data;

namespace cgv {
	namespace render {
		namespace gl {

// declare some colors by name
float black[4]     = { 0, 0, 0, 1 };
float white[4]     = { 1, 1, 1, 1 };
float gray[4]      = { 0.25f, 0.25f, 0.25f, 1 };
float green[4]     = { 0, 1, 0, 1 };
float brown[4]     = { 0.3f, 0.1f, 0, 1 };
float dark_red[4]  = { 0.4f, 0, 0, 1 };
float cyan[4]      = { 0, 1, 1, 1 };
float yellow[4]    = { 1, 1, 0, 1 };
float red[4]       = { 1, 0, 0, 1 };
float blue[4]      = { 0, 0, 1, 1 };


unsigned map_to_gl(cgv::data::ComponentFormat cf)
{
	static unsigned cf_to_gl[] = {
		0, /// undefinded format with no component
		GL_RED,		 /// red channel of color format
		GL_GREEN,		 /// green channel of color format
		GL_BLUE,		 /// blue channel of color format
		GL_ALPHA,		 /// alpha format
		GL_LUMINANCE,     /// color format with luminance component L
		GL_LUMINANCE,     /// color format with intensity component I
		GL_LUMINANCE_ALPHA,    /// color format with luminance and alpha components: L and A
		GL_LUMINANCE_ALPHA,    /// color format with intensity and alpha components: I and A
		GL_RGB,   /// color format with components R, G and B
		GL_RGBA,  /// color format with components R, G, B and A
		GL_BGR_EXT,   /// color format with components B, G and R
		GL_BGRA_EXT,  /// color format with components B, G, R and A
		GL_DEPTH_COMPONENT,     /// depth component
		GL_STENCIL_INDEX,     /// stencil component
	};
	if (cf == 0 || cf > cgv::data::CF_S) {
		std::cerr << "could not map component format " << cf << " to gl format" << std::endl;
		return GL_RGB;
	}
	return cf_to_gl[cf];
}

unsigned map_to_gl(TypeId ti)
{
	static unsigned ti_to_gl[] = {
		0, /// used for undefined type
		0, // bit
		0, /// void
		0, /// boolean
		GL_BYTE,  /// signed integer stored in 8 bits
		GL_SHORT, /// signed integer stored in 16 bits
		GL_INT, /// signed integer stored in 32 bits
		0, /// signed integer stored in 64 bits
		GL_UNSIGNED_BYTE, /// unsigned integer stored in 8 bits
		GL_UNSIGNED_SHORT, /// unsigned integer stored in 16 bits
		GL_UNSIGNED_INT, /// unsigned integer stored in 32 bits
		0, /// unsigned integer stored in 64 bits
		0,  /// floating point type stored in 16 bits
		GL_FLOAT, /// floating point type stored in 32 bits
		GL_DOUBLE, /// floating point type stored in 64 bits
		0, /// wchar
		0, /// string type
		0, /// wstring type
		0 /// always the index after the last type
	};
	if (ti_to_gl[ti] == 0) {
		std::cerr << "could not map component type " << ti << " to gl type" << std::endl;
		return GL_UNSIGNED_BYTE;
	}
	return ti_to_gl[ti];
}

unsigned get_gl_cube_map_target(unsigned side)
{
	GLint gl_cube_map_target[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT
	};
	return gl_cube_map_target[side];
}

GLuint gl_tex_dim[] = { GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP_EXT };

bool generate_mipmaps(unsigned int dim, std::string* last_error)
{
	if (dim == 0 || dim > 3) {
		if (last_error)
			*last_error = "wrong dimension of texture";
		return false;
	}
	if (!(ensure_glew_initialized() && GLEW_EXT_framebuffer_object)) {
		if (last_error)
			*last_error = "automatic generation of mipmaps not supported";
		return false;
	}
	glGenerateMipmapEXT(gl_tex_dim[dim-1]);
	return true;
}

static const GLenum gl_std_texture_format_ids[] = 
{
	GL_ALPHA, 
	GL_ALPHA4, 
	GL_ALPHA8, 
	GL_ALPHA12, 
	GL_ALPHA16, 

	GL_LUMINANCE, 
	GL_LUMINANCE4, 
	GL_LUMINANCE8, 
	GL_LUMINANCE12, 
	GL_LUMINANCE16,

	GL_LUMINANCE_ALPHA, 
	GL_LUMINANCE4_ALPHA4, 
	GL_LUMINANCE6_ALPHA2, 
	GL_LUMINANCE8_ALPHA8, 
	GL_LUMINANCE12_ALPHA4, 
	GL_LUMINANCE12_ALPHA12,
	GL_LUMINANCE16_ALPHA16, 

	GL_INTENSITY, 
	GL_INTENSITY4, 
	GL_INTENSITY8, 
	GL_INTENSITY12, 
	GL_INTENSITY16, 

	GL_R3_G3_B2, 
	GL_RGB, 
	GL_RGB4, 
	GL_RGB5, 
	GL_RGB8, 
	GL_RGB10, 
	GL_RGB12, 
	GL_RGB16, 

	GL_RGBA, 
	GL_RGBA2, 
	GL_RGBA4, 
	GL_RGB5_A1, 
	GL_RGBA8, 
	GL_RGB10_A2, 
	GL_RGBA12, 
	GL_RGBA16
};

static const char* std_texture_formats[] = {
	"[A]",
	"uint8:4[A]",
	"uint8[A]",
	"uint16:12[A]",
	"uint16[A]",

	"[L]",
	"uint8:4[L]",
	"uint8[L]",
	"uint16:12[L]",
	"uint16[L]",

	"[L,A]",
	"uint8:4[L,A]",
	"uint8[L:6,A:2]",
	"uint8[L,8]",
	"uint16[L:12,A:4]",
	"uint16:12[L,A]",
	"uint16[L]",

	"[I]",
	"uint8:4[I]",
	"uint8[I]",
	"uint16:12[I]",
	"uint16[I]",

	"uint8[R:3,G:3,B:2]",
	"[R,G,B]",
	"uint8:4|16[R,G,B]",
	"uint8:5|16[R,G,B]",
	"uint8[R,G,B]",
	"uint16:10[R,G,B]",
	"uint16:12[R,G,B]",
	"uint16[R,G,B]",

	"[R,G,B,A]",
	"uint8:2[R,G,B,A]",
	"uint8:4[R,G,B,A]",
	"uint8[R:5,G:5,B:5,A:1]",
	"uint8[R,G,B,A]",
	"uint16[R:10,G:10,B:10,A:2]",
	"uint16:12[R,G,B,A]",
	"uint16[R,G,B,A]",
	0
};
static const GLenum gl_float_texture_format_ids[] = 
{
	GL_RGBA32F_ARB,
	GL_RGB32F_ARB,
	GL_ALPHA32F_ARB,
	GL_INTENSITY32F_ARB,
	GL_LUMINANCE32F_ARB,
	GL_LUMINANCE_ALPHA32F_ARB,

	GL_RGBA16F_ARB,
	GL_RGB16F_ARB,
	GL_ALPHA16F_ARB,
	GL_INTENSITY16F_ARB,
	GL_LUMINANCE16F_ARB,
	GL_LUMINANCE_ALPHA16F_ARB,
};


static const char* float_texture_formats[] = {
	"flt32[R,G,B,A]",
	"flt32[R,G,B]",
	"flt32[A]",
	"flt32[I]",
	"flt32[L]",
	"flt32[L,A]",

	"flt16[R,G,B,A]",
	"flt16[R,G,B]",
	"flt16[A]",
	"flt16[I]",
	"flt16[L]",
	"flt16[L,A]",
	0
};

static const GLenum gl_int_texture_format_ids[] = 
{
	GL_RGBA32UI_EXT,
	GL_RGB32UI_EXT,
	GL_ALPHA32UI_EXT,
	GL_INTENSITY32UI_EXT,
	GL_LUMINANCE32UI_EXT,
	GL_LUMINANCE_ALPHA32UI_EXT,

	GL_RGBA16UI_EXT,
	GL_RGB16UI_EXT,
	GL_ALPHA16UI_EXT,
	GL_INTENSITY16UI_EXT,
	GL_LUMINANCE16UI_EXT,
	GL_LUMINANCE_ALPHA16UI_EXT,
	
	GL_RGBA8UI_EXT,
	GL_RGB8UI_EXT,
	GL_ALPHA8UI_EXT,
	GL_INTENSITY8UI_EXT,
	GL_LUMINANCE8UI_EXT,
	GL_LUMINANCE_ALPHA8UI_EXT,
	
	GL_RGBA32I_EXT,
	GL_RGB32I_EXT,
	GL_ALPHA32I_EXT,
	GL_INTENSITY32I_EXT,
	GL_LUMINANCE32I_EXT,
	GL_LUMINANCE_ALPHA32I_EXT,
	
	GL_RGBA16I_EXT,
	GL_RGB16I_EXT,
	GL_ALPHA16I_EXT,
	GL_INTENSITY16I_EXT,
	GL_LUMINANCE16I_EXT,
	GL_LUMINANCE_ALPHA16I_EXT,
	
	GL_RGBA8I_EXT,
	GL_RGB8I_EXT,
	GL_ALPHA8I_EXT,
	GL_INTENSITY8I_EXT,
	GL_LUMINANCE8I_EXT,
	GL_LUMINANCE_ALPHA8I_EXT
};

static const char* int_texture_formats[] = {
	"uint32[R,G,B,A]",
	"uint32[R,G,B]",
	"uint32[A]",
	"uint32[I]",
	"uint32[L]",
	"uint32[L,A]",

	"uint16[R,G,B,A]",
	"uint16[R,G,B]",
	"uint16[A]",
	"uint16[I]",
	"uint16[L]",
	"uint16[L,A]",

	"uint8[R,G,B,A]",
	"uint8[R,G,B]",
	"uint8[A]",
	"uint8[I]",
	"uint8[L]",
	"uint8[L,A]",

	"int32[R,G,B,A]",
	"int32[R,G,B]",
	"int32[A]",
	"int32[I]",
	"int32[L]",
	"int32[L,A]",

	"int16[R,G,B,A]",
	"int16[R,G,B]",
	"int16[A]",
	"int16[I]",
	"int16[L]",
	"int16[L,A]",

	"int8[R,G,B,A]",
	"int8[R,G,B]",
	"int8[A]",
	"int8[I]",
	"int8[L]",
	"int8[L,A]",
	0
};

static const GLenum gl_depth_format_ids[] = 
{
	GL_DEPTH_COMPONENT,
	GL_DEPTH_COMPONENT16_ARB,
	GL_DEPTH_COMPONENT24_ARB,
	GL_DEPTH_COMPONENT32_ARB
};

static const char* depth_formats[] = 
{
	"[D]",
	"uint16[D]",
	"uint32[D:24]",
	"uint32[D]",
	0
};

static const GLenum gl_rg_texture_format_ids[] =
{
	GL_RED,
	GL_RG,

	GL_R16F,    
	GL_R32F,   

	GL_RG16F,   
	GL_RG32F,  

	GL_R8I,   
	GL_R8UI,    
	GL_R16I,    
	GL_R16UI,   
	GL_R32I,  
	GL_R32UI,   

	GL_RG8I, 
	GL_RG8UI,   
	GL_RG16I,   
	GL_RG16UI,  
	GL_RG32I, 
	GL_RG32UI
};


static const char* rg_texture_formats[] = {
	"[R]",
	"[R,G]",

	"flt16[R]",
	"flt32[R]",

	"flt16[R,G]",
	"flt32[R,G]",

	"int8[R]",
	"uint8[R]",
	"int16[R]",
	"uint16[R]",
	"int32[R]",
	"uint32[R]",

	"int8[R,G]",
	"uint8[R,G]",
	"int16[R,G]",
	"uint16[R,G]",
	"int32[R,G]",
	"uint32[R,G]",
	0
};


unsigned find_best_texture_format(const cgv::data::component_format& _cf, cgv::data::component_format* best_cf, const std::vector<data_view>* palettes)
{
	cgv::data::component_format cf = _cf;
	cgv::data::component_format best_cf_;
	if (!best_cf)
		best_cf = &best_cf_;
	if (palettes && palettes->size() > 0 && cf.get_nr_components() == 1 && cf.get_component_name(0) == "0")
		cf = *palettes->at(0).get_format();
	unsigned int i = find_best_match(cf, std_texture_formats);
	if (best_cf)
		*best_cf = cgv::data::component_format(std_texture_formats[i]);
	unsigned gl_format = gl_std_texture_format_ids[i];

	if (ensure_glew_initialized() && GLEW_ARB_depth_texture) {
		i = find_best_match(cf,depth_formats, best_cf);
		if (i != -1) {
			if (best_cf)
				*best_cf = cgv::data::component_format(depth_formats[i]);
			gl_format = gl_depth_format_ids[i];
		}
	}
	if (ensure_glew_initialized() && GLEW_EXT_texture_integer) {
		i = find_best_match(cf,int_texture_formats, best_cf);
		if (i != -1) {
			if (best_cf)
				*best_cf = cgv::data::component_format(int_texture_formats[i]);
			gl_format = gl_int_texture_format_ids[i];
		}
	}
	if (ensure_glew_initialized() && GLEW_ARB_texture_float) {
		i = find_best_match(cf, float_texture_formats, best_cf);
		if (i != -1) {
			if (best_cf)
				*best_cf = cgv::data::component_format(float_texture_formats[i]);
			gl_format = gl_float_texture_format_ids[i];
		}
	}
	if (ensure_glew_initialized() && GLEW_ARB_texture_rg) {
		i = find_best_match(cf, rg_texture_formats, best_cf);
		if (i != -1) {
			if (best_cf)
				*best_cf = cgv::data::component_format(rg_texture_formats[i]);
			gl_format = gl_rg_texture_format_ids[i];
		}
	}
	return gl_format;
}

// return nr components
unsigned configure_src_format(const cgv::data::const_data_view& data, GLuint& src_type, GLuint& src_fmt, const std::vector<data_view>* palettes)
{
	unsigned nr_comp = data.get_format()->get_nr_components();
	// configure pixel transfer
	// glPixelStorei(GL_PACK_SWAP_BYTES, swap_bytes ? GL_TRUE : GL_FALSE);
	// glPixelStorei(GL_PACK_LSB_FIRST, lsb_first ? GL_TRUE : GL_FALSE); 
	// glPixelStorei(GL_PACK_ROW_LENGTH, (GLint) row_length);
	glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) data.get_format()->get_alignment(0));
	src_type = map_to_gl(data.get_format()->get_component_type());
	ComponentFormat cf = data.get_format()->get_standard_component_format();
	if (cf != CF_UNDEF)
		src_fmt = map_to_gl(cf);
	else {
		if (palettes && palettes->size() > 0 && nr_comp == 1 && data.get_format()->get_component_name(0) == "0") {
			src_fmt = GL_COLOR_INDEX;
			const data_view& pv(palettes->at(0));
			const data_format& pf(*pv.get_format());
			nr_comp = pf.get_nr_components();
			unsigned comp_size = pf.get_entry_size() / pf.get_nr_components();
			std::vector<float> tmp;
			tmp.resize(pf.get_width());
			for (unsigned ci=0; ci<nr_comp; ++ci) {
				glPixelTransferf(GL_RED_BIAS, 0.000001f);
				for (unsigned i=0; i<pf.get_width(); ++i)
					tmp[i] = pf.get<unsigned char>(ci, pv.step_i(pv.get_ptr<unsigned char>(), i))/255.0f;
				switch (pf.get_component_name(ci)[0]) {
				case 'R' : glPixelMapfv(GL_PIXEL_MAP_I_TO_R, pf.get_width(), &tmp.front()); break;
				case 'G' : glPixelMapfv(GL_PIXEL_MAP_I_TO_G, pf.get_width(), &tmp.front()); break;
				case 'B' : glPixelMapfv(GL_PIXEL_MAP_I_TO_B, pf.get_width(), &tmp.front()); break;
				case 'A' : glPixelMapfv(GL_PIXEL_MAP_I_TO_A, pf.get_width(), &tmp.front()); break;
				case 'L' : 
					glPixelMapfv(GL_PIXEL_MAP_I_TO_R, pf.get_width(), &tmp.front()); 
					glPixelMapfv(GL_PIXEL_MAP_I_TO_G, pf.get_width(), &tmp.front()); 
					glPixelMapfv(GL_PIXEL_MAP_I_TO_B, pf.get_width(), &tmp.front()); 
					break;
				}
			}				
		}
	}
	return nr_comp;
}

bool load_texture(const cgv::data::const_data_view& data, unsigned gl_tex_format, unsigned level, unsigned cube_side, const std::vector<data_view>* palettes)
{
	unsigned nr_dim = data.get_format()->get_nr_dimensions();
	const unsigned char* data_ptr = data.get_ptr<unsigned char>();
	unsigned w = data.get_format()->get_width();
	bool cube_map = (nr_dim == 2) && (cube_side != -1);

	GLuint src_type, src_fmt;
	unsigned nr_comp = configure_src_format(data, src_type, src_fmt, palettes);

	bool gen_mipmap = level == -1;
	if (gen_mipmap)
		level = 0;
	switch (nr_dim) {
	case 1 : glTexImage1D(GL_TEXTURE_1D, level, gl_tex_format, w, 0, src_fmt, src_type, data_ptr); break;
	case 2 : glTexImage2D(cube_map ? get_gl_cube_map_target(cube_side) : GL_TEXTURE_2D, level, 
						  gl_tex_format, w, data.get_format()->get_height(), 0, src_fmt, src_type, data_ptr); break;
	case 3 : if (ensure_glew_initialized() && GLEW_EXT_texture3D)
				glTexImage3D(GL_TEXTURE_3D, level, gl_tex_format, w, data.get_format()->get_height(), 
							 data.get_format()->get_depth(), 0, src_fmt, src_type, data_ptr);
			 break;
	}
	if (gen_mipmap) 
		gen_mipmap = generate_mipmaps(nr_dim);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	return gen_mipmap;
}

bool replace_texture(const cgv::data::const_data_view& data, int level, int x, int y, int z, const std::vector<cgv::data::data_view>* palettes)
{
	unsigned nr_dim = data.get_format()->get_nr_dimensions();
	const unsigned char* data_ptr = data.get_ptr<unsigned char>();
	unsigned w = data.get_format()->get_width();
	bool cube_map = (nr_dim == 2) && (z != -1);

	GLuint src_type, src_fmt;
	unsigned nr_comp = configure_src_format(data, src_type, src_fmt, palettes);

	bool gen_mipmap = level == -1;
	if (gen_mipmap)
		level = 0;
	switch (nr_dim) {
	case 1 :
		glTexSubImage1D(GL_TEXTURE_1D, level, x, w, src_fmt, src_type, data_ptr);
		break;
	case 2 :
		if (cube_map) 
			glTexSubImage2D(get_gl_cube_map_target(z), level, x, y, w, 
					data.get_format()->get_height(), src_fmt, src_type, data_ptr);
		else
			glTexSubImage2D(GL_TEXTURE_2D, level, x, y, w, 
					data.get_format()->get_height(), src_fmt, src_type, data_ptr);
		break;
	case 3 :
		glTexSubImage3D(GL_TEXTURE_3D, level, x, y, z, w, 
				data.get_format()->get_height(), data.get_format()->get_depth(), src_fmt, src_type, data_ptr);
		break;
	}
	if (gen_mipmap) 
		gen_mipmap = generate_mipmaps(nr_dim);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	return gen_mipmap;
}

unsigned int create_texture(const cgv::data::const_data_view& dv, bool mipmap, const std::vector<data_view>* palettes, unsigned tex_id)
{
	return create_texture(dv,mipmap?(unsigned)-1:0, (unsigned)-1, palettes, tex_id);
}

unsigned int create_texture(const cgv::data::const_data_view& dv, unsigned level, unsigned cube_side, const std::vector<cgv::data::data_view>* palettes, unsigned tex_id)
{
	if (tex_id == -1)
		glGenTextures(1,&tex_id);
	unsigned nr_dim = dv.get_format()->get_nr_dimensions();
	if ((nr_dim == 2) && (cube_side != -1))
		glBindTexture(gl_tex_dim[3], tex_id);
	else
		glBindTexture(gl_tex_dim[nr_dim-1], tex_id);

	unsigned gl_tex_format = find_best_texture_format(*dv.get_format(), 0, palettes);

	if (load_texture(dv, gl_tex_format, level, cube_side, palettes))
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return tex_id;
}

/// cover the current viewport with a textured quad
void gl_texture_to_screen(float xmin, float ymin, float xmax, float ymax, float umin, float vmin, float umax, float vmax)
{
	GLint mm;
	glGetIntegerv(GL_MATRIX_MODE, &mm);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		glBegin(GL_QUADS);
			glTexCoord2f(umin,vmin);
			glVertex2f(xmin, ymin);

			glTexCoord2f(umax,vmin);
			glVertex2f(xmax, ymin);

			glTexCoord2f(umax,vmax);
			glVertex2f(xmax, ymax);

			glTexCoord2f(umin,vmax);
			glVertex2f(xmin, ymax);
		glEnd();
		
		glPopMatrix();
		
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	
	glMatrixMode(mm);
}

void gl_1D_texture_to_screen(bool vary_along_x, float xmin, float ymin, float xmax, float ymax)
{
	GLint mm;
	glGetIntegerv(GL_MATRIX_MODE, &mm);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		glBegin(GL_QUADS);
			glTexCoord1f(0);
			glVertex2f(xmin, ymin);

			glTexCoord1f(vary_along_x ? 1.0f : 0.0f);
			glVertex2f(xmax, ymin);

			glTexCoord1f(1);
			glVertex2f(xmax, ymax);

			glTexCoord1f(vary_along_x ? 0.0f : 1.0f);
			glVertex2f(xmin, ymax);
		glEnd();
		
		glPopMatrix();
		
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	
	glMatrixMode(mm);
}

bool complete_program_form_render_to_texture3D(cgv::render::context& ctx, cgv::render::shader_program& prog, std::string* error_message)
{
	const char* vertex_shader_source = "\
#version 150 compatibility\n\
\n\
uniform float slice_coord;\n\
\n\
out vec3 tex_coord;\n\
\n\
void main()\n\
{\n\
	tex_coord.xy = gl_MultiTexCoord0.xy;\n\
	tex_coord.z = slice_coord;\n\
	gl_Position = gl_Vertex;\n\
}";

	if (!prog.attach_code(ctx, vertex_shader_source, cgv::render::ST_VERTEX)) {
		if (error_message)
			*error_message = "could not attach vertex shader source";
		return false;
	}
	if (!prog.link(ctx)) {
		if (error_message)
			*error_message = "could not link render to texture 3D program";
		return false;
	}

	return true;
}


bool render_to_texture3D(context& ctx, shader_program& prog, TextureSampling texture_sampling, texture& target_tex, texture* target_tex2, texture* target_tex3, texture* target_tex4)
{
	// extract texture resolution
	unsigned tex_res[3] = { target_tex.get_width(), target_tex.get_height(), target_tex.get_depth() };

	// check consistency of all texture resolutions
	if (target_tex2) {
		if (target_tex2->get_width() != tex_res[0] || target_tex2->get_height() != tex_res[1] || target_tex2->get_depth() != tex_res[2]) {
			std::cerr << "ERROR in cgv:render::gl::render_to_texture3D: texture resolution of target_tex2 does not match resolution of target_tex" << std::endl;
			return false;
		}
	}
	if (target_tex3) {
		if (target_tex3->get_width() != tex_res[0] || target_tex3->get_height() != tex_res[1] || target_tex3->get_depth() != tex_res[2]) {
			std::cerr << "ERROR in cgv:render::gl::render_to_texture3D: texture resolution of target_tex3 does not match resolution of target_tex" << std::endl;
			return false;
		}
	}
	if (target_tex4) {
		if (target_tex4->get_width() != tex_res[0] || target_tex4->get_height() != tex_res[1] || target_tex4->get_depth() != tex_res[2]) {
			std::cerr << "ERROR in cgv:render::gl::render_to_texture3D: texture resolution of target_tex4 does not match resolution of target_tex" << std::endl;
			return false;
		}
	}
	// create fbo with resolution of slices
	cgv::render::frame_buffer fbo;
	fbo.create(ctx, tex_res[0], tex_res[1]);

	fbo.attach(ctx, target_tex, 0, 0, 0);
	if (!fbo.is_complete(ctx)) {
		std::cerr << "fbo to update volume gradient not complete" << std::endl;
		return false;
	}

	static double V[4 * 3] = {
		-1, -1, 0, +1, -1, 0,
		+1, +1, 0, -1, +1, 0
	};
	static int F[1 * 4] = {
		0, 1, 2, 3
	};
	double T[4 * 2] = {
		0, 0, 1, 0,
		1, 1, 0, 1
	};
	if (texture_sampling == TS_VERTEX) {
		T[0] = T[6] = -0.5 / tex_res[0];
		T[2] = T[4] = 1.0 + 0.5 / tex_res[0];
		T[1] = T[3] = -0.5 / tex_res[1];
		T[5] = T[7] = 1.0 + 0.5 / tex_res[1];
	}

	// store transformation matrices and reset them to identity
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// go through slices
	for (int i = 0; i < (int) tex_res[2]; i++) {
		// attach textures to fbo
		fbo.attach(ctx, target_tex, i, 0, 0);
		if (target_tex2)
			fbo.attach(ctx, *target_tex2, i, 0, 1);
		if (target_tex3)
			fbo.attach(ctx, *target_tex3, i, 0, 2);
		if (target_tex4)
			fbo.attach(ctx, *target_tex4, i, 0, 3);
		fbo.enable(ctx, 0);
		
		// draw square
		if (texture_sampling == TS_CELL)
			prog.set_uniform(ctx, "slice_coord", (i + 0.5f) / tex_res[2]);
		else
			prog.set_uniform(ctx, "slice_coord", (float)i / (tex_res[2]-1));

		ctx.draw_faces(V, 0, T, F, 0, F, 1, 4);

		fbo.disable(ctx);
	}

	// restore transformation matrices
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	return true;
}


		}
	}
}