#include "gl_tools.h"

#include <cgv_gl/gl/gl.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/math/ftransform.h>
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
float black[4] = { 0, 0, 0, 1 };
float white[4] = { 1, 1, 1, 1 };
float gray[4] = { 0.25f, 0.25f, 0.25f, 1 };
float green[4] = { 0, 1, 0, 1 };
float brown[4] = { 0.3f, 0.1f, 0, 1 };
float dark_red[4] = { 0.4f, 0, 0, 1 };
float cyan[4] = { 0, 1, 1, 1 };
float yellow[4] = { 1, 1, 0, 1 };
float red[4] = { 1, 0, 0, 1 };
float blue[4] = { 0, 0, 1, 1 };


unsigned map_to_gl(cgv::data::ComponentFormat cf, cgv::data::ComponentIntegerInterpretation cii)
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
		GL_RG,   /// color format with components R, G
		GL_RGB,   /// color format with components R, G and B
		GL_RGBA,  /// color format with components R, G, B and A
		GL_BGR,   /// color format with components B, G and R
		GL_BGRA,  /// color format with components B, G, R and A
		GL_DEPTH_COMPONENT,     /// depth component
		GL_STENCIL_INDEX,     /// stencil component
	};
	static unsigned cf_to_gl_integer[] = {
		0, /// undefinded format with no component
		GL_RED_INTEGER,		 /// red channel of color format
		GL_GREEN_INTEGER,		 /// green channel of color format
		GL_BLUE_INTEGER,		 /// blue channel of color format
		GL_ALPHA_INTEGER,		 /// alpha format
		GL_LUMINANCE_INTEGER_EXT,     /// color format with luminance component L
		GL_LUMINANCE_INTEGER_EXT,     /// color format with intensity component I
		GL_LUMINANCE_ALPHA_INTEGER_EXT,    /// color format with luminance and alpha components: L and A
		GL_LUMINANCE_ALPHA_INTEGER_EXT,    /// color format with intensity and alpha components: I and A
		GL_RG_INTEGER,   /// color format with components R, G
		GL_RGB_INTEGER,   /// color format with components R, G and B
		GL_RGBA_INTEGER,  /// color format with components R, G, B and A
		GL_BGR_INTEGER,   /// color format with components B, G and R
		GL_BGRA_INTEGER,  /// color format with components B, G, R and A
		GL_DEPTH_COMPONENT,     /// depth component
		GL_STENCIL_INDEX,     /// stencil component
	};
	if (cf == 0 || cf > cgv::data::CF_S)
		return -1;

	if (cii == CII_INTEGER)
		return cf_to_gl_integer[cf];
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

GLuint gl_tex_dim[] = { GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_CUBE_MAP_EXT };

bool generate_mipmaps(unsigned int dim, bool is_cubemap, bool is_array, std::string* last_error)
{
	if (dim == 0 || dim > 3) {
		if (last_error)
			*last_error = "wrong dimension of texture";
		return false;
	}
	if(is_array && (dim < 2 || dim > 3)) {
		if(last_error)
			*last_error = "wrong dimension for array texture";
		return false;
	}
	if(is_cubemap && dim != 2) {
		if(last_error)
			*last_error = "wrong dimension for cubemap texture";
		return false;
	}
	if (!(ensure_glew_initialized() && GLEW_EXT_framebuffer_object)) {
		if (last_error)
			*last_error = "automatic generation of mipmaps not supported";
		return false;
	}
	if(is_array)
		dim += 2;
	if(is_cubemap)
		dim = 6;
	glGenerateMipmap(gl_tex_dim[dim-1]);
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

static const GLenum gl_snorm_texture_format_ids[] =
{
	GL_RED_SNORM,
	GL_RG_SNORM,
	GL_RGB_SNORM,
	GL_RGBA_SNORM,
	GL_ALPHA_SNORM,
	GL_LUMINANCE_SNORM,
	GL_LUMINANCE_ALPHA_SNORM,
	GL_INTENSITY_SNORM,
	GL_R8_SNORM,
	GL_RG8_SNORM,
	GL_RGB8_SNORM,
	GL_RGBA8_SNORM,
	GL_ALPHA8_SNORM,
	GL_LUMINANCE8_SNORM,
	GL_LUMINANCE8_ALPHA8_SNORM,
	GL_INTENSITY8_SNORM,
	GL_R16_SNORM,
	GL_RG16_SNORM,
	GL_RGB16_SNORM,
	GL_RGBA16_SNORM,
	GL_ALPHA16_SNORM,
	GL_LUMINANCE16_SNORM,
	GL_LUMINANCE16_ALPHA16_SNORM,
	GL_INTENSITY16_SNORM
};

static const char* snorm_texture_formats[] = {
	"s[R]",
	"s[R,G]",
	"s[R,G,B]",
	"s[R,G,B,A]",
	"s[A]",
	"s[L]",
	"s[L,A]",
	"s[I]",
	"sint8[R]",
	"sint8[R,G]",
	"sint8[R,G,B]",
	"sint8[R,G,B,A]",
	"sint8[A]",
	"sint8[L]",
	"sint8[L,A]",
	"sint8[I]",
	"sint16[R]",
	"sint16[R,G]",
	"sint16[R,G,B]",
	"sint16[R,G,B,A]",
	"sint16[A]",
	"sint16[L]",
	"sint16[L,A]",
	"sint16[I]",
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
	"_uint32[R,G,B,A]",
	"_uint32[R,G,B]",
	"_uint32[A]",
	"_uint32[I]",
	"_uint32[L]",
	"_uint32[L,A]",

	"_uint16[R,G,B,A]",
	"_uint16[R,G,B]",
	"_uint16[A]",
	"_uint16[I]",
	"_uint16[L]",
	"_uint16[L,A]",

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

	GL_R8,
	GL_R16,

	GL_R8I,
	GL_R8UI,
	GL_R16I,
	GL_R16UI,
	GL_R32I,
	GL_R32UI,

	GL_RG8,
	GL_RG16,

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

	"uint8[R]",
	"uint16[R]",

	"_int8[R]",
	"_uint8[R]",
	"_int16[R]",
	"_uint16[R]",
	"_int32[R]",
	"_uint32[R]",

	"uint8[R,G]",
	"uint16[R,G]",

	"_int8[R,G]",
	"_uint8[R,G]",
	"_int16[R,G]",
	"_uint16[R,G]",
	"_int32[R,G]",
	"_uint32[R,G]",
	0
};


unsigned find_best_texture_format(const cgv::data::component_format& _cf, cgv::data::component_format* best_cf, const std::vector<data_view>* palettes)
{
	cgv::data::component_format cf = _cf;
	if (cf.get_nr_components() == 1 && (cf.get_component_name(0) == "L" || cf.get_component_name(0) == "I"))
		cf.set_component_names("R");
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
	if (true){//ensure_glew_initialized() && GLEW_EXT_texture_snorm) {
		i = find_best_match(cf, snorm_texture_formats, best_cf);
		if (i != -1) {
			if (best_cf)
				*best_cf = cgv::data::component_format(snorm_texture_formats[i]);
			gl_format = gl_snorm_texture_format_ids[i];
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
		src_fmt = map_to_gl(cf, data.get_format()->get_integer_interpretation());
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

bool load_texture(const cgv::data::const_data_view& data, unsigned gl_tex_format, unsigned level, unsigned cube_side, int num_array_layers, const std::vector<data_view>* palettes)
{
	unsigned nr_dim = data.get_format()->get_nr_dimensions();
	const unsigned char* data_ptr = data.get_ptr<unsigned char>();
	unsigned w = data.get_format()->get_width();
	bool cube_map = (nr_dim == 2) && (cube_side != -1);
	bool texture_array = (nr_dim > 0) && (nr_dim < 4) && !cube_map && num_array_layers != 0;

	if(!(ensure_glew_initialized() && GLEW_EXT_texture_array))
		texture_array = false;

	GLuint src_type, src_fmt;
	unsigned nr_comp = configure_src_format(data, src_type, src_fmt, palettes);

	bool gen_mipmap = level == -1;
	if (gen_mipmap)
		level = 0;
	switch(nr_dim) {
	case 1:
		if(texture_array) {
			glTexImage2D(GL_TEXTURE_1D_ARRAY, level, gl_tex_format, w, 1, 0, src_fmt, src_type, data_ptr);
		} else {
			glTexImage1D(GL_TEXTURE_1D, level, gl_tex_format, w, 0, src_fmt, src_type, data_ptr);
		}
		break;
	case 2:
		if(texture_array) {
			if(num_array_layers < 0) {
				glTexImage2D(GL_TEXTURE_1D_ARRAY, level, gl_tex_format, w, data.get_format()->get_height(), 0, src_fmt, src_type, data_ptr);
			} else {
				if(ensure_glew_initialized() && GLEW_EXT_texture3D) {
					glTexImage3D(GL_TEXTURE_2D_ARRAY, level, gl_tex_format, w, data.get_format()->get_height(), 1, 0, src_fmt, src_type, data_ptr);
				}
			}
		} else {
			glTexImage2D(cube_map ? get_gl_cube_map_target(cube_side) : GL_TEXTURE_2D, level,
				gl_tex_format, w, data.get_format()->get_height(), 0, src_fmt, src_type, data_ptr);
		}
		break;
	case 3:
		if(ensure_glew_initialized() && GLEW_EXT_texture3D) {
			if(texture_array) {
				int num_layers = data.get_format()->get_depth();
				if(num_array_layers > 0)
					num_layers = std::min(data.get_format()->get_depth(), (unsigned)num_array_layers);

				glTexImage3D(GL_TEXTURE_2D_ARRAY, level, gl_tex_format, w, data.get_format()->get_height(),
					num_layers, 0, src_fmt, src_type, data_ptr);
			} else {
				glTexImage3D(GL_TEXTURE_3D, level, gl_tex_format, w, data.get_format()->get_height(),
					data.get_format()->get_depth(), 0, src_fmt, src_type, data_ptr);
			}
		}
		break;
	}
	if(gen_mipmap) {
		gen_mipmap = generate_mipmaps(nr_dim, texture_array);
	}
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
		gen_mipmap = generate_mipmaps(nr_dim, false);
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

	if (load_texture(dv, gl_tex_format, level, cube_side, false, palettes))
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return tex_id;
}

/// cover the current viewport with a textured quad using the textured default shader program or the one passed in the third parameter
bool cover_screen(context& ctx, shader_program* prog_ptr, bool flip_tex_v_coord, float xmin, float ymin, float xmax, float ymax, float umin, float vmin, float umax, float vmax)
{
	shader_program& prog = prog_ptr ? *prog_ptr : ctx.ref_default_shader_program(true);
	bool was_enabled = prog.is_enabled();
	if (!was_enabled) {
		if (!prog.enable(ctx)) {
			return false;
		}
		else
			if (!prog_ptr)
				ctx.set_color(rgba(1, 1, 1, 1));
	}
	int pos_idx = prog.get_position_index();
	int tex_idx = prog.get_texcoord_index();
	if (pos_idx == -1) {
		ctx.error("cgv::render::gl::render_2d_texture_to_screen() passed program does not have position vertex attributes", &prog);
		if (!was_enabled)
			prog.disable(ctx);
		return false;
	}
	ctx.push_modelview_matrix();
	ctx.set_modelview_matrix(cgv::math::identity4<double>());
	ctx.push_projection_matrix();
	ctx.set_projection_matrix(cgv::math::identity4<double>());
	
	vec4 positions[4] = {
		vec4(xmin,ymin, 0, 1),
		vec4(xmax,ymin, 0, 1),
		vec4(xmin,ymax, 0, 1),
		vec4(xmax,ymax, 0, 1)
	};
	vec2 texcoords[8] = {
		vec2(umin, vmin),
		vec2(umax, vmin),
		vec2(umin, vmax),
		vec2(umax, vmax),
		vec2(umin, vmax),
		vec2(umax, vmax),
		vec2(umin, vmin),
		vec2(umax, vmin)
	};

	attribute_array_binding::set_global_attribute_array(ctx, pos_idx, positions, 4);
	attribute_array_binding::enable_global_array(ctx, pos_idx);
	if (tex_idx != -1) {
		attribute_array_binding::set_global_attribute_array(ctx, tex_idx, &texcoords[flip_tex_v_coord?4:0], 4);
		attribute_array_binding::enable_global_array(ctx, tex_idx);
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	attribute_array_binding::disable_global_array(ctx, pos_idx);
	if (tex_idx != -1)
		attribute_array_binding::disable_global_array(ctx, tex_idx);

	ctx.pop_projection_matrix();
	ctx.pop_modelview_matrix();

	if (!was_enabled)
		prog.disable(ctx);
	return true;
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
#version 330\n\
\n\
uniform float slice_coord;\n\
layout(location = 0) in vec3 position;\n\
layout(location = 3) in vec2 texcoord;\n\
out vec3 tex_coord;\n\
\n\
void main()\n\
{\n\
	tex_coord.xy = texcoord;\n\
	tex_coord.z = slice_coord;\n\
	gl_Position = vec4(position,1.0);\n\
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
	static float V[4 * 3] = {
		-1, -1, 0, +1, -1, 0,
		+1, +1, 0, -1, +1, 0
	};
	static int F[1 * 4] = {
		0, 1, 2, 3
	};
	float T[4 * 2] = {
		0, 0, 1, 0,
		1, 1, 0, 1
	};
	if (texture_sampling == TS_VERTEX) {
		T[0] = T[6] = float(-0.5 / tex_res[0]);
		T[2] = T[4] = float(1.0 + 0.5 / tex_res[0]);
		T[1] = T[3] = float(-0.5 / tex_res[1]);
		T[5] = T[7] = float(1.0 + 0.5 / tex_res[1]);
	}
	ctx.push_window_transformation_array();
	ctx.set_viewport(ivec4(0, 0, tex_res[0], tex_res[1]));
	int slice_coord_loc = prog.get_uniform_location(ctx, "slice_coord");
	// go through slices
	for (int i = 0; i < (int) tex_res[2]; i++) {

		if (slice_coord_loc != -1) {
			float slice_coord = (texture_sampling == TS_CELL) ? (i + 0.5f) / tex_res[2] : (float)i / (tex_res[2] - 1);
			prog.set_uniform(ctx, slice_coord_loc, slice_coord);
		}
		// attach textures to fbo
		fbo.attach(ctx, target_tex, i, 0, 0);
		if (target_tex2)
			fbo.attach(ctx, *target_tex2, i, 0, 1);
		if (target_tex3)
			fbo.attach(ctx, *target_tex3, i, 0, 2);
		if (target_tex4)
			fbo.attach(ctx, *target_tex4, i, 0, 3);
		fbo.enable(ctx, 0);
		ctx.draw_faces(V, 0, T, F, 0, F, 1, 4);
		fbo.disable(ctx);
	}
	ctx.pop_window_transformation_array();
	return true;
}


		}
	}
}