#pragma once

#include <string>
#include "window.h"
#include "lib_begin.h"

namespace cgv {
	/// namespace that holds the abstract gui interface
	namespace gui {

/// the application class is only a container for static methods that give access to the windows of the application
class CGV_API application
{
public:
	/// create a window of the given %type, where all %gui implementations must support the %type "viewer"
	static window_ptr create_window(int w, int h, 
		const std::string& title, const std::string& window_type = "viewer");
	/// set the input focus to the given window
	static bool set_focus(const_window_ptr);
	/// return the number of created windows
	static unsigned int get_nr_windows();
	/// return the i-th created window
	static window_ptr get_window(unsigned int i);
	/// run the main loop of the %window system
	static bool run();
	/// quit the %application by closing all windows
	static void quit(int exit_code = 0);
	/// copy text to the clipboard
	static void copy_to_clipboard(const std::string& s);
	/// retreive text from clipboard
	static std::string paste_from_clipboard();
};

	}
}

#include <cgv/config/lib_end.h>