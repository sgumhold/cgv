#pragma once
#include <string>

#include "lib_begin.h"

namespace cgv {
	namespace os {

		/// enumerate resources of an executable or dll and print to std::cout
		extern CGV_API void enum_resources(const std::string& prog_name);
	}
}
#include <cgv/config/lib_end.h>
