#include "advanced_scan.h"

namespace cgv {
	namespace utils {

void split_to_tokens(
			const char* begin, const char* end,
			std::vector<token>& tokens,
			const std::string& separators, 
			bool merge_separators,
			const std::string& open_parenthesis, const std::string& close_parenthesis, 
			const std::string& whitespaces,
			unsigned int max_nr_tokens)
{
	const char* b = begin;
	const char* p = b;
	size_t pos;
	bool last_is_sep = false;
	while (p < end) {
		bool is_sep = false;
		bool is_whitespace = false;
		bool create_token = false;
		if (p == end || (is_whitespace = is_element(*p, whitespaces)))
			create_token = true;
		else {
			is_sep=is_element(*p, separators);
			if (is_sep) {
				if (!(last_is_sep && merge_separators))
					create_token = true;
			}
			else
				if (last_is_sep)
					create_token = true;
		}
		last_is_sep = is_sep;
		if (create_token) {
			if (p > b)
				tokens.push_back(token(b,p));
			if (is_whitespace)
				b = p+1;
			else
				b = p;
		}
		else if ((pos = open_parenthesis.find_first_of(*p)) != std::string::npos) {
			b = ++p;
			while (p != end && *p != close_parenthesis[pos])
				++p;
			tokens.push_back(token(b,p));
			b = p+1;
		}
		if (p == end)
			break;
		++p;
		if (tokens.size() >= max_nr_tokens)
			break;
		if (p == end) {
			if (p > b)
				tokens.push_back(token(b,p));
		}
	};
}

void split_to_lines(const char* global_begin, const char* global_end, 
						  std::vector<line>& lines, 
						  bool truncate_trailing_spaces)
{
	const char* ptr = global_begin;
	while (ptr < global_end) {
		const char* begin = ptr;
		while (ptr < global_end && *ptr != '\n' )
			++ptr;
		const char* end = ptr;
		if (truncate_trailing_spaces) {
			while (end > begin && is_space(end[-1]))
				--end;
		}
		lines.push_back(line(begin,end));
		++ptr;
	}
}

bool balanced_find_content(
	const char* begin, const char* end, 
	token& content, 
	char open_parenthesis, char close_parenthesis)
{
	begin = skip_spaces(begin, end);
	if (end-begin < 2)
		return false;
	if (*begin != open_parenthesis)
		return false;
	++begin;
	content.begin = begin;
	const char* p = begin;
	int nesting_level = 1;
	while (p < end) {
		if (*p == close_parenthesis) {
			if (--nesting_level == 0) {
				content.end = p;
				return true;
			}
		}
		else if (*p == open_parenthesis)
			++nesting_level;
		++p;
	}
	return false;
}


	}
}
