#include <cgv/media/font/font_server.h>

namespace cgv {
	namespace media {
		namespace font {


font_server_ptr& ref_font_server()
{
	static font_server_ptr fs;
	return fs;
}

/// return the currently installed font server or 0 if no font server available
font_server_ptr get_font_server()
{
	return ref_font_server();
}

/// install a font server
void register_font_server(font_server_ptr fs)
{
	ref_font_server() = fs;
}

		}
	}
}
