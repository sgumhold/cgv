#pragma once

#include "convert_string.h"

#include "lib_begin.h"

namespace cgv {
	namespace utils {

/// convert a 8-bit string to a 16-bit string
extern CGV_API std::wstring str2wstr(const std::string& s);
/// convert a 16-bit string to a 8-bit string
extern CGV_API std::string wstr2str(const std::wstring& s);

	}
}

#include <cgv/config/lib_end.h>