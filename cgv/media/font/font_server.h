#pragma once

#include <cgv/base/base.h>
#include <cgv/base/register.h>
#include <cgv/media/font/font.h>
#include <cgv/media/font/lib_begin.h>

namespace cgv {
	namespace media {
		namespace font {

/// provides the methods to create and search for fonts
class CGV_API font_server : public cgv::base::base, public cgv::base::server
{
public:
	/// find an installed font by name
	virtual font_ptr find_font(const std::string& font_name) = 0;
	/// return potentially font driver and platform specific default font
	virtual font_ptr default_font(bool mono_space);
	/// enumerate the names of all installed fonts
	virtual void enumerate_font_names(std::vector<const char*>& font_names) = 0;
};

/// ref counted pointer to font server
typedef data::ref_ptr<font_server> font_server_ptr;

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<font_server>;
#endif


/// return the currently installed font server or 0 if no font server available
extern CGV_API font_server_ptr get_font_server();
/// install a font server, call this in the on_register method of the server implementation
extern CGV_API void register_font_server(font_server_ptr fs);

		}
	}
}

#include <cgv/config/lib_end.h>
