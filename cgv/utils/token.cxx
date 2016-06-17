#include "token.h"
#include "scan.h"

namespace cgv {
	namespace utils {

/// construct with both pointers set to 0
token::token() : begin(0), end(0) 
{
}
/// construct from c-string
token::token(const char* _str) : begin(_str), end(_str)
{
	while (*end != 0)
		++end;
}

/// construct from character range
token::token(const char* _b, const char* _e) : begin(_b), end(_e) 
{
}
/// construct from string
token::token(const std::string& s) : begin(&s[0]), end(&s[0]+s.length()) 
{
}
/// return the length of the token in number of characters
//size_t token::get_length() const { return end-begin; }
/// return the length of the token in number of characters
//size_t token::size() const { return end-begin; }
/// return whether the token is empty
//bool token::empty() const { return begin == end; }
/// set begin by skipping all instances of the given character set
void token::skip(const std::string& skip_chars) 
{
	while (begin < end && is_element(*begin, skip_chars)) ++begin;
}
/// set end by skipping all instances of the given character set
void token::reverse_skip(const std::string& skip_chars) 
{
	while (begin < end && is_element(*(end-1), skip_chars)) --end; 
}

/// compare to const char*
bool token::operator == (const char* s) const
{
	return to_string(*this) == s;
}
/// compare to string
bool token::operator == (const std::string& s) const
{
	return to_string(*this) == s;
}
/// compare to const char*
bool token::operator != (const char* s) const
{
	return to_string(*this) != s;
}
/// compare to string
bool token::operator != (const std::string& s) const
{
	return to_string(*this) != s;
}

/*
/// convert to string
std::string token::str() const 
{
	return empty()?std::string():std::string(begin, end-begin); 
}
*/
/// return the i-th character of the token
//char token::operator [] (unsigned int i) const { return begin[i]; }

	}
}
