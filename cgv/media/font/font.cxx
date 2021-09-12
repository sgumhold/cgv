#include <cgv/media/font/font.h>
#include <cgv/media/font/font_server.h>

namespace cgv {
	namespace media {
		namespace font {

/// construct from attributes
font_face::font_face(int _ffa) : ffa(_ffa) 
{
}
/// returns "font_face"
std::string font_face::get_type_name() const
{
	return "font_face";
}

/// return the attributes
int font_face::get_attributes() const
{
	return ffa;
}


/// returns "font"
std::string font::get_type_name() const
{
	return "font";
}

/// find an installed font by name
font_ptr find_font(const std::string& font_name)
{
	if (get_font_server())
		return get_font_server()->find_font(font_name);
	return font_ptr();
}

/// return platform dependend default font
font_ptr default_font(bool mono_space)
{
	if (get_font_server())
		return get_font_server()->default_font(mono_space);
	return font_ptr();	
}

/// enumerate the names of all installed fonts
void enumerate_font_names(std::vector<const char*>& font_names)
{
	if (get_font_server())
		return get_font_server()->enumerate_font_names(font_names);
}

		}
	}
}