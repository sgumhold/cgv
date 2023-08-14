#pragma once

#include <cgv/media/font/font_server.h>

namespace fltk {
	struct Font;
}

using namespace cgv::media::font;

#include "lib_begin.h"

class fltk_font_face : public font_face
{
	fltk::Font* f;
public:
	/// construct from attributes
	fltk_font_face(fltk::Font* _f, int _ffa);
	/// virtual destructor for ref_ptr support
	~fltk_font_face();
	/// enumerate the supported sizes
	void enumerate_sizes(std::vector<int>& supported_sizes) const;
	/// return the width of a text printed in the given size, which is measured in pixels
	float measure_text_width(const std::string& text, float font_size) const;
	/// enables font face of given size and should be called once before calling draw_text functions
	void enable(void* context_ptr, float _font_size);
	/// draw text at given location with font_face implementation dependent rendering api and advance location
	void draw_text(float& x, float& y, const std::string& text) const;
	///
	fltk::Font* get_fltk_font() const;
};

class fltk_font : public font
{
	fltk::Font* f;
	int ffa;
public:
	/// 
	fltk_font(::fltk::Font* _f, int _ffa);
	/// virtual destructor for ref_ptr support
	~fltk_font();
	/// return the name of the font
	const char* get_name() const override;
	/// check whether the given font includes a face that include the possibly or-ed together selection of font face attributes
	bool supports_font_face(cgv::media::font::FontFaceAttributes _ffa) const override;
	/// enumerate the supported font sizes
	void enumerate_sizes(std::vector<int>& supported_sizes) const;
	/// return a pointer to a font face
	font_face_ptr get_font_face(cgv::media::font::FontFaceAttributes _ffa) const override;
};

/// always use this ref counted pointer to store font faces
typedef cgv::data::ref_ptr<fltk_font_face> fltk_font_face_ptr;
/// always use this ref counted pointer to store fonts
typedef cgv::data::ref_ptr<fltk_font> fltk_font_ptr;

/// implements a trigger server with fltk
class CGV_API fltk_font_server : public font_server
{
public:
	/// register server in frame work
	void on_register();
	/// return "fltk_font_server"
	std::string get_type_name() const;
	/// find an installed font by name
	font_ptr find_font(const std::string& font_name);
	/// enumerate the names of all installed fonts
	void enumerate_font_names(std::vector<const char*>& font_names);
};

#include <cgv/config/lib_end.h>