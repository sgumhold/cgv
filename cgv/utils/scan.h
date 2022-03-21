#pragma once

/** \file scan.h
 * Helper functions to process strings.
 */

#include <string>
#include <vector>

#include "date_time.h"

#include "lib_begin.h"

namespace cgv {
	namespace utils {

/// return new start pointer by skipping spaces at begin
extern CGV_API const char* skip_spaces(const char* begin, const char* end);
/// return new end pointer by cutting off spaces at the end
extern CGV_API const char* cutoff_spaces(const char* begin, const char* end);
/// trim white space or other characters from start of string
extern CGV_API std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ");
/// trim white space or other characters from end of string
extern CGV_API std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ");
/// trim white space or other characters from start and end of string
extern CGV_API std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ");
/// trim white space or other characters from start of string
extern CGV_API std::string ltrim(const std::string& str, const std::string& chars = "\t\n\v\f\r ");
/// trim white space or other characters from end of string
extern CGV_API std::string rtrim(const std::string& str, const std::string& chars = "\t\n\v\f\r ");
/// trim white space or other characters from start and end of string
extern CGV_API std::string trim(const std::string& str, const std::string& chars = "\t\n\v\f\r ");
/// check if char is a whitespace
extern CGV_API bool is_space(char c);
/// check if char is a special character from an url
extern CGV_API bool is_url_special(char c);
/// check if char is a digit
extern CGV_API bool is_digit(char c);
/// check if char is a letter
extern CGV_API bool is_letter(char c);
/// convert char to lower case
extern CGV_API char to_lower(char c);
/// convert to hex
extern CGV_API std::string to_hex(uint8_t v, bool use_upper_case = true);
/// convert from hex character
extern CGV_API uint8_t from_hex(char c);
/// parse bytes hex coded bytes
extern CGV_API std::vector<uint8_t> parse_hex_bytes(const std::string& byte_str);
/// convert string to lower case
extern CGV_API std::string to_lower(const std::string& _s);
/// convert char to upper case
extern CGV_API char to_upper(char c);
/// convert string to upper case
extern CGV_API std::string to_upper(const std::string& _s);
/// replaces the german special characters ä,ö,ü,ß,Ä,Ö,Ü
extern CGV_API std::string replace_special(const std::string& _s);
/// replace char \c c1 with \c c2 in the given string \c _s and return number of replacements
extern CGV_API unsigned int replace(std::string& _s, char c1, char c2);
/// replace string \c s1 with \c s2 in the given string \c _s and return number of replacements
extern CGV_API unsigned int replace(std::string& _s, const std::string& s1, const std::string& s2);
/// interprets the C++ special characters \c \\a, \c \\b, \c \\f, \c \\n, \c \\r, \c \\t, \c \\v, \c \\\', \c \\\", \c \\\\, \c \\?, \c \\ooo, \c \\xhh
extern CGV_API std::string interpret_special(const std::string& s);
/// escapes the C++ special characters \c \\a, \c \\b, \c \\f, \c \\n, \c \\r, \c \\t, \c \\v, \c \\\', \c \\\", \c \\\\, \c \\?
extern CGV_API std::string escape_special(const std::string& s);
/// check if string \c s is contained in the given array of names and in case of success store name index in \c idx
extern CGV_API bool find_name(const std::string& s, const char* names[], int& idx);
/// check if char \c c arises in string \c s
extern CGV_API bool is_element(char c, const std::string& s);
/// check if the string e is contained as element in the string s, which is a list separated by sep
extern CGV_API bool is_element(const std::string& e, const std::string& s, char sep = ';');
/** check if the string e is contained as element in the string s, which is a list separated 
	by sep and return the index of the element. If not contained, return -1. */
extern CGV_API int get_element_index(const std::string& e, const std::string& s, char sep = ';');
/** interpret s as a list separated by sep and return the element with the given element index. If index is out of range, return empty string. */
extern CGV_API std::string get_element(const std::string& s, int element_index, char sep = ';');
/// check if the text range (begin,end( defines an integer value. If yes, store the value in the passed reference.
extern CGV_API bool is_integer(const char* begin, const char* end, int& value);
/// check if the passed string defines an integer value. If yes, store the value in the passed reference.
extern CGV_API bool is_integer(const std::string& s, int& value);
/// check if the text range (begin,end( defines a double value. If yes, store the value in the passed reference.
extern CGV_API bool is_double(const char* begin, const char* end, double& value);
/// check if the passed string defines a double value. If yes, store the value in the passed reference.
extern CGV_API bool is_double(const std::string& s, double& value);
/// check and extract year from string token [\c begin, \c end]
extern CGV_API bool is_year(const char* begin, const char* end, unsigned short& year, bool short_allowed = true);
/// check and extract year from string \c s
extern CGV_API bool is_year(const std::string& s, unsigned short& year, bool short_allowed = true);
/// check and extract day from string token [\c begin, \c end]
extern CGV_API bool is_day(const char* begin, const char* end, unsigned char& day);
/// check and extract day from string \c s
extern CGV_API bool is_day(const std::string& s, unsigned char& day);
/// check and extract month from string token [\c begin, \c end]
extern CGV_API bool is_month(const char* begin, const char* end, unsigned char& month);
/// check and extract month from string \c s
extern CGV_API bool is_month(const std::string& s, unsigned char& month);
/// check and extract time from string token [\c begin, \c end]
extern CGV_API bool is_time(const std::string& s, cgv::utils::time& t, const char **new_end = 0);
/// check and extract time from string \c s
extern CGV_API bool is_time(const char* begin, const char* end, cgv::utils::time& t, const char **new_end = 0);
/// check and extract date from string token [\c begin, \c end]
extern CGV_API bool is_date(const std::string& s, cgv::utils::date& d, const char **new_end = 0);
/// check and extract date from string \c s
extern CGV_API bool is_date(const char* begin, const char* end, cgv::utils::date& d, const char **new_end = 0);
/// check and extract end of valid url from string \c s
extern CGV_API bool is_url(const std::string& s, const char** end = 0);
/// check and extract end of valid url from string token [\c begin, \c end]
extern CGV_API bool is_url(const char* begin, const char* end, const char** new_end = 0);

	}
}

#include <cgv/config/lib_end.h>