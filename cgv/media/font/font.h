#pragma once

#include <cgv/base/base.h>
#include <vector>
#include <string>

#include "lib_begin.h"

#ifdef WIN32
#pragma warning(disable:4275)
#pragma warning(disable:4231)
#endif

namespace cgv {
	namespace media {
		/// namespace for font support
		namespace font {

/// declaration of supported attributes of font faces
enum FontFaceAttributes
{
	FFA_REGULAR = 0,
	FFA_BOLD = 1,
	FFA_ITALIC = 2,
	FFA_BOLD_ITALIC = 3
};

/// interface class for different faces of a font, which can be constructed from a font
class CGV_API font_face : public base::base
{
protected:
	/// store the actual attributes of the font face
	int ffa;
public:
	/// construct from attributes that are declared in the enum FontFaceAttributes
	font_face(int _ffa);
	/// returns "font_face"
	std::string get_type_name() const;
	/// return the attributes
	virtual int get_attributes() const;
	/// enumerate the supported sizes; if nothing is enumerated any size is supported
	virtual void enumerate_sizes(std::vector<int>& supported_sizes) const = 0;
	/// return the width of a text printed in the given size, which is measured in pixels
	virtual float measure_text_width(const std::string& text, float font_size) const = 0;
	/// enables font face of given size and should be called once before calling draw_text functions
	virtual void enable(void* context_ptr, float _font_size) = 0;
	/// draw text at given location with font_face implementation dependent rendering api and advance location
	virtual void draw_text(float& x, float& y, const std::string& text) const = 0;
};

/// always use this ref counted pointer to store font faces
typedef data::ref_ptr<font_face> font_face_ptr;

/// interface class for fonts. Construct font with the find_font function
class CGV_API font : public base::base
{
public:
	/// returns "font"
	std::string get_type_name() const;
	/// return the name of the font
	virtual const char* get_name() const = 0;
	/// check whether the given font includes a face that include the possibly or-ed together selection of font face attributes
	virtual bool supports_font_face(cgv::media::font::FontFaceAttributes ffa) const = 0;
	/// return a pointer to a font face
	virtual font_face_ptr get_font_face(cgv::media::font::FontFaceAttributes ffa) const = 0;
	/// enumerate the supported font sizes
	virtual void enumerate_sizes(std::vector<int>& supported_sizes) const = 0;
};

/// always use this ref counted pointer to store fonts
typedef data::ref_ptr<font> font_ptr;

/// find an installed font by name (returns a null pointer if font could not be found)
extern CGV_API font_ptr find_font(const std::string& font_name);

/// find an installed font by name prefix (returns a null pointer if font could not be found)
extern CGV_API font_ptr find_font_by_prefix(const std::string& font_name_prefix);

/// find an installed font by name or return platform-specific default font if no font with that name exists
extern CGV_API font_ptr find_font_or_default(const std::string& font_name, bool default_font_mono_space = false);

/// return potentially font driver and platform-specific default font
extern CGV_API font_ptr default_font(bool mono_space = false);


/// enumerate the names of all installed fonts
extern CGV_API void enumerate_font_names(std::vector<const char*>& font_names);

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<font>;
CGV_TEMPLATE template class CGV_API data::ref_ptr<font_face>;
#endif

		}
	}
}

#include <cgv/config/lib_end.h>
