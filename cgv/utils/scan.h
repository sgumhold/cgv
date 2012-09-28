#pragma once

#include <string>

#include "date_time.h"

#include "lib_begin.h"

namespace cgv {
	namespace utils {

extern CGV_API const char* skip_spaces(const char* begin, const char* end);
extern CGV_API const char* cutoff_spaces(const char* begin, const char* end);
extern CGV_API bool is_space(char c);
extern CGV_API bool is_url_special(char c);
extern CGV_API bool is_digit(char c);
extern CGV_API bool is_letter(char c);
extern CGV_API char to_lower(char c);
extern CGV_API std::string to_lower(const std::string& _s);
extern CGV_API char to_upper(char c);
extern CGV_API std::string to_upper(const std::string& _s);
/// replaces the german special characters ä,ö,ü,ß,Ä,Ö,Ü
extern CGV_API std::string replace_special(const std::string& _s);
extern CGV_API unsigned int replace(std::string& _s, char c1, char c2);
extern CGV_API unsigned int replace(std::string& _s, const std::string& s1, const std::string& s2);
/// interprets the C++ special characters \a, \b, \f, \n, \r, \t, \v, \', \", \\, \?, \ooo, \xhh
extern CGV_API std::string interpret_special(const std::string& s);
/// escapes the C++ special characters \a, \b, \f, \n, \r, \t, \v, \', \", \\, \?
extern CGV_API std::string escape_special(const std::string& s);
extern CGV_API bool find_name(const std::string& s, const char* names[], int& idx);
extern CGV_API bool is_element(char c, const std::string& s);
/// check if the string e is contained as element in the string s, which is a list separated by sep
extern CGV_API bool is_element(const std::string& e, const std::string& s, char sep = ';');
/** check if the string e is contained as element in the string s, which is a list separated 
	by sep and return the index of the element. If not contained, return -1. */
extern CGV_API int get_element_index(const std::string& e, const std::string& s, char sep = ';');
/// check if the text range (begin,end( defines an integer value. If yes, store the value in the passed reference.
extern CGV_API bool is_integer(const char* begin, const char* end, int& value);
/// check if the passed string defines an integer value. If yes, store the value in the passed reference.
extern CGV_API bool is_integer(const std::string& s, int& value);
/// check if the text range (begin,end( defines a double value. If yes, store the value in the passed reference.
extern CGV_API bool is_double(const char* begin, const char* end, double& value);
/// check if the passed string defines a double value. If yes, store the value in the passed reference.
extern CGV_API bool is_double(const std::string& s, double& value);
extern CGV_API bool is_year(const char* begin, const char* end, unsigned short& year, bool short_allowed = true);
extern CGV_API bool is_year(const std::string& s, unsigned short& year, bool short_allowed = true);
extern CGV_API bool is_day(const char* begin, const char* end, unsigned char& day);
extern CGV_API bool is_day(const std::string& s, unsigned char& day);
extern CGV_API bool is_month(const char* begin, const char* end, unsigned char& month);
extern CGV_API bool is_month(const std::string& s, unsigned char& month);
extern CGV_API bool is_time(const std::string& s, cgv::utils::time& t, const char **new_end = 0);
extern CGV_API bool is_time(const char* begin, const char* end, cgv::utils::time& t, const char **new_end = 0);
extern CGV_API bool is_date(const std::string& s, cgv::utils::date& d, const char **new_end = 0);
extern CGV_API bool is_date(const char* begin, const char* end, cgv::utils::date& d, const char **new_end = 0);
extern CGV_API bool is_url(const std::string& s, const char** end = 0);
extern CGV_API bool is_url(const char* begin, const char* end, const char** new_end = 0);

	}
}

#include <cgv/config/lib_end.h>