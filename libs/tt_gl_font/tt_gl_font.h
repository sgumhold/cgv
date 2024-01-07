#pragma once

#include <cgv/media/font/font.h>
#include <cgv/render/context.h>
#include <cgv/render/texture.h>
#include <cgv_gl/rectangle_renderer.h>
#include <vector>
#include <string>
#include <map>
#include "stb_truetype.h"

#include "lib_begin.h"

namespace cgv {

struct font_face_info
{
	std::string file_name;
	unsigned nr_glyphs;
	int fi;
	bool is_valid() const { return nr_glyphs > 0 && !file_name.empty(); }
	font_face_info() : nr_glyphs(0), fi(0) {}
	font_face_info(const std::string& _file_name, unsigned _nr_glyphs, int _fi) :
		file_name(_file_name), nr_glyphs(_nr_glyphs), fi(_fi) {}
};

/// this rectangle style is used by default drawing implementation in tt_gl_font_face
extern CGV_API cgv::render::rectangle_render_style& ref_rectangle_render_style();

/// <summary>
/// font face with support for text drawing and generation of textured quads
/// </summary>
class CGV_API tt_gl_font_face : public cgv::media::font::font_face
{
private:
	bool bitmap_out_of_date;
	bool tex_out_of_date;
protected:
	unsigned char* ttf_buffer;
	std::vector<unsigned char> internal_ttf_buffer;

	std::string font_name;
	stbtt_fontinfo f;

	float font_size;
	unsigned fst_char, nr_chars;
	std::vector<stbtt_bakedchar> baked_chars;

	unsigned bitmap_width, bitmap_height;
	std::vector<unsigned char> bitmap;
	bool build_mipmap;

	cgv::render::texture* tex_ptr;
	cgv::render::context* ctx_ptr;

	void bake_characters();
	void ensure_bitmap();
	void ensure_texture(cgv::render::context& ctx);
	bool read_font(const std::string& file_name, int fi = 0);
public:
	/// construct font face
	tt_gl_font_face(const std::string file_name, float _font_size, int fi = 0);
	///
	bool is_valid() const;
	/// construct font face
	tt_gl_font_face(const std::string name, const stbtt_fontinfo& _f, float _font_size, unsigned char* _ttf_buffer, int ffa);
	/// destruct font face
	~tt_gl_font_face();
	/// returns "tt_gl_font_face"
	std::string get_type_name() const;
	/// enumerate the supported sizes
	void enumerate_sizes(std::vector<int>& supported_sizes) const;
	/// return the width of a text printed in the given size, which is measured in pixels
	float measure_text_width(const std::string& text, float font_size) const;
	/// enables font face of given size and should be called once before calling draw_text functions
	void enable(void* context_ptr, float _font_size);
	/// draw text at given location with rectangle renderer initialized with default render style
	void draw_text(float& x, float& y, const std::string& text) const;
	unsigned get_nr_glyphs() const;
	std::string get_font_name() const;
	void set_character_range(int _fst_char, unsigned _nr_chars);
	void set_font_size(float _font_size);
	bool write_atlas(const std::string& file_name);
	cgv::render::texture& ref_texture(cgv::render::context& ctx) const;
	cgv::render::texture& ref_texture(cgv::render::context& ctx);
	unsigned text_to_quads(vec2& p, const std::string& text, std::vector<render::textured_rectangle>& Q, float scale = 1.0f, bool flip_y = false) const;
	unsigned text_to_quads(vec2& p, const std::string& text, std::vector<render::textured_rectangle>& Q, float scale = 1.0f, bool flip_y = false);
	box2 compute_box(const std::string& text, float scale = 1.0f, bool flip_y = false) const;
	vec2 align_text(const vec2& p, const std::string& text, cgv::render::TextAlignment ta, float scale = 1.0f, bool flip_y = false) const;
};

/// reference counted pointer to tt_gl_font_faces
typedef cgv::data::ref_ptr<tt_gl_font_face> tt_gl_font_face_ptr;

/// simple info structure to describe info needed to construct font
struct font_info
{
	font_face_info font_faces[4];
};

/// <summary>
/// implementation of font interface with minor extensions to set font size and character range
/// </summary>
class CGV_API tt_gl_font : public cgv::media::font::font
{
protected:
	std::string font_name;
	/// This array stores four variants of the font (i.e. normal, bold, italic, and
	std::array<tt_gl_font_face_ptr, 4> font_faces;
public:
	/// contruct font from name, size and infos necessary to create font faces, where the latter can be retrieved by name from ref_font_table() after fonts have been scanned
	tt_gl_font(const std::string& _font_name, float _font_size, const font_info& FI);
	/// returns "tt_gl_font"
	std::string get_type_name() const override;
	/// return the name of the font
	const char* get_name() const override;
	/// check whether the given font includes a face that include the possibly or-ed together selection of font face attributes
	bool supports_font_face(cgv::media::font::FontFaceAttributes ffa) const override;
	/// return a pointer to a font face or empty pointer if font face is not supported
	virtual cgv::media::font::font_face_ptr get_font_face(cgv::media::font::FontFaceAttributes ffa) const override;
	/// does nothing to communicate that any font size is supported
	void enumerate_sizes(std::vector<int>& supported_sizes) const override;
	/// set character range for which font face glyphs are rasterized into bitmaps
	void set_character_range(int _fst_char, unsigned _nr_chars);
	/// set font size in pixel at which font face glyphs are rasterized into bitmaps
	void set_font_size(float _font_size);
};
/// reference counted pointer to tt_gl_font
typedef cgv::data::ref_ptr<tt_gl_font> tt_gl_font_ptr;

/// return reference to font table
extern CGV_API std::map<std::string, font_info>& ref_font_table();
/// return whether fonts have been scanned
extern CGV_API bool fonts_scanned();
/// <summary>
/// scan usable fonts from given list of file names and store fonts in font table.
/// </summary>
/// <param name="file_names">reference to list of to be scanned font file names</param>
extern CGV_API void scan_fonts(const std::vector<std::string>& file_names);

/// <summary>
/// glob file names from font directory and scan them for usable fonts which are stored in font table.
/// If font directory is empty, use "c:/windows/fonts" on windows or otherwise "/usr/share/fonts"
/// </summary>
/// <param name="font_directory">font directory or empty to use system default font installation path</param>
extern CGV_API void scan_fonts(std::string font_directory = "");
/// return timestamp starting with 1 and incremented whenever font table is scanned
extern CGV_API size_t get_tt_gl_fonts_timestamp();
/// ensures that font table is built
void ensure_font_table();
/// looks for a specific font after having ensured that font table is built with ensure_font_table()
extern CGV_API tt_gl_font_ptr find_font(const std::string& font_name);
/// return reference to cached list of all scanned font names after having ensured that font table is built with ensure_font_table()
extern CGV_API const std::vector<std::string>& get_font_names();
/// return a string "enums='Arial,...'" with all scanned fonts that can be used to define a dropdown menu on an int variable i which provides font name via get_font_names()[i]
extern CGV_API const std::string& get_font_enum_declaration();
}

#include <cgv/config/lib_end.h>