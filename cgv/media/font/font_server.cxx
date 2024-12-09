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
	// list typical monospace fonts that should be present on most operating systems
	static std::vector<std::string> monospace_font_names = {
		"Consolas", // Windows default, but should also be available on Mac OS.
		"Courier", // Should be available on Mac OS. Will also match Courier New (Windows) and Courier 10 Pitch (Linux-based systems).
		"FreeMono", // For Linux-based systems.
		"DejaVu Sans Mono" // Alternative for Linux-based systems.
	};

	// list typical sans-serif fonts that should be present on most operating systems
	static std::vector<std::string> sans_serif_font_names = {
		"Open Sans", // Microsoft font.
		"Arial", // Available on Windows. Should also be available on MacOS.
		"Tahoma", // Available on Windows. Should also be available on MacOS.
		"DejaVu Sans", // Available on Linux-based systems.
		"Lucida", // Available on Mac OS. Will also match Lucida Grande.
		"Helvetica" // Available on Mac OS. Will also match Helvetica Neue.
	};

	font_ptr f;

	auto& default_font_names = mono_space ? monospace_font_names : sans_serif_font_names;
	for(const auto& font_name : default_font_names) {
		f = find_font_by_prefix(font_name);
		if(f)
			return f;
	}

	// select the first listed font if none of the default fonts could be found
	std::vector<const char*> font_names;
	enumerate_font_names(font_names);

	if(!font_names.empty())
		f = find_font(font_names.front());

	return f;
}

void register_font_server(font_server_ptr fs)
{
	ref_font_server() = fs;
}

		}
	}
}
