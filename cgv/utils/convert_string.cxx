#include "convert_string.h"

namespace cgv {
	namespace utils {

template <>
std::string to_string(const std::string& v, unsigned int w, unsigned int p)
{
	return v;
}

template <>
bool from_string(std::string& v, const std::string& s)
{
	v = s;
	return true;
}

	}
}
