#include <cgv/media/font/font_server.h>

namespace cgv {
	namespace media {
		namespace font {


font_server_ptr& ref_font_server()
{
	static font_server_ptr fs;
	return fs;
}

font_server_ptr get_font_server()
{
	return ref_font_server();
}

font_ptr font_server::default_font(bool mono_space)
{
	font_ptr f;
	if (mono_space) {
		if (f = find_font("Consolas"))
			return f;
		if (f = find_font("FreeMono"))
			return f;
		if (f = find_font("Courier New"))
			return f;
		if (f = find_font("Courier"))
			return f;
	}
	else {
		if (f = find_font("Open Sans"))
			return f;
		if (f = find_font("Tahoma"))
			return f;
		if (f = find_font("Arial"))
			return f;
	}
	return f;
}

void register_font_server(font_server_ptr fs)
{
	ref_font_server() = fs;
}

		}
	}
}
