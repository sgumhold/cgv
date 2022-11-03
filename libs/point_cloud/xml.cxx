#include "xml.h"
#include <sstream>
#include <cgv/utils/advanced_scan.h>

namespace cgv {
namespace pointcloud {
namespace file_parser {

class xml_parsing_error : public std::exception
{
	std::string msg;
public:
	inline xml_parsing_error() = default;

	inline xml_parsing_error(const std::string& m) : msg(m) {}

	inline const char* what() const noexcept
	{
		return msg.c_str();
	}
};

std::unique_ptr<xml_tag_node> read_xml(const std::string& xml_content)
{

	enum class XMLTokenType { TAG, CDATA, CONTENT };

	struct XMLTokens
	{
		XMLTokenType type;
		std::string content;
	};

	std::string::const_iterator str_iter = xml_content.cbegin();
	bool found_tag_begin = false;
	size_t tag_begin;
	size_t tag_end;
	std::string content;
	std::vector<XMLTokens> tokens;
	bool is_cdata = false;

	int i = 0;

	while (i < xml_content.size()) {
		if (xml_content[i] == '<' && !found_tag_begin) {
			tag_begin = i;
			found_tag_begin = true;

			if (i + 12 <= xml_content.size()) { //length of empty cdata tag = 12
				std::string cdata = xml_content.substr(i, 9);
				if (cdata.compare("<![CDATA[") == 0) {
					//TODO find end
					size_t it = xml_content.find("]]>", i + 8);
					if (it != std::string::npos) {
						found_tag_begin = false;
						tokens.emplace_back();
						tokens.back().type = XMLTokenType::CDATA;
						tokens.back().content = xml_content.substr(i, it - i + 3);
						i = it + 3;
						continue;
					}
				}
			}

			if (content.size() > 0) {
				tokens.emplace_back();
				tokens.back().content.swap(content);
				tokens.back().type = XMLTokenType::CONTENT;
			}
		}
		else if (xml_content[i] == '>') {
			if (found_tag_begin) {
				tag_end = i + 1;
				found_tag_begin = false;
				std::string str = xml_content.substr(tag_begin, tag_end - tag_begin);
				tokens.emplace_back();
				tokens.back().content = str;
				tokens.back().type = XMLTokenType::TAG;
			}
			else {
				// error
				throw xml_parsing_error("unexpected >");
			}
		}
		else {
			if (!found_tag_begin)
				content.push_back(xml_content[i]);
			else if (xml_content[i] == 0) {
				break;
			}
		}

		++i;
	}

	xml_tag preamble;
	std::unique_ptr<xml_tag_node> xml_root;
	std::vector<xml_tag_node*> node_stack;

	for (int i = 0; i < tokens.size(); ++i) {
		auto token = tokens[i];
		if (token.type == XMLTokenType::TAG) {
			xml_tag tag = xml_read_tag(token.content);
			if (tag.type == XMLTagType::XTT_PREAMBLE) {
				preamble = tag;
				continue;
			}
			else if (xml_root == nullptr) {
				if (tag.type == XMLTagType::XTT_OPEN) {
					xml_root = std::make_unique<xml_tag_node>(tag);
					node_stack.push_back(xml_root.get());
				}
				else {
					throw xml_parsing_error("invalid root tag");
				}
			}
			else {
				switch (tag.type) {
				case XMLTagType::XTT_OPEN: {

					auto* new_node = new xml_tag_node(tag);
					node_stack.back()->add_child(new_node);
					node_stack.push_back(new_node);
					break;
				}
				case XMLTagType::XTT_CLOSE: {
					if (node_stack.size() == 0) {
						std::stringstream ss;
						ss << "found closing tag" << tag.name <<" without opening tag";
						throw xml_parsing_error(ss.str());
					}
					if (tag.name == node_stack.back()->tag.name) {
						node_stack.pop_back();
					}
					else {
						xml_tag& last_tag = node_stack[node_stack.size() - 1]->tag;
						std::stringstream ss;
						ss << "expected closing tag for " << last_tag.name << ", but found " << tag.name;
						throw xml_parsing_error(ss.str());
					}
					break;
				}
				case XMLTagType::XTT_SINGLE: {
					auto* new_node = new xml_tag_node(tag);
					node_stack.back()->add_child(new_node);
					break;
				}
				default: {
					throw xml_parsing_error("found unknown tag");
				}
				}
			}
		}
		else if (token.type == XMLTokenType::CDATA) {
			xml_tag tag;
			tag.name = "CDATA";
			tag.type = XMLTagType::XTT_CDATA;
			auto* new_node = new xml_string_node(token.content.substr(9, token.content.size() - (9 + 3)));
			//new_node->add_content(token.content.substr(9, token.content.size() - (9+3)));
			node_stack.back()->add_child(new_node);
		}
		else if (token.type == XMLTokenType::CONTENT) {
			if (node_stack.size() > 0) {
				auto* new_node = new xml_string_node(token.content);
				//node_stack.back()->add_content(token.content);
				node_stack.back()->add_child(new_node);
			}
			else {
				std::string non_whitespace_content = cgv::utils::trim(token.content);
				if (non_whitespace_content.size() > 0) {
					std::stringstream ss;
					ss << "found string  " << non_whitespace_content << " outside of tag ";
					throw xml_parsing_error(ss.str());
				}
			}
		}
	}

	if (node_stack.size() > 0) {
		std::stringstream ss;
		ss << "missing close tag for " << node_stack.back()->tag.name;
		throw xml_parsing_error(ss.str());
	}

	return xml_root;
}

//xml_node::xml_node(const xml_tag& tag) : tag(tag) {}

void xml_node::free_childs() {
	for (auto* ch : childs)
		delete ch;
}

void xml_node::add_child(xml_node* node)
{
	childs.push_back(node);
}

/* 
void xml_node::add_content(const std::string& c)
{
	non_tag_content.push_back(c);
}
*/

// modified copy from cgv/utils/xml.h
xml_attribute xml_read_attribute(const std::string& attribute)
{
	std::string name = "";
	std::string value = "";

	size_t pos = attribute.find_first_of('=');

	if (pos != std::string::npos) {
		name = attribute.substr(0, pos);
	}

	size_t pos_start = attribute.find_first_of("\"", pos);
	size_t pos_end = attribute.find_last_of("\"");

	if (pos_start != std::string::npos && pos_end != std::string::npos && pos_start < pos_end && attribute.length() > 2)
	{
		value = attribute.substr(pos_start + 1, pos_end - pos_start - 1);
	}

	return {name, value};
}

xml_tag xml_read_tag(const std::string& str)
{
	xml_tag tag;
	// trim whitespace
	std::string s = cgv::utils::trim(str);
	// trim trailing line breaks
	cgv::utils::rtrim(s, "\n");

	if (s.length() < 3)
		return tag;

	size_t r = 1;
	size_t l = 1;

	if (s[0] == '<') {
		tag.type = XTT_OPEN;
		if (s[1] == '/') {
			tag.type = XTT_CLOSE;
			r = 2;
		}
		else if (s[1] == '?') {
			tag.type = XTT_PREAMBLE;
			l = 2;
			r = 2;
		}
		else if (s[1] == '!') {
			size_t it = s.find("<![CDATA[");
			if (it == 0) {
				tag.type = XTT_CDATA;
				r = 2;

				size_t len = s.length();
				size_t cdata_end = s.find("]]>");

				if (cdata_end != std::string::npos) {
					tag.name = "CDATA";
					tag.attributes.emplace("CDATA", s.substr(9,cdata_end-9));
					return tag;
				}
			}
		}
		else {
			size_t len = s.length();
			if (s[len - 2] == '/' && s[len - 1] == '>') {
				tag.type = XTT_SINGLE;
				l = 2;
			}
		}
	}

	s = s.substr(r, s.length() - l - 1 - (r - 1));

	std::vector<cgv::utils::token> tokens;
	split_to_tokens(s, tokens, "", true, "\"", "\"");

	if (tokens.size() > 0) {
		tag.name = to_string(tokens[0]);

		if (tag.type != XTT_CLOSE) {
			for (size_t i = 1; i < tokens.size(); i += 2) {
				std::string name = to_string(tokens[i]);
				if (name.back() == '=')
					name = name.substr(0, name.length() - 1);
				tag.attributes.emplace(name, to_string(tokens[i + 1]));
				// tag.attributes.emplace(xml_read_attribute(to_string(tokens[i])));
			}
		}
	}
	else {
		tag.type = XTT_UNDEF;
	}

	return tag;
}

xml_string_node::xml_string_node(const std::string& str) : content (str) {}
	
xml_tag_node::xml_tag_node(const xml_tag& tag) : tag(tag) {}

bool xml_node::has_tag() const
{
	return false;
}

xml_node::~xml_node() {
	free_childs();
}

bool xml_tag_node::has_tag() const
{
	return true;
}

} // namespace file_parser
}
}