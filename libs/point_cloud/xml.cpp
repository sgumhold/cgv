#include "xml.h"
#include <sstream>
#include <cgv/utils/advanced_scan.h>

namespace cgv {
namespace pointcloud {
namespace file_parser {

std::unique_ptr<xml_node> read_xml(const std::string& xml_content)
{

	enum class XMLTokenType { TAG, CONTENT };

	struct XMLTokens
	{
		XMLTokenType type;
		std::string content;
	};

	std::string::const_iterator str_iter = xml_content.cbegin();
	bool found_tag_begin = false;
	std::string::const_iterator tag_begin;
	std::string::const_iterator tag_end;
	std::string content;
	std::vector<XMLTokens> tokens;

	while (str_iter < xml_content.cend()) {
		if (*str_iter == '<' && !found_tag_begin) {
			tag_begin = str_iter;
			found_tag_begin = true;
			if (content.size() > 0) {
				// TODO store content
				tokens.emplace_back();
				tokens.back().content.swap(content);
				tokens.back().type = XMLTokenType::CONTENT;
			}
		}
		else if (*str_iter == '>') {
			if (found_tag_begin) {
				tag_end = str_iter + 1;
				found_tag_begin = false;
				std::string str(tag_begin, tag_end);
				tokens.emplace_back();
				tokens.back().content = str;
				tokens.back().type = XMLTokenType::TAG;
			}
			else {
				// error
				throw xml_parsing_error{"unexpected >"};
			}
		}
		else {
			if (!found_tag_begin)
				content.push_back(*str_iter);
			else if (*str_iter == 0) {
				break;
			}
		}

		++str_iter;
	}

	xml_tag preamble;
	std::unique_ptr<xml_node> xml_root;
	std::vector<xml_node*> node_stack;

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
					xml_root = std::make_unique<xml_node>(tag);
					node_stack.push_back(xml_root.get());
				}
				else {
					throw xml_parsing_error{"invalid root tag"};
				}
			}
			else {
				switch (tag.type) {
				case XMLTagType::XTT_OPEN: {

					auto* new_node = new xml_node(tag);
					node_stack.back()->add_child(new_node);
					node_stack.push_back(new_node);
					break;
				}
				case XMLTagType::XTT_CLOSE: {

					if (tag.name == node_stack.back()->tag.name) {
						node_stack.pop_back();
					}
					else {
						xml_tag& parent_tag = node_stack[node_stack.size() - 2]->tag;
						std::stringstream ss;
						ss << "expected closign tag for" << parent_tag.name << ", but found tag.name";
						throw xml_parsing_error{ss.str()};
					}
					break;
				}
				case XMLTagType::XTT_SINGLE: {
					auto* new_node = new xml_node(tag);
					node_stack.back()->add_child(new_node);
					break;
				}
				default: {
					throw xml_parsing_error{"found unknown tag"};
				}
				}
			}
		}
		else if (token.type == XMLTokenType::CONTENT) {
			if (node_stack.size() > 0) {
				node_stack.back()->add_content(token.content);
			}
			else {
				std::string non_whitespace_content = cgv::utils::trim(token.content);
				if (non_whitespace_content.size() > 0) {
					std::stringstream ss;
					ss << "found string  " << non_whitespace_content << " outside of tag ";
					throw xml_parsing_error{ss.str()};
				}
			}
		}
	}

	if (node_stack.size() > 0) {
		std::stringstream ss;
		ss << "missing close tag for " << node_stack.back()->tag.name;
		throw xml_parsing_error{ss.str()};
	}

	return xml_root;
}

xml_node::xml_node(const xml_tag& tag) : tag(tag) {}

void xml_node::add_child(xml_node* node)
{
	childs.push_back(node);
}
void xml_node::add_content(const std::string& c){
	non_tag_content.push_back(c);
}


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


}
}
}