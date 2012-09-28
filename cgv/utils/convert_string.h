#pragma once

#include <string>
#include <sstream>
#include <iomanip>

#include "lib_begin.h"

namespace cgv {
	namespace utils {

template <typename T>
std::string to_string(const T& v, unsigned int w = -1, unsigned int p = -1)
{
	std::stringstream ss;
	if (w != (unsigned int)-1)
		ss << std::setw(w);
	if (p != (unsigned int)-1)
		ss << std::setprecision(p);
	ss << v;
	return ss.str();
}

template <> CGV_API std::string to_string(const std::string& v, unsigned int w, unsigned int p);

template <typename T>
bool from_string(T& v, const std::string& s)
{
	std::stringstream ss(s);
	ss >> v;
	return !ss.fail();
}

template <> CGV_API bool from_string(std::string& v, const std::string& s);

	}
}

#include <cgv/config/lib_end.h>
