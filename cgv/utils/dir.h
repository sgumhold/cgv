#pragma once

#include <string>

#include "lib_begin.h"

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
		}
	}
}

#include <cgv/config/lib_end.h>

