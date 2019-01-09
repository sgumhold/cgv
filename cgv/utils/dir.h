#pragma once

#include <string>

#include "lib_begin.h"
#include <vector>
#include <string>

namespace cgv {
	namespace utils {
		namespace dir {
			/// check if directory exists
			CGV_API bool exists(const std::string& dir_name);
			/// change current directory
			CGV_API bool chdir(const std::string& dir_name);
			/// create a new directory
			CGV_API bool mkdir(const std::string& dir_name);
			/// remove a directory
			CGV_API bool rmdir(const std::string& dir_name);
			/// get the current directory of the this process.
			CGV_API std::string current();
			/// collect within a given directory all file names relativ to given directory that match filter string; optionally collect files recursively and subdirectory names
			CGV_API bool glob(const std::string& dir_name, std::vector<std::string>& file_names, const std::string& filter = "*.*", bool recursive = false, bool short_file_names = false, std::vector<std::string>* subdir_names = 0);
		}
	}
}

#include <cgv/config/lib_end.h>

