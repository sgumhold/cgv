#pragma once

#include <string>
#include <vector>
#include "lib_begin.h"

namespace cgv {
	namespace gui {
		/** ask the user for a file name to open a file. 
		    The filter string is composed of '|'-separated pairs of the form '<text>:<filter>', where 
			<text> is an arbitrary text (without '|' and ':' symbols) shown with the filter and <filter> 
			is one filter string or a ';'-separated list of filters. For example the filter parameter could
			be set to "Image Files (jpg,gif):*.jpg;*.gif|Video Files (avi):*.avi|All Files:*.*".
			The path parameter can be used to define an initial directory and or an initial file for the dialog:
			- if path is empty no initial directory not path are selected, 
			- if path defines a directory, it is used as initial directory without initial file, 
			- if path defines file without directory the initial file is set and the standard directory is used
			- if path defines a directory and file, both initial directory and file are set.
		*/
		extern CGV_API std::string file_open_dialog(const std::string& title, const std::string& filter, const std::string& path = "");
		/// ask user for an open dialog that can select multiple files, return common path prefix and fill field of filenames
		extern CGV_API std::string files_open_dialog(std::vector<std::string>& file_names, const std::string& title, const std::string& filter, const std::string& path = "");
		/** ask the user for a file name to save a file. The filter string uses the syntax defined in the docu of 
		    file_open_dialog(). */
		extern CGV_API std::string file_save_dialog(const std::string& title, const std::string& filter, const std::string& path = "");

	}
}

#include <cgv/config/lib_end.h>