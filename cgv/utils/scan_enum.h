#pragma once

#include <vector>
#include "token.h"

#include "lib_begin.h"

namespace cgv {
	namespace utils {

/// parse an enum declaration string into names and values
extern CGV_API void parse_enum_declarations(const std::string& enum_declarations, std::vector<token>& enum_names, std::vector<int>& enum_values);

/// convert value to index
extern CGV_API unsigned find_enum_index(int value, const std::vector<int>& enum_values);

/// convert name to index
extern CGV_API unsigned find_enum_index(const std::string& name, const std::vector<token>& enum_names);

	}
}

#include <cgv/config/lib_end.h>