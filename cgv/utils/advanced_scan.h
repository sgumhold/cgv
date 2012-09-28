#pragma once

#include <vector>

#include "token.h"
#include "scan.h"

#include "lib_begin.h"

namespace cgv {
	namespace utils {

/** a line in a text is simply represented as a token */
struct line : public token
{
	/// construct from range
	line(const char* _b = 0, const char* _e = 0) : token(_b, _e) {}
};

/// different types that a typed_token can have
enum token_type {
	PLAIN, URL, FLOAT_VALUE, TIME_VALUE, DATE_VALUE
};

/** a typed token also stores the type and value of a parsed token. Although
    a union of the different typed values should have been used, the values
	 of different type are stored successively in the typed_token because of 
	 problems with standard construction. */
struct typed_token : public token
{
	token_type type;
	typed_token(const token& t, token_type tt = PLAIN) : token(t), type(tt), date_value(0) {}
	double float_value;
	utils::time time_value;
	utils::date date_value;
};

/** this function splits a text range into tokens. The function is typically applied
    to a line. One can specify whitespaces that are used to detect split locations
	 but do not generate any tokens. The separators also mark split locations and 
	 generate tokens. If merge_separators is true, successive separators are merged
	 into one token. Finally, one can specify a pair of character lists 
	 open_paranthesis and close_paranthesis that are used to overwrite the 
	 splitting mechanism and allow to generate tokens including whitespaces and 
	 separators. Some examples:

	 applying 

	 vector<token> tokens;
	 split_to_tokens(line.begin, line.end, tokens, ".,;:", "'{", "'}", " \t");

	 to the line

	 Hello world... This is a "split to tokens" test line. {copyright: S. G.}

	 generates the following tokens:

	 Hello
	 world
	 ...
	 This
	 is
	 a
	 split to tokens
	 test
	 line
	 .
	 copyright: S. G.
*/
extern CGV_API void split_to_tokens(
			const char* begin, const char* end,
			std::vector<token>& tokens,
			const std::string& separators, 
			bool merge_separators = true,
			const std::string& open_parenthesis = "", const std::string& close_parenthesis = "", 
			const std::string& whitespaces = " \t\n",
			unsigned int max_nr_tokens = -1);

/// text range given as token
inline void split_to_tokens(
			const token& tok,
			std::vector<token>& tokens,
			const std::string& separators, 
			bool merge_separators = true,
			const std::string& open_parenthesis = "", const std::string& close_parenthesis = "", 
			const std::string& whitespaces = " \t\n",
			unsigned int /*max_nr_tokens*/ = -1) 
{
	split_to_tokens(
		tok.begin,tok.end,
		tokens,
		separators,merge_separators,open_parenthesis,close_parenthesis,whitespaces); 
}

/// text range given as string
inline void split_to_tokens(
			const std::string& s,
			std::vector<token>& tokens,
			const std::string& separators, 
			bool merge_separators = true,
			const std::string& open_parenthesis = "", const std::string& close_parenthesis = "", 
			const std::string& whitespaces = " \t\n",
			unsigned int /*max_nr_tokens*/ = (unsigned int)-1)
{
	split_to_tokens(&s[0],&s[0]+s.size(),tokens,separators,merge_separators,open_parenthesis,close_parenthesis,whitespaces);
}

/** this function splits a text range at the newline characters into single lines.
    If truncate_trailing_spaces is true all spaces and tabs at the end of the
	 line are excluded. */
extern CGV_API void split_to_lines(const char* begin, const char* end, 
											  std::vector<line>& lines, 
											  bool truncate_trailing_spaces = true);
/// text range given as token
inline void split_to_lines(const token& tok, 
											  std::vector<line>& lines, 
											  bool truncate_trailing_spaces = true) { 
	split_to_lines(tok.begin,tok.end,lines,truncate_trailing_spaces); 
}

/// text range given as string
inline void split_to_lines(const std::string& s, 
											  std::vector<line>& lines, 
											  bool truncate_trailing_spaces = true) { 
	split_to_lines(&s[0],&s[0]+s.size(),lines,truncate_trailing_spaces); 
}

/** the input range must begin with an open parenthesis. The function
    finds the matching closing parenthesis and returns a token with the
	 content inside the parentheses.

	 Examples:

	 balanced_find_content( "a+b", => , '(', ')' ) ==> false;
	 balanced_find_content( "(a+b)", => "a+b", '(', ')' ) ==> true;
	 balanced_find_content( "(a+(c*d),b)", => "a+(c*d),b", '(', ')' ) ==> true;
*/
extern CGV_API bool balanced_find_content(
	const char* begin, const char* end, 
	token& content, 
	char open_parenthesis, char close_parenthesis);

inline bool balanced_find_content(
			const token& expression, 
			token& content, 
			char open_parenthesis, char close_parenthesis) {
	return balanced_find_content(expression.begin, expression.end,
		content,open_parenthesis,close_parenthesis);
}
inline bool balanced_find_content(
			const std::string& expression, 
			token& content, 
			char open_parenthesis, char close_parenthesis) {
	return balanced_find_content(&expression[0], &expression[0]+expression.size(),
		content,open_parenthesis,close_parenthesis);
}

	}
}

#include <cgv/config/lib_end.h>