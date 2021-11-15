#pragma once

#include <string>

#include "lib_begin.h"

namespace cgv {
	namespace os {

/// expand all files from zip archive \c file_path into folder \c destination_path
extern CGV_API bool expand_archive(const std::string& file_path, const std::string& destination_path);

/// compress all files specified by \c file_filter_path into zip archive \c archive_path
extern CGV_API bool compress_archive(const std::string& file_filter_path, const std::string& archive_path);


	}
}

#include <cgv/config/lib_end.h>
