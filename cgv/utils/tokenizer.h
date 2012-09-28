#pragma once

#include <vector>

#include "token.h"

#include "lib_begin.h"

namespace cgv {
	namespace utils {

/** the tokenizer allows to split text into tokens in a convenient way.
    It supports splitting at white spaces and single or multi charactor
	 separators. Furthermore, it supports enclosing character pairs like
	 parantheses or string separators that skip white spaces and 
	 separators between enclosing pairs.

	 By default white spaces are set to space, tab, newline. The list
	 of separators and skip character pairs is empty by default.

	 A tokenizer can be constructed from a string, a cont char* or a
	 token. The resulting tokens are stored as two pointers to the 
	 begin and after the end of the token. No new memory is allocated
	 and the tokens are only valid as long as the string or const char*
	 is valid from which the tokenizer has been construct.

	 In the simplest usage, the tokenizer generates a vector of tokens
	 through the bite_all function. Suppose you want to split the string
	 str="Hello tokenizer." at the white spaces into two tokens
	 <Hello> and <tokenizer.>. Notice that no token contains the white
	 space separating the tokens. The following code performs this task:
\code
std::vector<token> toks;
bite_all(tokenizer(str), toks);
\endcode
	 If you want to also cut the dot into a separate token, just set
	 the list of separators with the set_sep method:
\code
std::vector<token> toks;
bite_all(tokenizer(str).set_sep("."), toks);
\endcode
    The result are three tokens: <Hello>, <tokenizer> and <.>. If you
	 want to split a semicolon separated list with tokens that can
	 contain white spaces and ignoring the semicolons, you can set the
	 semicolon character as the only white space:
\code
std::vector<token> toks;
bite_all(tokenizer(str).set_ws(";"), toks);
\endcode
    The previous code would split the string "a and b;c and d" into
	 two tokens <a and b> and <c and d>.

	 If you want to not split into tokens in between strings enclosed
	 by <'> and in between paranthesis, you can several skip character
	 pairs:
\code
std::vector<token> toks;
bite_all(tokenizer(str).set_sep("[]").set_skip("'({", "')}"), toks);
\endcode
    The previous code example would split the string "'a b'[{c d}]"
	 into four tokens: <'a b'>, <[>, <{c d}> and <]>. Note that you
	 can apply several setter methods to the tokenizer in a sequence
	 as each setter returns a reference to the tokenizer itself similar
	 to the stream operators.

	 */
class CGV_API tokenizer : public token
{
protected:
	std::string separators;
	bool merge_separators;
	std::string begin_skip;
	std::string end_skip;
	std::string escape_skip;
	std::string whitespaces;
	void init();
	bool handle_skip(token& result);
	bool handle_separators(token& result,bool check_skip=true);
	bool reverse_handle_skip(token& result);
	bool reverse_handle_separators(token& result,bool check_skip=true);
public:
	/// construct empty tokenizer
	tokenizer();
	/// construct from token
	tokenizer(const token&);
	/// construct from character string
	tokenizer(const char*);
	/// construct from string
	tokenizer(const std::string&);
	/// set the list of white spaces, that separate tokens and are skipped
	tokenizer& set_ws(const std::string& ws);
	/// set several character pairs that enclose tokens that are not split
	tokenizer& set_skip(const std::string& open, const std::string& close);
	/// set several character pairs that enclose tokens that are not split and one escape character for each pair
	tokenizer& set_skip(const std::string& open, const std::string& close, const std::string& escape);
	/// set the list of separators and specify whether succeeding separators are merged into single tokens
	tokenizer& set_sep(const std::string& sep, bool merge);
	/// set the list of separators 
	tokenizer& set_sep(const std::string& sep);
	/// specify whether succeeding separators are merged into single tokens
	tokenizer& set_sep_merge(bool merge);
	/// bite away a single token from the front
	token bite();
	/// bite away a single token from the back
	token reverse_bite();
	/// skip whitespaces at the back
	void reverse_skip_whitespaces();
	/// skip whitespaces at the front
	void skip_whitespaces();
	/// skip whitespaces at the front and return whether the complete text has been processed
	bool skip_ws_check_empty() { skip_whitespaces(); return empty(); }
	/// skip whitespaces at the back and return whether the complete text has been processed
	bool reverse_skip_ws_check_empty() { reverse_skip_whitespaces(); return empty(); }
	/// bite one token until all potentially nested opended parenthesis have been closed again
	bool balanced_bite(token& result, const std::string& open_parenthesis, const std::string& close_parenthesis, bool wait_for_sep = false);
	/// 
	void bite_all(std::vector<token>& result);
};

/// bite all tokens into a token vector
inline void bite_all(tokenizer& t, std::vector<token>& result) { while(!t.skip_ws_check_empty())result.push_back(t.bite()); }

	}
}

#include <cgv/config/lib_end.h>