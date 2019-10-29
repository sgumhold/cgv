#pragma once

/** \file options.h
 * Helper functions to access cgv options provided in the CGV_OPTIONS environment variable.
 */

#include <string>
#include <vector>

#include "lib_begin.h"

namespace cgv {
	namespace utils {
		/// check whether the system variable CGV_OPTIONS contains the given option (the comparison is case insensitve) within a semicolon separated list
		extern bool CGV_API has_option(const std::string& option);
		/// push back all options provided in the CGV_OPTIONS system variable
		extern void CGV_API enumerate_options(std::vector<std::string>& options);
	}
}

#include <cgv/config/lib_end.h>