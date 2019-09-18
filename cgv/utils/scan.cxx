#include "scan.h"

#include <stdlib.h>
#include <algorithm>

namespace cgv {
	namespace utils {

bool is_space(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool is_url_special(char c) {
	return c == '.' || c == ',' || c == ';' || c == ':';
}

bool is_digit(char c) {
	return c >= '0' && c <= '9';
}

bool is_letter(char c)
{
	c = to_lower(c);
	return c >= 'a' && c <= 'z';
}


char to_lower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return (c-'A')+'a';
	switch (c) {
		case 'Ö' : return 'ö';
		case 'Ü' : return 'ü';
		case 'Ä' : return 'ä';
		default: return c;
	}
}

std::string to_lower(const std::string& _s)
{
	std::string s(_s);
	for (unsigned int i=0; i<s.size(); ++i)
		s[i] = to_lower(s[i]);
	return s;
}

char to_upper(char c)
{
	if (c >= 'a' && c <= 'z')
		return (c-'a')+'A';
	switch (c) {
		case 'ö' : return 'Ö';
		case 'ü' : return 'Ü';
		case 'ä' : return 'Ä';
		default: return c;
	}
}

std::string to_upper(const std::string& _s)
{
	std::string s(_s);
	for (unsigned int i=0; i<s.size(); ++i)
		s[i] = to_upper(s[i]);
	return s;
}

std::string replace_special(const std::string& _s)
{
	std::string s;
	for (unsigned int i=0; i<_s.size(); ++i) {
		switch (_s[i]) {
		case 'Ä' : s += "Ae"; break;
		case 'Ö' : s += "Oe"; break;
		case 'Ü' : s += "Ue"; break;
		case 'ä' : s += "ae"; break;
		case 'ö' : s += "oe"; break;
		case 'ü' : s += "ue"; break;
		case 'ß' : s += "ss"; break;
		default  : s += _s[i]; break;
		}
	}
	return s;
}

unsigned int replace(std::string& s, char c1, char c2)
{
	unsigned int count = 0;
	for (unsigned int i=0; i<s.size(); ++i) {
		if (s[i] == c1) {
			s[i] = c2;
			++count;
		}
	}
	return count;
}

unsigned int replace(std::string& _s, const std::string& s1, const std::string& s2)
{
	if (s1.empty())
		return 0;
	size_t l  = _s.size();
	size_t l1 = s1.size();
	if (l1 > l)
		return 0;
	size_t l2 = s2.size();
	// count number of replacements in n
	size_t n = 0;
	for (size_t pos = 0; pos <= l - l1; ++pos) {
		size_t i;
		for (i=0; i<l1; ++i)
			if (_s[pos+i] != s1[i])
				break;
		if (i == l1) {
			if (l1 == l2) {
				for (i=0; i<l1; ++i)
					_s[pos+i] = s2[i];
			}
			else {
				_s = _s.substr(0, pos) + s2 + _s.substr(pos+l1);
				l  = _s.size();
			}
			pos += l2;
			++n;
			if (l1 > l)
				return (unsigned)n;
		}
	}
	return (unsigned)n;
}

std::string escape_special(const std::string& s)
{
	std::string r;
	for (unsigned int i = 0; i < s.size(); ++i) {
		switch (s[i]) {
		case '\a' : r += "\\a"; break;
		case '\b' : r += "\\b"; break;
		case '\f' : r += "\\f"; break;
		case '\n' : r += "\\n"; break;
		case '\r' : r += "\\r"; break;
		case '\t' : r += "\\t"; break;
		case '\v' : r += "\\v"; break;
		case '\'' : r += "\\'"; break;
		case '"'  : r += "\\\""; break;
		case '\\' : r += "\\\\"; break;
		default:
			r += s[i];
		}
	}
	return r;
}

std::string interpret_special(const std::string& s)
{
	std::string r;
	for (unsigned int i = 0; i < s.size(); ++i) {
		if (s[i] == '\\') {
			if (i+1 < s.size()) {
				++i;
				switch (s[i]) {
				case 'a' : r += '\a'; break;
				case 'b' : r += '\b'; break;
				case 'f' : r += '\f'; break;
				case 'n' : r += '\n'; break;
				case 'r' : r += '\r'; break;
				case 't' : r += '\t'; break;
				case 'v' : r += '\v'; break;
				case '\'' : r += '\''; break;
				case '\"' : r += '\"'; break;
				case '\\' : r += '\\'; break;
				case '?' : r += '\?'; break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					if (i+2 < s.size() && is_digit(s[i+1]) && is_digit(s[i+2]) ) {
						r += char((s[i]-'0')*64+(s[i+1]-'0')*8+(s[i+2]-'0'));
						i += 2;
					}
					else 
						r += s[i];
					break;
				case 'x' :
					if (i+2 < s.size()) {
						char c = 0;
						if (is_digit(s[i+1]))
							c += (s[i+1]-'0')*16;
						else 
							c += (to_lower(s[i+1])-'a')*16;
						if (is_digit(s[i+2]))
							c += s[i+2]-'0';
						else 
							c += to_lower(s[i+2])-'a';
						r += c;
						i += 2;
					}
					else
						r += s[i];
					break;
				default:    r += s[i]; break;
				}
			}
			else
				r += s[i];
		}
		else 
			r += s[i];
	}
	return r;
}


bool is_element(char c, const std::string& s)
{
	return s.find_first_of(c) != std::string::npos;
}

bool is_element(const std::string& e, const std::string& s, char sep)
{
	return get_element_index(e,s,sep) != -1;
}

std::string get_element(const std::string& s, int element_index, char sep)
{
	size_t end_pos, start_pos = 0;
	int ei = -1;
	do {
		++ei;
		if (ei > 0) {
			start_pos = end_pos + 1;
			if (start_pos == s.size())
				return "";
		}
		end_pos = s.find_first_of(sep, start_pos);
		if (end_pos == std::string::npos) {
			end_pos = s.size();
			break;
		}
	} while (ei < element_index);
	if (ei < element_index)
		return "";
	return s.substr(start_pos, end_pos - start_pos);
}

int get_element_index(const std::string& e, const std::string& s, char sep)
{
	if (e.empty() && s.empty())
		return 0;

	size_t n = s.size();
	bool at_start = true;
	int idx = 0;
	unsigned k = 0;
	bool match = true;
	for (size_t i = 0; i<n; ++i) {
		if (s[i] == sep) {
			if (k == e.size() && match)
				return idx;
			k = 0;
			++idx;
			match = true;
		}
		else {
			at_start = false;
			if (match) {
				if (k == e.size() || s[i] != e[k])
					match = false;
				else
					++k;
			}
		}
	}
	if (k == e.size() && match)
		return idx;
	return -1;
}

bool is_integer(const char* begin, const char* end, int& value)
{
	if (begin == end)
		return false;
	// check for hexadecimal case
	if (end-begin>2 && begin[0] == '0' && to_upper(begin[1]) == 'X') {
		int new_value = 0, b = 1;
		const char* p;
		for (p = end; p>begin+2; b *= 16) {
			--p;
			if (is_digit(*p))
				new_value += (int)(*p - '0')*b;
			else {
				char c = to_upper(*p);
				if (c >= 'A' && c <= 'F')
					new_value += (int)(c - 'A' + 10)*b;
				else
					return false;
			}
		}
		value = new_value;
		return true;
	}
	int new_value = 0, b = 1;
	const char* p;
	for (p = end; p>begin; b *= 10) {
		--p;
		if (is_digit(*p))
			new_value += (int)(*p - '0')*b;
		else {
			if (*p == '-') {
				new_value = -new_value;
				break;
			}
			if (*p != '+')
				return false;
			break;
		}
	}
	if (p == begin) {
		value = new_value;
		return true;
	}
	return false;
}

bool is_integer(const std::string& s, int& value)
{
	return is_integer(&s[0], &s[0]+s.size(), value);
}

bool is_double(const char* begin, const char* end, double& value)
{
	if (begin == end)
		return false;
	bool found_digit = false;
	int nr_dots = 0;
	int nr_exp = 0;
	bool sign_may_follow = true;
	const char* p;
	for (p = begin; p<end; ++p) {
		switch (*p) {
		case '0' :
		case '1' :
		case '2' :
		case '3' :
		case '4' :
		case '5' :
		case '6' :
		case '7' :
		case '8' :
		case '9' :
			found_digit = true;
			sign_may_follow = false;
			break;
		case '+' :
		case '-' :
			if (!sign_may_follow)
				return false;
			sign_may_follow = false;
			break;
		case '.' :
			if (++nr_dots > 1)
				return false;
			sign_may_follow = false;
			break;
		case 'e' :
		case 'E' :
			if (++nr_exp > 1)
				return false;
			sign_may_follow = true;
			break;
		default:
			return false;
		}
	}
	if (!found_digit)
		return false;
	value = atof(std::string(begin,end-begin).c_str());
	return true; 
}

bool is_double(const std::string& s, double& value)
{
	return is_double(&s[0], &s[0]+s.size(), value);
}


bool is_year(const char* begin, const char* end, unsigned short& year, bool short_allowed)
{
	int i;
	unsigned int size = (unsigned int) (end-begin);
	if ((size == 2 && short_allowed) || 
		 ((size == 4) && is_integer(begin, end, i))) {
		year = i;
		return true;
	}
	return false;
}

bool is_year(const std::string& s, unsigned short& year, bool short_allowed)
{
	return is_year(&s[0], &s[0]+s.size(), year, short_allowed);
}

bool find_name(const std::string& s, const char* names[], int& idx)
{
	unsigned int i=0;
	while (names[i]) {
		if (s == names[i]) {
			idx = i;
			return true;
		}
		++i;
	}
	return false;
}

bool is_month(const char* begin, const char* end, unsigned char& month)
{
	static const char* months_short[] = {
		"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec", 0
	};
	static const char* months_english[] = {
		"january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december", 0
	};
	static const char* months_german[] = {
		"januar", "februar", "maerz", "april", "mai", "juni", "juli", "august", "september", "oktober", "november", "dezember", 0
	};
	int i;
	if (is_integer(begin,end,i)) {
		if (i > 0 && i <= 12) {
			month = i+1;
			return true;
		}
	}
	std::string s = to_lower(replace_special(std::string(begin,end-begin)));
	if (find_name(s, months_short, i) || find_name(s, months_english, i) || find_name(s, months_german, i) ) {
		month = i+1;
		return true;
	}
	return false;
}

bool is_month(const std::string& s, unsigned char& month)
{
	return is_month(&s[0], &s[0]+s.size(), month);
}

bool is_time(const std::string& s, cgv::utils::time& t, const char **new_end)
{
	if (s.size() == 0)
		return false;
	size_t end = s.size();
	while (end > 0 && is_url_special(s[end-1]))
		--end;
	size_t pos = s.find_first_of(':');
	if (pos == std::string::npos)
		return false;
	int i;
	if (!(is_integer(&s[0], &s[pos], i) && i >= 0 && i < 24))
		return false;
	t.h = (unsigned char) i;
	if (pos >= end)
		return false;
	++pos;
	size_t pos2 = s.find_first_of(':', pos);
	size_t pos3 = pos2+1;
	if (pos2 == std::string::npos)
		pos3 = pos2 = end;
	if (!(is_integer(&s[pos], &s[pos2], i) && i >= 0 && i < 60))
		return false;
	t.minutes = (unsigned char) i;
	if (pos2 >= end) {
		if (new_end)
			*new_end = &s[0]+pos3;
		return true;
	}
	pos = ++pos2;
	while (pos2 < end && is_digit(s[pos2]))
		++pos2;
	if (!(is_integer(&s[pos], &s[pos2], i) && i >= 0 && i < 60))
		return false;
	t.sec = i;
	if (new_end)
		*new_end = &s[0]+pos2;
	return true;
}

bool is_date(const std::string& s, cgv::utils::date& d, const char **new_end)
{
	if (s.size() == 0)
		return false;
	size_t end = s.size();
	while (end > 0 && is_url_special(s[end-1]))
		--end;
	size_t pos = s.find_first_of('.');
	if (pos == std::string::npos)
		return false;
	int i;
	if (!(is_integer(&s[0], &s[pos], i) && i > 0 && i < 32))
		return false;
	d.day = (unsigned char)i;
	if (pos >= end) {
		if (new_end)
			*new_end = &s[0]+pos+1;
		return true;
	}
	++pos;
	size_t pos2 = s.find_first_of('.', pos);
	if (pos2 == std::string::npos)
		return false;
	if (!is_month(&s[pos], &s[pos2], d.month))
		return false;
	if (pos2 >= end) {
		if (new_end)
			*new_end = &s[0]+pos2+1;
		return true;
	}
	pos = ++pos2;
	while (pos2 < end && is_digit(s[pos2]))
		++pos2;
	if (!is_year(&s[pos],&s[pos2],d.year))
		return false;
	if (new_end)
		*new_end = &s[0]+pos2;
	return true;
}

bool is_time(const char* begin, const char* end, cgv::utils::time& t, const char **new_end)
{
	std::string s(begin,end-begin);
	if (is_time(s, t, new_end)) {
		if (new_end)
			*new_end = end + (*new_end - (&s[0]+s.length()));
		return true;
	}
	return false;
}

bool is_date(const char* begin, const char* end, cgv::utils::date& d, const char **new_end)
{
	std::string s(begin,end-begin);
	if (is_date(s, d, new_end)) {
		if (new_end)
			*new_end = end + (*new_end - (&s[0]+s.length()));
		return true;
	}
	return false;
}

bool is_url(const std::string& s, const char** end)
{
	if (s.substr(0,8) == "https://" || 
		 s.substr(0,7) == "http://" ||
		 s.substr(0,6) == "ftp://" ||
		 s.substr(0,6) == "smb://" ||
		 s.substr(0,6) == "svn://" ||
		 s.substr(0,7) == "file://") {
			 if (end) {
				 *end = &s[s.length()-1];
				 while (is_url_special(**end))
					 --*end;
				 ++*end;
			 }
			 return true;
	}
	return false;
}

bool is_url(const char* begin, const char* end, const char** new_end)
{
	std::string s(begin,end-begin);
	if (is_url(s, new_end)) {
		if (new_end)
			*new_end = end + (*new_end - (&s[0]+s.length()));
		return true;
	}
	return false;
}

const char* skip_spaces(const char* begin, const char* end)
{
	while (begin < end && is_space(*begin))
		++begin;
	return begin;
}

const char* cutoff_spaces(const char* begin, const char* end)
{
	while (begin < end && is_space(*(end-1)))
		--end;
	return end;
}

	}
}
