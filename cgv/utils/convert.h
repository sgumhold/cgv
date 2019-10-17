#pragma once

#include "convert_string.h"

#include "lib_begin.h"

namespace cgv {
	namespace utils {

/// convert a 8-bit string to a 16-bit string
extern CGV_API std::wstring str2wstr(const std::string& s);
/// convert a 16-bit string to a 8-bit string
extern CGV_API std::string wstr2str(const std::wstring& s);
/// encode a string into base64
extern CGV_API std::string encode_base64(const std::string& s);
/// decode a base64 encoded string
extern CGV_API std::string decode_base64(const std::string& s);

	}
}

#include <cgv/config/lib_end.h>