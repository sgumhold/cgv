#include "tokenizer.h"
#include "scan.h"

namespace cgv {
	namespace utils {

void tokenizer::init()
{
	separators = "";
	merge_separators = false;
	begin_skip = "";
	end_skip = "";
	whitespaces = " \t\n";
}

tokenizer::tokenizer()
{
	init();
}

tokenizer::tokenizer(const token& t) : token(t)
{
	init();
}

tokenizer::tokenizer(const char* p)
{
	begin = p;
	end = p + std::string(p).size();
	init();
}

tokenizer::tokenizer(const std::string& s) : token(s)
{
	init();
}

tokenizer& tokenizer::set_ws(const std::string& ws)
{
	whitespaces = ws;
	return *this;
}

tokenizer& tokenizer::set_skip(const std::string& open, const std::string& close)
{
	begin_skip = open;
	end_skip = close;
	return *this;
}

tokenizer& tokenizer::set_skip(const std::string& open, const std::string& close, const std::string& escape)
{
	begin_skip = open;
	end_skip = close;
	escape_skip = escape;
	return *this;
}

tokenizer& tokenizer::set_sep(const std::string& sep, bool merge)
{
	separators = sep;
	merge_separators = merge;
	return *this;
}

tokenizer& tokenizer::set_sep(const std::string& sep)
{
	separators = sep;
	return *this;
}

tokenizer& tokenizer::set_sep_merge(bool merge)
{
	merge_separators = merge;
	return *this;
}

bool tokenizer::handle_skip(token& result)
{
	if (result.end >= end)
		return false;
	std::size_t i = begin_skip.find_first_of(*result.end);
	if (i == std::string::npos) 
		return false;
	++result.end;
	if (escape_skip.size() > i) {
		bool last_was_escape = false;
		while (result.end < end) {
			if (last_was_escape)
				last_was_escape = false;
			else if (*result.end == escape_skip[i])
				last_was_escape = true;
			else if (*result.end == end_skip[i]) 
				break;
			++result.end;
		}
	}
	else {
		while (result.end < end) {
			if (*result.end == end_skip[i]) 
				break;
			++result.end;
		}
	}
	return true;
}

bool tokenizer::reverse_handle_skip(token& result)
{
	if (result.begin <= begin)
		return false;
	std::size_t i = end_skip.find_first_of(*(result.begin-1));
	if (i == std::string::npos) 
		return false;
	--result.begin;
	if (escape_skip.size() > i) {
		while (result.begin > begin) {
			if (*(result.begin-1) == begin_skip[i]) {
				unsigned nr_escape = 0;
				while (result.begin-nr_escape-1 > begin && *(result.begin-nr_escape-2) == escape_skip[i])
					++nr_escape;
				if ((nr_escape & 1) == 1)
					break;
			}
			--result.begin;
		}
	}
	else {
		while (result.begin > begin) {
			if (*(result.begin-1) == begin_skip[i]) 
				break;
			--result.begin;
		}
	}
	return true;
}

bool tokenizer::handle_separators(token& result, bool check_skip)
{
	// handle separator tokens
	bool did_skip = false;
	if (check_skip)
		did_skip = handle_skip(result);
	if (result.end < end && is_element(*result.end, separators)) {
		if (merge_separators && !did_skip) {
			skip(separators);
			result.end = begin;
		}
		else
			begin = result.end = result.end+1;
		return true;
	}
	return false;
}

bool tokenizer::reverse_handle_separators(token& result, bool check_skip)
{
	// handle separator tokens
	bool did_skip = false;
	if (check_skip)
		did_skip = reverse_handle_skip(result);

	if (result.begin > begin && is_element(*(result.begin-1), separators)) {
		if (merge_separators && !did_skip) {
			reverse_skip(separators);
			end = result.begin;
		}
		else
			end = result.begin = result.begin+1;
		return true;
	}
	return false;
}

token tokenizer::bite()
{
	// handle whitespaces
	skip_whitespaces();
	token result(begin, begin);

	if (handle_separators(result)) {
		begin = result.end;
		return result;
	}
	if (result.end == end) {
		begin = result.end;
		return result;
	}
	// merge non separator characters
	while (++result.end < end) {
		// handle skip characters
		const char* tmp_end = result.end;
		if (handle_skip(result) && result.end == end)
			break;
		if (result.end < end &&
			 ( is_element(*result.end, separators) ||
			   is_element(*result.end, whitespaces) ) ) {
			begin = result.end = tmp_end;
			return result;
		}
	}
	begin = result.end;
	return result;
}

token tokenizer::reverse_bite()
{
	// handle whitespaces
	reverse_skip_whitespaces();
	token result(end, end);

	if (reverse_handle_separators(result))
		return result;
	if (result.begin == begin)
		return result;
	// merge non separator characters
	while (--result.begin > begin) {
		// handle skip characters
		const char* tmp_begin = result.begin;
		reverse_handle_skip(result);
		if (result.begin > begin &&
			 ( is_element(*(result.begin-1), separators) ||
			   is_element(*(result.begin-1), whitespaces) ) ) {
			end = result.begin = tmp_begin;
			return result;
		}
	}
	end = result.begin;
	return result;
}

bool tokenizer::balanced_bite(token& result, const std::string& open_parenthesis, const std::string& close_parenthesis, bool wait_for_sep)
{
	// count the nesting level of all parentheses
	std::vector<int> nesting;
	nesting.resize(open_parenthesis.size());
	std::fill(nesting.begin(),nesting.end(),0);
	int nr_nested = 0;
	// handle whitespaces
	skip_whitespaces();
	result = token(begin, begin);

	if (handle_separators(result, false))
		return true;
	// remember whether we are inside a token
	bool inside_token = true;
	// merge non separator characters
	while (result.end < end) {
		// handle parenthesis
		std::size_t i_close = close_parenthesis.find_first_of(*result.end);
		std::size_t i_open  = open_parenthesis.find_first_of(*result.end);
		if (i_close != std::string::npos || i_open != std::string::npos) {
			if (!wait_for_sep)
				inside_token = false;
			// first handle case when open and close parentheses are identical
			if (i_open == i_close) {
				if (nesting[i_open] == 0) {
					++nesting[i_open];
					++nr_nested;
				}
				else {
					if (--nesting[i_open] == 0)
						--nr_nested;
				}
			}
			else if (i_close != std::string::npos) {
				if (nesting[i_close] == 0)
					return false;
				if (--nesting[i_close] == 0)
					--nr_nested;
			}
			else {
				if (++nesting[i_open] == 1)
					++nr_nested;
			}
		}
		else {
			// handle skip characters
			const char* tmp_end = result.end;
			handle_skip(result);
			if (is_element(*result.end, separators) ||
				 (is_element(*result.end, whitespaces) && nr_nested == 0) ) {
				begin = result.end = tmp_end;
				return nr_nested == 0;
			}
		}
		++result.end;
		if (!inside_token && nr_nested == 0)
			break;
	}
	if (nr_nested == 0) {
		begin = result.end;
		return true;
	}
	return false;
}

void tokenizer::skip_whitespaces()
{ 
	skip(whitespaces);
}

void tokenizer::reverse_skip_whitespaces()
{ 
	reverse_skip(whitespaces);
}

/// 
void tokenizer::bite_all(std::vector<token>& result)
{
	while(!skip_ws_check_empty())
		result.push_back(bite());
}

	}
}
