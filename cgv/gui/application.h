#pragma once

#include <string>
#include "window.h"
#include "lib_begin.h"

namespace cgv {
	/// namespace that holds the abstract gui interface
	namespace gui {

/// monitor description
struct monitor_description
{
	// pixel coordinate of monitor
	int x, y;
	// pixel dimensions of monitor
	unsigned w, h;
	// pixel densities
	float dpi_x, dpi_y;
};

/// the application class is only a container for static methods that give access to the windows of the application
class CGV_API application
{
public:
	/// fill the passed vector with a list of all monitors descriptions; returns false if no gui driver is available
	static bool enumerate_monitors(std::vector<monitor_description>& monitor_descriptions);
	/// create a window of the given %type, where all %gui implementations must support the %type "viewer"
	static window_ptr create_window(int w, int h, 
		const std::string& title, const std::string& window_type = "viewer");
	/// remove a window from the application's list of windows
	static bool remove_window(window_ptr w);
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