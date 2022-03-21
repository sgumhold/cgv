#pragma once
#include <string>
#include <iostream>

#include "lib_begin.h"

namespace cgv {
	namespace os {
		/// parse line from stream independent of how line breaks are encoded
		extern CGV_API std::istream& safe_getline(std::istream& is, std::string& line);
	}
}
#include <cgv/config/lib_end.h>
