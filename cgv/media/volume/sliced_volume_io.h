#pragma once

#include "sliced_volume.h"

#include "../lib_begin.h"

namespace cgv {
	namespace media {
		namespace volume {

			extern CGV_API bool read_from_sliced_volume(const std::string& file_name, volume& V);

			extern CGV_API bool write_as_sliced_volume(const std::string& file_name, const std::string& file_name_pattern, const volume& V);
		}
	}
}

#include <cgv/config/lib_end.h>