#include "xml.h"

namespace cgv {
	namespace utils {

xml_attribute xml_read_attribute(const std::string& attribute)
{
	std::string name = "";
	std::string value = "";

	size_t pos = attribute.find_first_of('=');

	if(pos != std::string::npos) {
		name = attribute.substr(0, pos);
	}

	size_t pos_start = attribute.find_first_of("\"", pos);
	size_t pos_end = attribute.find_last_of("\"");

	if(pos_start != std::string::npos &&
		pos_end != std::string::npos &&
		pos_start < pos_end &&
		attribute.length() > 2) {
		value = attribute.substr(pos_start + 1, pos_end - pos_start - 1);
	}

	return { name, value };
}

xml_tag xml_read_tag(const std::string& str)
{
	xml_tag tag;
	// trim whitespace
	std::string s = trim(str);
	// trim trailing line breaks
	rtrim(s, "\n");

	if(s.length() < 3)
		return tag;

	size_t r = 1;
	size_t l = 1;

	if(s[0] == '<') {
		tag.type = XTT_OPEN;
		if(s[1] == '/') {
			tag.type = XTT_CLOSE;
			r = 2;
		} else {
			size_t len = s.length();
			if(s[len - 2] == '/' && s[len - 1] == '>') {
				tag.type = XTT_SINGLE;
				l = 2;
			}
		}
	}

	s = s.substr(r, s.length() - l - 1 - (r - 1));

	std::vector<cgv::utils::token> tokens;
	split_to_tokens(s, tokens, "", true, "\"", "\"");

	if(tokens.size() > 0) {
		tag.name = to_string(tokens[0]);

		if(tag.type != XTT_CLOSE) {
			for(size_t i = 1; i < tokens.size(); i += 2) {
				std::string name = to_string(tokens[i]);
				if(name.back() == '=')
					name = name.substr(0, name.length() - 1);
				tag.attributes.emplace(name, to_string(tokens[i + 1]));
				//tag.attributes.emplace(xml_read_attribute(to_string(tokens[i])));
			}
		}
	} else {
		tag.type = XTT_UNDEF;
	}

	return tag;
}

	}
}
