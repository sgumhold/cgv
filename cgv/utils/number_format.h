#pragma once

#include <string>

#include "lib_begin.h"

namespace cgv {
namespace utils {

/// @brief A class to store number formatting specification and allow converting numbers to strings.
class CGV_API number_format {
public:
	std::string convert(float value) const;

	void precision_from_range(float first, float last);

	unsigned precision = 0;
	bool decimal_integers = false;
	bool fixed = true;
	bool trailing_zeros = true;
	int group_size = 3;
	std::string group_separator = ",";
	bool grouping = false;

private:
	// Can only group integer part without sign
	std::string apply_grouping(const std::string& value) const;
};

} // namesapce utils
} // namesapce cgv

#include <cgv/config/lib_end.h>
