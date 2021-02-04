#pragma once

/** \file cgv/utils/convert_string.h
 * Helper functions to convert numeric types into strings using std streams.
 */

#include <string>
#include <sstream>
#include <iomanip>

#include "lib_begin.h"

namespace cgv {
	namespace utils {

/// convert a numeric type \c T into string of width \c w and precision \c p
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

/// convert an integer type \c T into string of width \c w with given fill character
template <typename T>
std::string to_string(const T& v, unsigned int w, char fill_char)
{
	std::stringstream ss;
	ss << std::setw(w) << std::setfill(fill_char) << v;
	return ss.str();
}

/// specialization of conversion from string to strings
template <> CGV_API std::string to_string(const std::string& v, unsigned int w, unsigned int p);

/// extract value from string
template <typename T>
bool from_string(T& v, const std::string& s)
{
	std::stringstream ss(s);
	ss >> v;
	return !ss.fail();
}

/// specialization to extract string value from string
template <> CGV_API bool from_string(std::string& v, const std::string& s);

	}
}

#include <cgv/config/lib_end.h>
