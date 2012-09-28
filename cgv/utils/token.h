#pragma once

#include <string>
#include <iostream>

#include "lib_begin.h"

namespace cgv {
	namespace utils {

/** representation of a token in a text by two pointers begin and end, that
    point to the first character of the token and behind the last character
	 of the token. In this way the two pointers form a range of characters that
	 can be used with stl functions. */
struct CGV_API token
{
	/// pointers that define the range of characters
	const char* begin, *end;
	/// construct with both pointers set to 0
	token();
	/// construct from character range
	token(const char* _b, const char* _e);
	/// construct from string
	token(const std::string& s);
	/// return the length of the token in number of characters
	size_t get_length() const;
	/// return the length of the token in number of characters
	size_t size() const;
	/// return whether the token is empty
	bool empty() const;
	/// set begin by skipping all instances of the given character set
	void skip(const std::string& skip_chars);
	/// set end by skipping all instances of the given character set
	void reverse_skip(const std::string& skip_chars);
	/// return the i-th character of the token
	char operator [] (unsigned int i) const;
	/// compare to const char*
	bool operator == (const char* s) const;
	/// compare to string
	bool operator == (const std::string& s) const;
	/// compare to const char*
	bool operator != (const char* s) const;
	/// compare to string
	bool operator != (const std::string& s) const;
};

/// return the length of the token in number of characters
inline size_t token::get_length() const { return end-begin; }
/// return the length of the token in number of characters
inline size_t token::size() const { return end-begin; }
/// return whether the token is empty
inline bool token::empty() const { return begin == end; }
/// return the i-th character of the token
inline char token::operator [] (unsigned int i) const { return begin[i]; }
/// convert to string
inline std::string to_string(const token& t) { return t.empty()?std::string():std::string(t.begin, t.get_length()); }
/// stream out operator
inline std::ostream& operator << (std::ostream& os, const token& t) { if (!t.empty()) os << std::string(t.begin, t.get_length()); return os; }

	}
}

#include <cgv/config/lib_end.h>