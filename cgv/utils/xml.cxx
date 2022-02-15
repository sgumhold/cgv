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
	std::vector<cgv::utils::token> tokens;
	split_to_tokens(str, tokens, "", true, "", "");

	xml_tag tag;

	if(tokens.size() > 0) {
		tag.name = to_string(tokens[0]);

		for(size_t i = 1; i < tokens.size(); ++i)
			tag.attributes.emplace(xml_read_attribute(to_string(tokens[i])));
	}

	return tag;
}

	}
}
