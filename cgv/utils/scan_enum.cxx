#include "scan_enum.h"
#include "tokenizer.h"
#include "scan.h"
#include <algorithm>

namespace cgv {
	namespace utils {

/// parse an enum declaration string into names and values
void parse_enum_declarations(const std::string& enum_declarations, std::vector<token>& enum_names, std::vector<int>& enum_values)
{
	std::vector<token> toks;
	tokenizer(enum_declarations).set_ws(";,").bite_all(toks);
	int val = 0;
	for (unsigned int i=0; i<toks.size(); ++i) {
		std::vector<token> defs;
		tokenizer(toks[i]).set_ws("=").bite_all(defs);
		if (defs.size() < 1)
			continue;
		int new_val;
		if (defs.size() >= 2 && is_integer(defs[1].begin, defs[1].end, new_val))
			val = new_val;
		enum_names.push_back(defs[0]);
		enum_values.push_back(val);
		++val;
	}

}

/// convert value to index
unsigned find_enum_index(int value, const std::vector<int>& enum_values)
{
	std::vector<int>::const_iterator i = std::find(enum_values.begin(), enum_values.end(), value);
	if (i == enum_values.end())
		return -1;
	return (int)(i-enum_values.begin());
}

/// convert name to index
unsigned find_enum_index(const std::string& name, const std::vector<token>& enum_names)
{
	for (unsigned i=0; i<enum_names.size(); ++i) {
		if (enum_names[i] == name)
			return i;
	}
	return -1;
}

	}
}