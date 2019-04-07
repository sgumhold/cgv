#include "command_token.h"
#include <cgv/utils/scan.h>
#include <cgv/utils/convert_string.h>
#include <cgv/utils/tokenizer.h>

using namespace cgv::utils;

namespace cgv {
	namespace ppp {

		std::string command_token::last_error;
		token command_token::last_error_token;

		struct command_info
		{
			CommandType ct;
			const char* word;
			unsigned length;
			unsigned skip_length;
			unsigned min_nr_expressions;
			unsigned nr_expressions;
			bool block_follows;
			bool remove_empty_before;
			bool parse_expressions;
			const char* open_parentheses;
			const char* close_parentheses;
		};

		const command_info& get_command_info(CommandType ct)
		{
			static const command_info infos[] = {
			{ CT_TEXT,      "text",    4, 4, 1, 1, false,  true, true,  "(",   ")" },
			{ CT_DEFINE,    "define",  6, 6, 1, 1, false,  true, true,  "(",   ")" },
			{ CT_SKIP,      "skip",    4, 4, 1, 1, false,  true, true,  "(",   ")" },
			{ CT_READ,      "read",    4, 4, 3, 3, false,  true, true,  "(",   ")" },
			{ CT_WRITE,     "write",  5, 5, 3, 3, false,   true, true,  "(",   ")" },
			{ CT_RAND,      "rand",    4, 4, 3, 4, false,  true, true,  "(",   ")" },
			{ CT_NAMESPACE, "namespace", 9, 9, 1, 1, true, true, true,  "(",   ")" },
			{ CT_FOR,       "for",     3, 3, 3, 3, true,   true, true,  "(",   ")" },
			{ CT_IF,        "if",      2, 2, 1, 1, true,   true, true,  "(",   ")" },
			{ CT_ELIF,      "elif",    4, 4, 1, 1, true,   true,  true,  "(",   ")" },
			{ CT_ELSE,      "else",    4, 4, 0, 0, true,   true,  true,  "",    "" },
			{ CT_EVAL,      "(",       1, 0, 1, 1, false,  false, true,  "(",   ")" },
			{ CT_STRING,    "\"",      1, 0, 1, 1, false,  false, true,  "\"",  "\"" },
			{ CT_LIST,      "[",       1, 0, 3, 3, false,  false, true,  "[",   "]" },
			{ CT_BEGIN,     "{",       1, 1, 0, 0, false,  true,  true,  "",    "" },
			{ CT_END,       "}",       1, 1, 0, 0, false,  true, true,  "",    "" },
			{ CT_INCLUDE,   "include", 7, 7, 1, 1, false,  true, false, "<\"(", ">\")" },
			{ CT_CINCLUDE,  "#include", 8, 8, 1, 1, false,  true, false, "<\"(", ">\")" },
			{ CT_EXCLUDE,   "exclude", 7, 7, 1, 1, false,  true, false, "<\"(", ">\")" },
			{ CT_INSERT,    "insert",  6, 6, 1, 1, false,  true, false, "<\"(", ">\")" },
			{ CT_REFLECT_NEXT_LINE, ">", 1, 1, 0, 0, false, false, false, "", "" },
			{ CT_REFLECT_PREV_LINE, "<", 1, 1, 0, 0, false, false, false, "", "" },
			{ CT_COUT,    "cout",    4, 4, 1, 1, false, true, true,  "(",   ")" },
			{ CT_CIN,     "cin",     3, 3, 1, 1, false, true, true,  "(",   ")" },
			{ CT_ERROR,   "error",   5, 5, 2, 2, false, true, true,  "(",   ")" },
			{ CT_WARNING, "warning", 7, 7, 2, 2, false, true, true,  "(",   ")" },
			{ CT_SYSTEM,  "system",  6, 6, 2, 2, false, true, true,  "(",   ")" },
			{ CT_DIR,  "dir",        3, 3, 4, 4, false, true, true,  "(",   ")" },
			{ CT_FUNC,  "func",        4, 4, 1, 2, true, true, true,  "(",   ")" },
			{ CT_EXIT,  "exit",        4, 4, 1, 1, false, true, true,  "(",   ")" },
			{ CT_TRANSFORM,  "transform", 9, 9, 2, 2, false, true, true,  "(",   ")" },
			{ CT_SCAN_INCLUDES,  "scan_includes", 13, 13, 2, 2, false, true, true,  "(",   ")" },
			};
			return infos[ct];
		}

		const char* get_command_word(CommandType ct)
		{
			switch (ct) {
			case CT_IMPLICIT_TEXT: return "text";
			case CT_UNDEF: return "undef";
			default: return get_command_info(ct).word;
			}
		}


		CommandType determine_command_type(const std::string& s)
		{
			// analyze length of command
			unsigned l = 0;
			while (l < s.size() && (is_digit(s[l]) || is_letter(s[l]) || s[l] == '#' || s[l] == '_'))
				++l;
			if (l == 0)
				l = 1;

			for (CommandType ct = (CommandType)0; ct < CT_UNDEF; ct = CommandType(ct + 1)) {
				const command_info& ci = get_command_info(ct);
				if (l == ci.length && s.substr(0, ci.length) == ci.word)
					return ct;
			}
			return CT_UNDEF;
		}

		command_token::command_token()
			: ct(CT_UNDEF), parenthesis_index(0), block_end(0)
		{
		}
		/// constructs a command token of type text
		command_token::command_token(const token& t)
			: token(t), ct(CT_IMPLICIT_TEXT), parenthesis_index(0), block_end(0)
		{
		}

		/// get the number of characters to be skipped before expressions start
		unsigned command_token::get_skip_length() const
		{
			return get_command_info(ct).skip_length;
		}

		/// return the number of expressions that the command takes as argument
		unsigned command_token::get_nr_expressions() const
		{
			return get_command_info(ct).nr_expressions;
		}

		/// return the minimum number of expressions that the command takes as argument
		unsigned command_token::get_min_nr_expressions() const
		{
			return get_command_info(ct).min_nr_expressions;
		}

		/// return whether the number of expressions that the command takes as argument is fix
		bool command_token::is_nr_expressions_fix() const
		{
			return get_min_nr_expressions() == get_nr_expressions();
		}


		/// return the keyword describing the command
		const char* command_token::get_keyword() const
		{
			return get_command_info(ct).word;
		}

		/// check whether the token only contains white spaces
		bool command_token::is_empty() const
		{
			if (ct != CT_IMPLICIT_TEXT)
				return false;
			if (empty())
				return true;

			if (skip_spaces(begin, end) != end)
				return false;

			unsigned int nr_lines = 0;
			for (const char* p = begin; p < end; ++p)
				if (*p == '\n')
					if (++nr_lines > 1)
						break;
			if (nr_lines < 2)
				return true;
			return false;
		}

		/// return whether the command token must be followed by a block
		bool command_token::block_follows() const
		{
			if (ct == CT_IMPLICIT_TEXT || ct == CT_UNDEF)
				return false;
			return get_command_info(ct).block_follows;
		}

		/// return whether empty text tokens before this command token should be deleted
		bool command_token::remove_preceeding_empty_text_token() const
		{
			if (ct == CT_IMPLICIT_TEXT || ct == CT_UNDEF)
				return false;
			return get_command_info(ct).remove_empty_before;
		}


		/// return the open parenthesis enclosing the expressions
		char command_token::get_open_parenthesis() const
		{
			return get_command_info(ct).open_parentheses[parenthesis_index];
		}

		/// return the close parenthesis enclosing the expressions
		char command_token::get_close_parenthesis() const
		{
			return get_command_info(ct).close_parentheses[parenthesis_index];
		}

		/// splits a command token from the front of a given token
		bool command_token::split_off_from(token& t)
		{
			std::string s = to_string(t);
			ct = determine_command_type(s);
			if (ct == CT_UNDEF) {
				last_error = "could not determine command";
				last_error_token = t;
				return false;
			}
			begin = t.begin;
			t.begin += get_skip_length();
			end = t.begin;
			unsigned ne = get_nr_expressions();
			unsigned mne = get_min_nr_expressions();
			if (ne == 0)
				return true;

			const std::string& open_ps = get_command_info(ct).open_parentheses;
			const std::string& close_ps = get_command_info(ct).close_parentheses;

			token exp_tok;
			if (tokenizer(t).set_skip("'\"", "'\"").balanced_bite(exp_tok, open_ps, close_ps)) {
				if (!is_element(*exp_tok.begin, open_ps)) {
					last_error = "enclosing parenthesis not valid";
					last_error_token = *this;
					return false;
				}
				t.begin = exp_tok.end;
				parenthesis_index = (int)open_ps.find_first_of(*exp_tok.begin);
				++exp_tok.begin;
				--exp_tok.end;
				std::vector<token> expr_tokens;
				bite_all(tokenizer(exp_tok).set_skip("'\"", "'\"").set_ws(";"), expr_tokens);
				if (expr_tokens.size() > ne || expr_tokens.size() < mne) {
					last_error = "found " + to_string(expr_tokens.size()) + " expressions, but " + get_keyword() + " allows ";
					if (ne == mne)
						last_error += "only " + to_string(ne) + " expressions";
					else
						last_error += "from " + to_string(mne) + " to " + to_string(ne) + " expressions";
					last_error_token = *this;
					return false;
				}
				if (get_command_info(ct).parse_expressions || (parenthesis_index == 2)) {
					for (unsigned int i = 0; i < expr_tokens.size(); ++i) {
						expression_processor ep;
						if (!ep.parse(expr_tokens[i])) {
							last_error = "parse error in expression ";
							last_error += to_string(i + 1);
							last_error_token = expr_tokens[i];
							return false;
						}
						bool allow_several_values = false;
						if (ct == CT_FUNC && i == 1)
							allow_several_values = true;
						if (!ep.validate(allow_several_values)) {
							last_error = "could not validate expression ";
							last_error += to_string(i + 1) + " (";
							last_error += ep.get_last_error() + ")";
							if (ep.get_last_error_token().empty())
								last_error_token = expr_tokens[i];
							else
								last_error_token = ep.get_last_error_token();
							return false;
						}
						expressions.push_back(ep);
					}
				}
				else {
					begin = exp_tok.begin;
					end = exp_tok.end;
				}
				return true;
			}
			last_error = "could not find parenthesis enclosing expressions";
			last_error_token = t;
			return false;
		}

		const std::string& command_token::get_last_error()
		{
			return last_error;
		}
		const token& command_token::get_last_error_token()
		{
			return last_error_token;
		}

	}
}
