#pragma once

#include <vector>
#include "expression_processor.h"

#include "lib_begin.h"

namespace cgv {
	namespace media {
		namespace text {
			namespace ppp {

enum CommandType {
	CT_IMPLICIT_TEXT = -1,
	CT_TEXT = 0,
	CT_DEFINE, 
	CT_SKIP,
	CT_READ,
	CT_WRITE,
	CT_RAND,
	CT_NAMESPACE,
	CT_FOR,
	CT_IF, CT_ELIF, CT_ELSE,
	CT_EVAL, CT_STRING, CT_LIST, 
	CT_BEGIN, CT_END, 
	CT_INCLUDE, CT_CINCLUDE, CT_EXCLUDE, CT_INSERT,
	CT_REFLECT_NEXT_LINE, CT_REFLECT_PREV_LINE,
	CT_COUT, CT_CIN, CT_ERROR, CT_WARNING, 
	CT_SYSTEM, CT_DIR, CT_FUNC, CT_EXIT,	
	CT_TRANSFORM,
	CT_SCAN_INCLUDES,
	CT_UNDEF
};

struct CGV_API command_token : public token
{
protected:
	static std::string last_error;
	static token last_error_token;
public:
	static const std::string& get_last_error();
	static const token& get_last_error_token();
	/// store command type
	CommandType ct;
	/// store index of parenthesis that enclosed the expressions
	unsigned int parenthesis_index;
	/// store the index of the command token that terminates the block following this command
	unsigned int block_end;
	/// vector of parsed expressions
	std::vector<expression_processor> expressions;

	/// constructs an undefined command token
	command_token();
	/// constructs a command token of type text
	command_token(const token& t);
	/// splits a command token from the front of a given token
	bool split_off_from(token& t);
	/// get the number of characters to be skipped before expressions start
	unsigned get_skip_length() const;
	/// return the number of expressions that the command takes as argument
	unsigned get_nr_expressions() const;
	/// return the minimum number of expressions that the command takes as argument
	unsigned get_min_nr_expressions() const;
	/// return whether the number of expressions that the command takes as argument is fix
	bool is_nr_expressions_fix() const;
	/// return the keyword describing the command
	const char* get_keyword() const;
	/// check whether the token only contains white spaces
	bool is_empty() const;
	/// return whether the command token must be followed by a block
	bool block_follows() const;
	/// return whether empty text tokens before this command token should be deleted
	bool remove_preceeding_empty_text_token() const;
	/// return the open parenthesis enclosing the expressions
	char get_open_parenthesis() const;
	/// return the close parenthesis enclosing the expressions
	char get_close_parenthesis() const;
};

			}
		}
	}
}
#include <cgv/config/lib_end.h>